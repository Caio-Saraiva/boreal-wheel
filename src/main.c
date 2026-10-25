#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <process.h>
typedef SOCKET socket_t;
#define CLOSESOCKET closesocket
static CRITICAL_SECTION mutex;
#define LOCK() EnterCriticalSection(&mutex)
#define UNLOCK() LeaveCriticalSection(&mutex)
#else
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
typedef int socket_t;
#define INVALID_SOCKET (-1)
#define CLOSESOCKET close
static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
#define LOCK() pthread_mutex_lock(&mutex)
#define UNLOCK() pthread_mutex_unlock(&mutex)
#endif
#include "bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
_Static_assert(sizeof(BWReport)==12,"XUSB report ABI must be 12 bytes");
_Static_assert(offsetof(BWReport,lx)==4,"XUSB report ABI mismatch");
static volatile sig_atomic_t running=1;
static BWConfig config;
static BWInput latest, manual_input;
static BWReport report;
static int demo_mode, demo_auto=1, output_requested, output_active;
static double map_us, observed_hz;
static unsigned long long packets;
static char output_status[256]="Virtual output disabled";
static int port=4580;
static void stop_signal(int sig){(void)sig;running=0;}
static void config_load(void) {
    FILE *f=fopen("profile.ini","rb");char line[160],key[64],val[64];BWConfig candidate=config;
    if(!f)return;
    while(fgets(line,sizeof(line),f))if(sscanf(line,"%63[^=]=%63s",key,val)==2)bw_setting(&candidate,key,val);
    fclose(f);if(bw_config_valid(&candidate))config=candidate;
}
static int config_save(const BWConfig *c) {
    FILE *f=fopen("profile.ini.tmp","wb");int good;
    if(!f)return 0;
    fprintf(f,"poll_hz=%d\nslot=%d\ndirectinput=%d\nsteering_axis=%d\npedal_axis=%d\n",c->poll_hz,c->slot,c->directinput,c->steering_axis,c->pedal_axis);
    for(int i=0;i<6;++i) {
        const BWAxisConfig *a=&c->axis[i];
        fprintf(f,"axis_source_%d=%d\naxis_invert_%d=%d\naxis_deadzone_%d=%.6f\naxis_limit_%d=%.6f\naxis_gamma_%d=%.6f\naxis_center_%d=%.6f\n",i,a->source,i,a->invert,i,a->deadzone,i,a->limit,i,a->gamma,i,a->center);
        if(i<4)fprintf(f,"axis_negative_%d=%d\n",i,a->negative);
    }
    for(int i=0;i<16;++i)if(i!=10&&i!=11)fprintf(f,"button_source_%d=%d\nbutton_direction_%d=%d\nbutton_threshold_%d=%.6f\n",i,c->button[i].source,i,c->button[i].direction,i,c->button[i].threshold);
    good=!ferror(f);if(fclose(f)!=0)good=0;if(!good)return 0;
#ifdef _WIN32
    return MoveFileExA("profile.ini.tmp","profile.ini",MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
#else
    return rename("profile.ini.tmp","profile.ini")==0;
#endif
}
#ifdef _WIN32
static unsigned __stdcall worker(void *unused)
#else
static void *worker(void *unused)
#endif
{
    double next=bw_now(),window=next;unsigned count=0;int old_demo=-1,old_di=-1,old_slot=-2;
    (void)unused;
    while(running) {
        BWConfig c; BWInput in, manual; BWReport out;int demo,automatic,requested,active;
        double start,elapsed;char status[256];
        LOCK();c=config;demo=demo_mode;automatic=demo_auto;manual=manual_input;requested=output_requested;UNLOCK();
        if(demo!=old_demo||c.directinput!=old_di||c.slot!=old_slot){bw_input_close();old_demo=demo;old_di=c.directinput;old_slot=c.slot;}
        memset(&in,0,sizeof(in));
        if(demo) {
            double t=bw_now();
            if(automatic){in.steering=(float)(sin(t*.62)*.67);in.throttle=(float)((sin(t*1.2)+1)*.43);in.brake=(float)fmax(0,sin(t*.8-2)*.7);in.buttons=(uint16_t)(1u<<(12+((int)t/2)%4));}
            else in=manual;
            in.connected=1;in.axes[0]=in.steering;in.axes[1]=in.ly;in.axes[2]=in.brake*2-1;in.axes[3]=in.rx;in.axes[4]=in.ry;in.axes[5]=in.throttle*2-1;
            snprintf(in.name,sizeof(in.name),"%s",bw_text("Ferrari 458 Italia · simulation"));snprintf(in.backend,sizeof(in.backend),bw_text("Demonstration %s"),automatic?bw_text("automatic"):bw_text("interactive"));
            snprintf(in.detail,sizeof(in.detail),"%s",bw_text("Synthetic inputs for interface testing. No hardware is detected in this mode; game output is blocked."));
        } else bw_input_read(&in,&c);
        start=bw_now();bw_map(&in,&c,&out);elapsed=(bw_now()-start)*1e6;
        active=bw_output(&out,requested && !demo && in.connected,status,sizeof(status));
        if(demo)snprintf(status,sizeof(status),"%s",bw_text("Demonstration · game output blocked"));
        else if(requested&&!in.connected)snprintf(status,sizeof(status),"%s",bw_text("Waiting for physical input · output neutralized"));
        ++count;
        LOCK();latest=in;report=out;map_us=elapsed;output_active=active;snprintf(output_status,sizeof(output_status),"%s",status);++packets;
        if(bw_now()-window>=1){observed_hz=count/(bw_now()-window);count=0;window=bw_now();}
        UNLOCK();
        next+=1./c.poll_hz;
        if(next<bw_now()) next=bw_now();
        bw_sleep(next-bw_now());
    }
    bw_output_close();bw_input_close();
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}
/* Bounded JSON writer; strings from device drivers are escaped. */
typedef struct {char *data;size_t used,capacity;} Writer;
static void put(Writer *w,const char *fmt,...) {
    va_list args;int n;if(w->used>=w->capacity)return;
    va_start(args,fmt);n=vsnprintf(w->data+w->used,w->capacity-w->used,fmt,args);va_end(args);
    if(n<0)return;
    w->used+=(size_t)n < w->capacity-w->used?(size_t)n:w->capacity-w->used-1;
}
static void quoted(Writer *w,const char *s) {
    put(w,"\"");for(;*s;++s){unsigned char c=(unsigned char)*s;if(c=='"'||c=='\\')put(w,"\\%c",c);else if(c<32)put(w,"\\u%04x",c);else put(w,"%c",c);}put(w,"\"");
}
static void config_json(Writer *w,const BWConfig *c,int legacy) {
    put(w,"{\"poll_hz\":%d,\"slot\":%d,\"directinput\":%d,\"steering_axis\":%d,\"pedal_axis\":%d",c->poll_hz,c->slot,c->directinput,c->steering_axis,c->pedal_axis);
    for(int i=0;i<6;++i) {
        const BWAxisConfig *a=&c->axis[i];
        put(w,",\"axis_source_%d\":%d,\"axis_invert_%d\":%d,\"axis_deadzone_%d\":%.6f,\"axis_limit_%d\":%.6f,\"axis_gamma_%d\":%.6f,\"axis_center_%d\":%.6f",i,a->source,i,a->invert,i,a->deadzone,i,a->limit,i,a->gamma,i,a->center);
        if(i<4)put(w,",\"axis_negative_%d\":%d",i,a->negative);
    }
    for(int i=0;i<16;++i)if(i!=10&&i!=11)put(w,",\"button_source_%d\":%d,\"button_direction_%d\":%d,\"button_threshold_%d\":%.6f",i,c->button[i].source,i,c->button[i].direction,i,c->button[i].threshold);
    if(legacy) {
        put(w,",\"deadzone\":%.6f,\"gamma\":%.6f,\"center\":%.6f,\"pedal_deadzone\":%.6f,\"invert_steering\":%d,\"invert_throttle\":%d,\"invert_brake\":%d,\"button_map\":[",c->axis[0].deadzone,c->axis[0].gamma,c->axis[0].center,c->axis[4].deadzone,c->axis[0].invert,c->axis[4].invert,c->axis[5].invert);
        for(int i=0;i<16;++i)put(w,"%s%d",i?",":"",c->button_map[i]);
        put(w,"]");
    }
    put(w,"}");
}
static void status_json(char *buf,size_t cap) {
    BWConfig c;BWInput in;BWReport out;char status[256];int demo,automatic,requested,active;double us,hz;unsigned long long p;int i;
    Writer w={buf,0,cap};
    LOCK();c=config;in=latest;out=report;demo=demo_mode;automatic=demo_auto;requested=output_requested;active=output_active;us=map_us;hz=observed_hz;p=packets;snprintf(status,sizeof(status),"%s",output_status);UNLOCK();
    put(&w,"{\"demo\":%d,\"demo_auto\":%d,\"connected\":%d,\"physical_connected\":%d,\"usb_present\":%d,\"combined_pedals\":%d,\"output_requested\":%d,\"output_active\":%d,\"platform\":\"%s\",\"name\":",demo,automatic,in.connected,in.connected&&!demo,in.usb_present,in.combined_pedals,requested,active,
#ifdef _WIN32
    "windows"
#else
    "linux"
#endif
    );quoted(&w,in.name);put(&w,",\"backend\":");quoted(&w,in.backend);put(&w,",\"detail\":");quoted(&w,in.detail);put(&w,",\"output_status\":");quoted(&w,status);
    put(&w,",\"map_us\":%.3f,\"observed_hz\":%.1f,\"packets\":%llu,\"raw\":{\"steering\":%.6f,\"throttle\":%.6f,\"brake\":%.6f,\"ly\":%.6f,\"rx\":%.6f,\"ry\":%.6f,\"buttons\":%u,\"axes\":[",us,hz,p,in.steering,in.throttle,in.brake,in.ly,in.rx,in.ry,in.buttons);
    for(i=0;i<6;++i)put(&w,"%s%.6f",i?",":"",in.axes[i]);
    put(&w,"]},\"report\":{\"lx\":%d,\"ly\":%d,\"rx\":%d,\"ry\":%d,\"lt\":%u,\"rt\":%u,\"buttons\":%u},",out.lx,out.ly,out.rx,out.ry,out.lt,out.rt,out.buttons);
    put(&w,"\"input\":[");
    for(i=0;i<6;++i)put(&w,"%s%.6f",i?",":"",bw_axis_input(&in,&c,i));
    put(&w,"],\"config\":");config_json(&w,&c,1);put(&w,"}");
}
static void send_all(socket_t fd,const char *buf,size_t len){while(len){int n=send(fd,buf,(int)len,0);if(n<=0)return;buf+=n;len-=(size_t)n;}}
static void response(socket_t fd,int code,const char *type,const char *body,size_t len) {
    char header[1024];int n=snprintf(header,sizeof(header),"HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %llu\r\nConnection: close\r\nCache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\nContent-Security-Policy: default-src 'self'; style-src 'self'; script-src 'self'; img-src 'self' data:; connect-src 'self'; frame-ancestors 'none'; base-uri 'none'\r\n\r\n",code,code==200?"OK":code==403?"Forbidden":code==404?"Not Found":"Bad Request",type,(unsigned long long)len);
    send_all(fd,header,(size_t)n);send_all(fd,body,len);
}
static void error_response(socket_t fd,int code,const char *msg) {response(fd,code,"text/plain; charset=utf-8",msg,strlen(msg));}
static int parse_pairs(BWConfig *c,char *body,int profile) {
    char *p=body;int count=0;char keys[100][64];
    while(p&&*p){
        char *next=strchr(p,'&'),*eq;
        if(next)*next++='\0';
        eq=strchr(p,'=');if(!eq)return 0;*eq++='\0';
        if(count>=100||strlen(p)>=sizeof(keys[0]))return 0;
        for(int i=0;i<count;++i)if(!strcmp(keys[i],p))return 0;
        if(profile && strncmp(p,"axis_",5) && strncmp(p,"button_source_",14) && strncmp(p,"button_direction_",17) && strncmp(p,"button_threshold_",17) && strcmp(p,"poll_hz") && strcmp(p,"slot") && strcmp(p,"directinput") && strcmp(p,"steering_axis") && strcmp(p,"pedal_axis"))return 0;
        if(!bw_setting(c,p,eq))return 0;
        snprintf(keys[count++],sizeof(keys[0]),"%s",p);p=next;
    }
    return (profile?count==87:count>0) && bw_config_valid(c);
}
static int demo_values(char *body) {
    BWInput tmp={0};char *p=body;
    while(p&&*p){char *next=strchr(p,'&'),*eq,*tail;double v;if(next)*next++=0;eq=strchr(p,'=');if(!eq)return 0;*eq++=0;v=strtod(eq,&tail);if(!*eq||*tail||!isfinite(v))return 0;
        if(!strcmp(p,"steering")&&v>=-1&&v<=1)tmp.steering=(float)v;
        else if(!strcmp(p,"throttle")&&v>=0&&v<=1)tmp.throttle=(float)v;
        else if(!strcmp(p,"brake")&&v>=0&&v<=1)tmp.brake=(float)v;
        else if(!strcmp(p,"ly")&&v>=-1&&v<=1)tmp.ly=(float)v;
        else if(!strcmp(p,"rx")&&v>=-1&&v<=1)tmp.rx=(float)v;
        else if(!strcmp(p,"ry")&&v>=-1&&v<=1)tmp.ry=(float)v;
        else if(!strncmp(p,"raw_",4)&&strlen(p)==5&&p[4]>='0'&&p[4]<='5'&&v>=-1&&v<=1)tmp.axes[p[4]-'0']=(float)v;
        else if(!strcmp(p,"buttons")&&v>=0&&v<=65535&&v==floor(v))tmp.buttons=(uint16_t)v;
        else return 0;
        p=next;
    }
    LOCK();manual_input=tmp;UNLOCK();return 1;
}
static void handle(socket_t fd) {
    char request[32768],method[12],path[128],host[96]="",origin[128]="",ctype[96]="";size_t used=0,body_len=0;char *end,*line,*body;int n;
#ifdef _WIN32
    DWORD timeout=1000;setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
#else
    struct timeval timeout={1,0};setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
#endif
    while(used<sizeof(request)-1){n=recv(fd,request+used,(int)(sizeof(request)-1-used),0);if(n<=0)return;used+=(size_t)n;request[used]=0;if(strstr(request,"\r\n\r\n"))break;}
    end=strstr(request,"\r\n\r\n");if(!end){error_response(fd,400,bw_text("Request too large"));return;}body=end+4;
    if(sscanf(request,"%11s %127s",method,path)!=2){error_response(fd,400,bw_text("Invalid request"));return;}
    /* Parse headers case insensitively; reject chunking and ambiguous lengths. */
    int length_seen=0;
    for(line=strstr(request,"\r\n")+2;line<end;) {
        char *nl=strstr(line,"\r\n"),*colon=strchr(line,':'),*value;
        if(!nl||!colon||colon>nl){error_response(fd,400,bw_text("Invalid header"));return;}
        *nl=0;*colon=0;value=colon+1;while(*value==' '||*value=='\t')++value;
        for(char *k=line;*k;++k)if(*k>='A'&&*k<='Z')*k=(char)(*k+32);
        if(!strcmp(line,"host"))snprintf(host,sizeof(host),"%s",value);
        else if(!strcmp(line,"origin"))snprintf(origin,sizeof(origin),"%s",value);
        else if(!strcmp(line,"content-type"))snprintf(ctype,sizeof(ctype),"%s",value);
        else if(!strcmp(line,"transfer-encoding")){error_response(fd,400,bw_text("Chunking unsupported"));return;}
        else if(!strcmp(line,"content-length")){char *tail;unsigned long len=strtoul(value,&tail,10);if(length_seen++||!*value||*tail||len>16384){error_response(fd,400,bw_text("Invalid length"));return;}body_len=(size_t)len;}
        line=nl+2;
    }
    char expected_host[64],expected_origin[96];snprintf(expected_host,sizeof(expected_host),"127.0.0.1:%d",port);snprintf(expected_origin,sizeof(expected_origin),"http://%s",expected_host);
    if(strcmp(host,expected_host)){error_response(fd,403,bw_text("Use the 127.0.0.1 address printed in the terminal"));return;}
    size_t offset=(size_t)(body-request);
    if(offset+body_len>=sizeof(request)){error_response(fd,400,bw_text("Request too large"));return;}
    while(used<offset+body_len){n=recv(fd,request+used,(int)(offset+body_len-used),0);if(n<=0)return;used+=(size_t)n;}body[body_len]=0;
    if(!strcmp(method,"GET")&&!strcmp(path,"/api/state")){char json[16384];status_json(json,sizeof(json));response(fd,200,"application/json",json,strlen(json));return;}
    if(!strcmp(method,"GET")&&!strcmp(path,"/api/profile")) {
        char json[16384];Writer w={json,0,sizeof(json)};BWConfig c;
        LOCK();c=config;UNLOCK();put(&w,"{\"format\":\"boreal-wheel\",\"version\":2,\"settings\":");config_json(&w,&c,0);put(&w,"}");
        response(fd,200,"application/json",json,strlen(json));return;
    }
    if(!strcmp(method,"POST")) {
        BWConfig c;
        if(strcmp(origin,expected_origin)||strcmp(ctype,"application/x-www-form-urlencoded")){error_response(fd,403,bw_text("Same-origin form requests required"));return;}
        LOCK();c=config;UNLOCK();
        if(!strcmp(path,"/api/config")||!strcmp(path,"/api/profile")) {
            if(!strcmp(path,"/api/profile"))bw_defaults(&c);
            if(!parse_pairs(&c,body,!strcmp(path,"/api/profile"))){error_response(fd,400,bw_text("Invalid setting"));return;}
            if(!config_save(&c)){error_response(fd,400,bw_text("Cannot save profile.ini"));return;}
            LOCK();config=c;UNLOCK();
        } else if(!strcmp(path,"/api/demo")) {if(!demo_values(body)){error_response(fd,400,bw_text("Invalid demo input"));return;}}
        else if(!strcmp(path,"/api/action")) {
            if(!strcmp(body,"action=reset")) {
                bw_defaults(&c);if(!config_save(&c)){error_response(fd,400,bw_text("Cannot save profile"));return;}LOCK();config=c;UNLOCK();
            } else if(!strcmp(body,"action=center") || (!strncmp(body,"action=center_",14)&&strlen(body)==15&&body[14]>='0'&&body[14]<='3')) {
                int axis=strlen(body)==15?body[14]-'0':0;
                LOCK();BWInput in=latest;UNLOCK();BWConfig temporary=c;temporary.axis[axis].center=0;float center=bw_axis_input(&in,&temporary,axis);int available=in.connected&&c.axis[axis].source>=0;
                if(!available){error_response(fd,400,bw_text("No input available for calibration"));return;}
                if(fabsf(center)>.5f){error_response(fd,400,bw_text("Keep the control near center before capturing"));return;}
                c.axis[axis].center=center;if(!config_save(&c)){error_response(fd,400,bw_text("Cannot save profile"));return;}LOCK();config=c;UNLOCK();
            } else if(!strncmp(body,"action=limit_",13)&&strlen(body)==14&&body[13]>='0'&&body[13]<='5') {
                int axis=body[13]-'0';LOCK();BWInput in=latest;UNLOCK();
                float value=fabsf(bw_axis_input(&in,&c,axis));
                if(!in.connected||c.axis[axis].source<0||value<.05f||value<c.axis[axis].deadzone+.02f){error_response(fd,400,bw_text("Hold the control at full travel before capturing"));return;}
                c.axis[axis].limit=value;
                if(!config_save(&c)){error_response(fd,400,bw_text("Cannot save profile"));return;}LOCK();config=c;UNLOCK();
            } else {
                LOCK();
                if(!strcmp(body,"action=demo")){demo_mode=1;output_requested=0;demo_auto=1;}
                else if(!strcmp(body,"action=physical")){demo_mode=0;output_requested=0;}
                else if(!strcmp(body,"action=auto")){demo_auto=1;}
                else if(!strcmp(body,"action=manual")){demo_auto=0;memset(&manual_input,0,sizeof(manual_input));}
                else if(!strcmp(body,"action=start")){if(demo_mode){UNLOCK();error_response(fd,400,bw_text("Demo cannot feed games"));return;}output_requested=1;}
                else if(!strcmp(body,"action=stop"))output_requested=0;
                else {UNLOCK();error_response(fd,400,bw_text("Unknown action"));return;}
                UNLOCK();
            }
        } else {error_response(fd,404,bw_text("Not found"));return;}
        response(fd,200,"application/json","{\"ok\":true}",11);return;
    }
    if(strcmp(method,"GET")){error_response(fd,400,bw_text("Unsupported method"));return;}
    const char *file=NULL,*type=NULL;
    if(!strcmp(path,"/")){file="web/index.html";type="text/html; charset=utf-8";}
    else if(!strcmp(path,"/style.css")){file="web/style.css";type="text/css; charset=utf-8";}
    else if(!strcmp(path,"/app.js")){file="web/app.js";type="text/javascript; charset=utf-8";}
    else if(!strcmp(path,"/i18n.js")){file="web/i18n.js";type="text/javascript; charset=utf-8";}
    else if(!strcmp(path,"/locales.js")){file="web/locales.js";type="text/javascript; charset=utf-8";}
    else if(!strcmp(path,"/favicon.svg")){file="web/favicon.svg";type="image/svg+xml";}
    if(!file){error_response(fd,404,bw_text("Not found"));return;}
    FILE *f=fopen(file,"rb");if(!f){error_response(fd,404,bw_text("UI assets missing; start from boreal-wheel"));return;}
    if(fseek(f,0,SEEK_END)!=0){fclose(f);return;}long len=ftell(f);rewind(f);
    if(len<0||len>1024*1024){fclose(f);return;}char *data=malloc((size_t)len+1);if(!data){fclose(f);return;}
    size_t got=fread(data,1,(size_t)len,f);fclose(f);response(fd,200,type,data,got);free(data);
}
int main(int argc,char **argv) {
    int i;socket_t listener;struct sockaddr_in address;
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    if(getenv("BW_LANG") && !bw_set_language(getenv("BW_LANG"))){fprintf(stderr,"%s",bw_text("Invalid language. Use en or pt-BR.\n"));return 1;}
    /* Choose the process language before validating other arguments. */
    for(i=1;i<argc;++i)if(!strcmp(argv[i],"--lang")){
        if(i+1>=argc||!bw_set_language(argv[i+1])){fprintf(stderr,"%s",bw_text("Invalid language. Use en or pt-BR.\n"));return 1;}
        ++i;
    }
    bw_defaults(&config);config_load();
    for(i=1;i<argc;++i){if(!strcmp(argv[i],"--lang")){++i;}else if(!strcmp(argv[i],"--demo"))demo_mode=1;else if(!strcmp(argv[i],"--port")&&i+1<argc){char *tail;long p=strtol(argv[++i],&tail,10);if(*tail||p<1024||p>65535){fprintf(stderr,"%s",bw_text("Invalid port\n"));return 1;}port=(int)p;}else{fprintf(stderr,"%s",bw_text("Usage: boreal-wheel [--demo] [--port 4580] [--lang en|pt-BR]\n"));return 1;}}
    signal(SIGINT,stop_signal);signal(SIGTERM,stop_signal);
#ifdef _WIN32
    WSADATA data;if(WSAStartup(MAKEWORD(2,2),&data))return 1;InitializeCriticalSection(&mutex);
#else
    signal(SIGPIPE,SIG_IGN);
#endif
    listener=socket(AF_INET,SOCK_STREAM,0);if(listener==INVALID_SOCKET){perror("socket");return 1;}
#ifndef _WIN32
    int reuse=1;setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
#else
    int exclusive=1;setsockopt(listener,SOL_SOCKET,SO_EXCLUSIVEADDRUSE,(const char*)&exclusive,sizeof(exclusive));
#endif
    memset(&address,0,sizeof(address));address.sin_family=AF_INET;address.sin_port=htons((unsigned short)port);address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    if(bind(listener,(struct sockaddr*)&address,sizeof(address))!=0||listen(listener,16)!=0){fprintf(stderr,bw_text("Port %d unavailable. Try --port 4581.\n"),port);CLOSESOCKET(listener);return 1;}
#ifdef _WIN32
    HANDLE thread=(HANDLE)_beginthreadex(NULL,0,worker,NULL,0,NULL);if(!thread){CLOSESOCKET(listener);return 1;}
#else
    pthread_t thread;if(pthread_create(&thread,NULL,worker,NULL)){CLOSESOCKET(listener);return 1;}
#endif
    printf(bw_text("\n  BOREAL WHEEL / Input, reconnected.\n  http://127.0.0.1:%d\n  Mode: %s\n  Ctrl+C to stop.\n\n"),port,demo_mode?bw_text("DEMONSTRATION (no hardware detection)"):bw_text("PHYSICAL INPUT"));fflush(stdout);
    while(running){fd_set readset;struct timeval wait={0,200000};FD_ZERO(&readset);FD_SET(listener,&readset);if(select((int)listener+1,&readset,NULL,NULL,&wait)>0){socket_t fd=accept(listener,NULL,NULL);if(fd!=INVALID_SOCKET){handle(fd);CLOSESOCKET(fd);}}}
    CLOSESOCKET(listener);
#ifdef _WIN32
    WaitForSingleObject(thread,INFINITE);CloseHandle(thread);DeleteCriticalSection(&mutex);WSACleanup();
#else
    pthread_join(thread,NULL);
#endif
    return 0;
}
