#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <mmsystem.h>
#include <xinput.h>
#include <dinput.h>
#include <setupapi.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "bridge.h"
/* ViGEm's public C ABI; no copied implementation, no SDK required to compile.
 * x64 only. Calling convention on x64 is unified. DLL must be built from
 * https://github.com/nefarius/ViGEmClient (MIT). */
static HMODULE xi_dll, vi_dll;
typedef DWORD (WINAPI *GetStateFn)(DWORD,XINPUT_STATE*);
typedef DWORD (WINAPI *GetCapsFn)(DWORD,DWORD,XINPUT_CAPABILITIES*);
static GetStateFn get_state; static GetCapsFn get_caps;
static void *client,*target;
static void *(*v_alloc)(void), *(*v_target_alloc)(void);
static void (*v_free)(void*),(*v_disconnect)(void*),(*v_target_free)(void*);
static unsigned int (*v_connect)(void*),(*v_add)(void*,void*),(*v_remove)(void*,void*),(*v_update)(void*,void*,BWReport),(*v_index)(void*,void*,PULONG);
static ULONG virtual_slot=ULONG_MAX;
static IDirectInput8A *di;
static IDirectInputDevice8A *di_device;
static char di_name[160];
static double next_di_scan;
static int timer_enabled;
static HANDLE wait_timer;
double bw_now(void) { LARGE_INTEGER n,f; QueryPerformanceCounter(&n);QueryPerformanceFrequency(&f);return (double)n.QuadPart/(double)f.QuadPart; }
void bw_sleep(double seconds) {
    LARGE_INTEGER due;
    if(seconds<=0)return;
    if(!wait_timer)wait_timer=CreateWaitableTimerExW(NULL,NULL,0x00000002,TIMER_ALL_ACCESS);
    if(!wait_timer)wait_timer=CreateWaitableTimerW(NULL,FALSE,NULL);
    due.QuadPart=-(LONGLONG)ceil(seconds*10000000.);
    if(wait_timer && SetWaitableTimer(wait_timer,&due,0,NULL,NULL,FALSE))WaitForSingleObject(wait_timer,INFINITE);
    else Sleep((DWORD)ceil(seconds*1000.));
}
static float normalized(SHORT x) { return x<0?x/32768.f:x/32767.f; }
static BOOL CALLBACK find_wheel(const DIDEVICEINSTANCEA *info,void *context) {
    (void)context;
    WORD vid=LOWORD(info->guidProduct.Data1),pid=HIWORD(info->guidProduct.Data1);
    if (!((vid==0x24c6 && (pid==0x5b00||pid==0x5b03)) || (vid==0x044f && strstr(info->tszProductName,"458")))) return DIENUM_CONTINUE;
    if(SUCCEEDED(IDirectInput8_CreateDevice(di,&info->guidInstance,&di_device,NULL))) {
        DIPROPRANGE range;
        if(FAILED(IDirectInputDevice8_SetDataFormat(di_device,&c_dfDIJoystick2)) ||
           FAILED(IDirectInputDevice8_SetCooperativeLevel(di_device,GetDesktopWindow(),DISCL_BACKGROUND|DISCL_NONEXCLUSIVE))) {
            IDirectInputDevice8_Release(di_device); di_device=NULL; return DIENUM_CONTINUE;
        }
        memset(&range,0,sizeof(range)); range.diph.dwSize=sizeof(range);range.diph.dwHeaderSize=sizeof(range.diph);range.diph.dwHow=DIPH_DEVICE;range.lMin=-32768;range.lMax=32767;
        IDirectInputDevice8_SetProperty(di_device,DIPROP_RANGE,&range.diph);
        IDirectInputDevice8_Acquire(di_device);
        snprintf(di_name,sizeof(di_name),"%.159s",info->tszProductName);
        return DIENUM_STOP;
    }
    return DIENUM_CONTINUE;
}
/* Only enumerate PnP while no readable input exists, outside the active path. */
static void detect_unreadable_usb(BWInput *in) {
    static double next_scan;
    static int present;
    if(bw_now()>=next_scan) {
        HDEVINFO devices=SetupDiGetClassDevsA(NULL,NULL,NULL,DIGCF_ALLCLASSES|DIGCF_PRESENT);
        SP_DEVINFO_DATA info;DWORD index;present=0;next_scan=bw_now()+1;
        memset(&info,0,sizeof(info));info.cbSize=sizeof(info);
        if(devices!=INVALID_HANDLE_VALUE) {
            for(index=0;SetupDiEnumDeviceInfo(devices,index,&info);++index) {
                char ids[4096]={0};
                if(!SetupDiGetDeviceRegistryPropertyA(devices,&info,SPDRP_HARDWAREID,NULL,(BYTE*)ids,sizeof(ids)-2,NULL))continue;
                for(size_t j=0;j<sizeof(ids);++j)if(ids[j]>='a'&&ids[j]<='z')ids[j]=(char)(ids[j]-32);
                for(char *id=ids;*id;id+=strlen(id)+1) {
                    if(strstr(id,"VID_24C6&PID_5B00")||strstr(id,"VID_24C6&PID_5B03")){present=1;break;}
                }
                if(present)break;
            }
            SetupDiDestroyDeviceInfoList(devices);
        }
    }
    if(present) {
        in->usb_present=1;
        snprintf(in->name,sizeof(in->name),"Thrustmaster Ferrari 458 Italia · USB detectado");
        snprintf(in->detail,sizeof(in->detail),"Volante presente no USB, mas sem entrada legível. Confira o vínculo com o driver Xbox 360 no Gerenciador de Dispositivos e teste em joy.cpl; tente o slot XInput físico em Diagnóstico.");
    }
}
void bw_input_read(BWInput *in,const BWConfig *c) {
    DWORD i;
    memset(in,0,sizeof(*in));
    if(!timer_enabled) { timeBeginPeriod(1);timer_enabled=1; }
    if(!xi_dll) {
        xi_dll=LoadLibraryExA("xinput1_4.dll",NULL,LOAD_LIBRARY_SEARCH_SYSTEM32);
        if(xi_dll) { FARPROC a=GetProcAddress(xi_dll,"XInputGetState"),b=GetProcAddress(xi_dll,"XInputGetCapabilities");memcpy(&get_state,&a,sizeof(a));memcpy(&get_caps,&b,sizeof(b)); }
    }
    if(!c->directinput && get_state && get_caps) {
        for(i=0;i<4;++i) {
            XINPUT_STATE s; XINPUT_CAPABILITIES caps;
            if(i==virtual_slot || (c->slot>=0 && i!=(DWORD)c->slot)) continue;
            if(get_caps(i,0,&caps)!=ERROR_SUCCESS)continue;
            if(c->slot<0 && caps.SubType!=XINPUT_DEVSUBTYPE_WHEEL)continue;
            if(get_state(i,&s)!=ERROR_SUCCESS)continue;
            in->connected=1; in->steering=normalized(s.Gamepad.sThumbLX);
            in->throttle=s.Gamepad.bRightTrigger/255.f;in->brake=s.Gamepad.bLeftTrigger/255.f;
            in->ly=normalized(s.Gamepad.sThumbLY);in->rx=normalized(s.Gamepad.sThumbRX);in->ry=normalized(s.Gamepad.sThumbRY);
            in->buttons=s.Gamepad.wButtons;
            in->axes[0]=in->steering;in->axes[1]=in->ly;in->axes[2]=in->brake*2-1;in->axes[3]=in->rx;in->axes[4]=in->ry;in->axes[5]=in->throttle*2-1;
            snprintf(in->name,sizeof(in->name),caps.SubType==XINPUT_DEVSUBTYPE_WHEEL?"Volante XInput (subtipo WHEEL)":"Controle XInput · seleção manual");
            snprintf(in->backend,sizeof(in->backend),"XInput 1.4 · slot %lu",(unsigned long)i);
            snprintf(in->detail,sizeof(in->detail),"Eixos independentes · LX direção / RT acelerador / LT freio. Confirme o modelo pelos movimentos.");
            return;
        }
    }
    if(c->slot>=0 && !c->directinput) {
        snprintf(in->detail,sizeof(in->detail),"Slot XInput selecionado não disponível (ou é a saída virtual do Boreal Wheel)."); detect_unreadable_usb(in);return;
    }
    if(!di) DirectInput8Create(GetModuleHandle(NULL),DIRECTINPUT_VERSION,&IID_IDirectInput8A,(void**)&di,NULL);
    if(di && !di_device && bw_now()>=next_di_scan) { next_di_scan=bw_now()+1; IDirectInput8_EnumDevices(di,DI8DEVCLASS_GAMECTRL,find_wheel,NULL,DIEDFL_ATTACHEDONLY); }
    if(di_device) {
        DIJOYSTATE2 s; HRESULT hr;
        IDirectInputDevice8_Poll(di_device);
        hr=IDirectInputDevice8_GetDeviceState(di_device,sizeof(s),&s);
        if(FAILED(hr)) { IDirectInputDevice8_Acquire(di_device); hr=IDirectInputDevice8_GetDeviceState(di_device,sizeof(s),&s); }
        if(SUCCEEDED(hr)) {
            LONG axes[]={s.lX,s.lY,s.lZ,s.lRx,s.lRy,s.lRz};
            static const int bits[]={12,13,14,15,8,9,5,4,6,7};
            int j; in->connected=1;in->usb_present=1;in->combined_pedals=1;
            for(j=0;j<6;++j) in->axes[j]=axes[j]<0?axes[j]/32768.f:axes[j]/32767.f;
            in->steering=in->axes[c->steering_axis];
            float z=in->axes[c->pedal_axis]; in->brake=z>0?z:0;in->throttle=z<0?-z:0;
            for(j=0;j<10;++j) if(s.rgbButtons[j]&0x80) in->buttons |= (uint16_t)(1u<<bits[j]);
            if(LOWORD(s.rgdwPOV[0])!=0xFFFF) {
                DWORD p=s.rgdwPOV[0];
                if(p>=31500||p<=4500)in->buttons|=1;
                if(p>=13500&&p<=22500)in->buttons|=2;
                if(p>=22500&&p<=31500)in->buttons|=4;
                if(p>=4500&&p<=13500)in->buttons|=8;
            }
            snprintf(in->name,sizeof(in->name),"%s",di_name);snprintf(in->backend,sizeof(in->backend),"DirectInput 8 · fallback");
            snprintf(in->detail,sizeof(in->detail),"Pedais combinados em DirectInput: não preserva acelerador + freio simultâneos. Prefira XInput ou selecione o slot físico.");
            return;
        }
        IDirectInputDevice8_Unacquire(di_device);IDirectInputDevice8_Release(di_device);di_device=NULL;
    }
    snprintf(in->detail,sizeof(in->detail),"Sem entrada legível. Confira o driver Xbox 360 em devmgmt.msc e teste em joy.cpl. Se listado como gamepad, escolha o slot físico.");
    detect_unreadable_usb(in);
}
void bw_input_close(void) {
    if(di_device){IDirectInputDevice8_Unacquire(di_device);IDirectInputDevice8_Release(di_device);di_device=NULL;}
    if(di){IDirectInput8_Release(di);di=NULL;}
    if(xi_dll){FreeLibrary(xi_dll);xi_dll=NULL;}get_state=NULL;get_caps=NULL; if(timer_enabled){timeEndPeriod(1);timer_enabled=0;}
    if(wait_timer){CloseHandle(wait_timer);wait_timer=NULL;}
}
void bw_output_close(void) {
    if(target) { BWReport neutral={0};v_update(client,target,neutral);v_remove(client,target);v_target_free(target);target=NULL; }
    if(client){v_disconnect(client);v_free(client);client=NULL;}
    virtual_slot=ULONG_MAX;
}
int bw_output(const BWReport *report,int enabled,char *status,int capacity) {
    static double next_retry;
    if(!enabled){if(target||client)bw_output_close();snprintf(status,capacity,"Saída virtual desativada");return 0;}
    if(!vi_dll) {
        char path[MAX_PATH];DWORD len=GetModuleFileNameA(NULL,path,sizeof(path));char *slash;
        if(len==0||len>=sizeof(path)){snprintf(status,capacity,"Caminho do executável inválido");return 0;}
        slash=strrchr(path,'\\');if(slash)slash[1]='\0';
        if(strlen(path)+strlen("ViGEmClient.dll")>=sizeof(path))return 0;
        strcat(path,"ViGEmClient.dll");
        vi_dll=LoadLibraryExA(path,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
        if(!vi_dll){snprintf(status,capacity,"ViGEmClient.dll ausente ao lado do executável");return 0;}
#define VLOAD(var,name) do { FARPROC proc=GetProcAddress(vi_dll,name); memcpy(&var,&proc,sizeof(proc)); if(!var)goto invalid_dll; } while(0)
        VLOAD(v_alloc,"vigem_alloc");VLOAD(v_free,"vigem_free");VLOAD(v_connect,"vigem_connect");VLOAD(v_disconnect,"vigem_disconnect");
        VLOAD(v_target_alloc,"vigem_target_x360_alloc");VLOAD(v_target_free,"vigem_target_free");VLOAD(v_add,"vigem_target_add");VLOAD(v_remove,"vigem_target_remove");
        VLOAD(v_update,"vigem_target_x360_update");VLOAD(v_index,"vigem_target_x360_get_user_index");
#undef VLOAD
    }
    if(!target && bw_now()<next_retry){snprintf(status,capacity,"Aguardando nova tentativa do ViGEmBus");return 0;}
    if(!client) {
        client=v_alloc();
        if(!client || v_connect(client)!=0x20000000u) {bw_output_close();next_retry=bw_now()+2;snprintf(status,capacity,"ViGEmBus não disponível · instale o driver oficial");return 0;}
    }
    if(!target) {
        target=v_target_alloc();
        if(!target||v_add(client,target)!=0x20000000u){bw_output_close();next_retry=bw_now()+2;snprintf(status,capacity,"Falha ao criar Xbox 360 virtual no ViGEmBus");return 0;}
    }
    /* Never read our own target. Fail closed if its XInput slot is not known. */
    if(v_index(client,target,&virtual_slot)!=0x20000000u || virtual_slot>3){bw_output_close();next_retry=bw_now()+2;snprintf(status,capacity,"Não foi possível identificar o slot virtual com segurança");return 0;}
    if(v_update(client,target,*report)!=0x20000000u){bw_output_close();next_retry=bw_now()+2;snprintf(status,capacity,"Falha ao enviar relatório ao ViGEmBus");return 0;}
    snprintf(status,capacity,"Xbox 360 virtual ativo · slot %lu",(unsigned long)virtual_slot);return 1;
invalid_dll:
    FreeLibrary(vi_dll);vi_dll=NULL;snprintf(status,capacity,"ViGEmClient.dll incompatível · confira build x64");return 0;
}
