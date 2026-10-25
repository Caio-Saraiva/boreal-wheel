#define _POSIX_C_SOURCE 200809L
#include "bridge.h"
#include <linux/input.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
static int wheel_fd=-1;
static double next_scan;
static BWInput device;
double bw_now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return (double)t.tv_sec+t.tv_nsec/1e9; }
void bw_sleep(double seconds) {
    struct timespec t;
    if (seconds<=0) return;
    t.tv_sec=(time_t)seconds; t.tv_nsec=(long)((seconds-(double)t.tv_sec)*1e9);
    while (nanosleep(&t,&t)<0 && errno==EINTR) {}
}
static float axis_value(int code, int pedal) {
    struct input_absinfo a;
    if (ioctl(wheel_fd,EVIOCGABS(code),&a)<0 || a.maximum==a.minimum) return 0.f;
    float t=(float)(a.value-a.minimum)/(float)(a.maximum-a.minimum);
    return pedal ? t : t*2.f-1.f;
}
static uint16_t button_bit(int code) {
    switch(code) {
    case BTN_SOUTH: return 0x1000; case BTN_EAST: return 0x2000;
    case BTN_WEST: return 0x4000; case BTN_NORTH: return 0x8000;
    case BTN_TL: return 0x100; case BTN_TR: return 0x200;
    case BTN_SELECT:return 0x20; case BTN_START:return 0x10;
    case BTN_THUMBL:return 0x40; case BTN_THUMBR:return 0x80;
    case BTN_DPAD_UP:return 1; case BTN_DPAD_DOWN:return 2;
    case BTN_DPAD_LEFT:return 4; case BTN_DPAD_RIGHT:return 8;
    default: return 0;
    }
}
static void read_snapshot(void) {
    unsigned long keys[(KEY_MAX+8*sizeof(unsigned long))/(8*sizeof(unsigned long))];
    int i; memset(keys,0,sizeof(keys));
    device.steering=axis_value(ABS_X,0); device.throttle=axis_value(ABS_RZ,1); device.brake=axis_value(ABS_Z,1);
    device.ly=-axis_value(ABS_Y,0); device.rx=axis_value(ABS_RX,0); device.ry=-axis_value(ABS_RY,0);
    device.axes[0]=device.steering; device.axes[1]=device.ly;
    device.axes[2]=device.brake*2-1; device.axes[3]=device.rx; device.axes[4]=device.ry; device.axes[5]=device.throttle*2-1;
    device.buttons=0;
    if (ioctl(wheel_fd,EVIOCGKEY(sizeof(keys)),keys)>=0) {
        for(i=BTN_MISC;i<=KEY_MAX;++i) if (keys[i/(8*sizeof(unsigned long))] & (1ul<<(i%(8*sizeof(unsigned long))))) device.buttons |= button_bit(i);
    }
    struct input_absinfo a;
    if(ioctl(wheel_fd,EVIOCGABS(ABS_HAT0X),&a)>=0) device.buttons |= a.value<0?4:a.value>0?8:0;
    if(ioctl(wheel_fd,EVIOCGABS(ABS_HAT0Y),&a)>=0) device.buttons |= a.value<0?1:a.value>0?2:0;
}
void bw_input_read(BWInput *in,const BWConfig *c) {
    (void)c;
    if(wheel_fd<0 && bw_now()>=next_scan) {
        int i;
        next_scan=bw_now()+1;
        memset(&device,0,sizeof(device));
        for(i=0;i<64;++i) {
            char path[160];unsigned vendor=0,product=0;FILE *sys;int fd;struct input_id id;
            snprintf(path,sizeof(path),"/sys/class/input/event%d/device/id/vendor",i);
            sys=fopen(path,"r");if(!sys)continue;if(fscanf(sys,"%x",&vendor)!=1)vendor=0;fclose(sys);
            if(vendor!=0x24c6)continue;
            snprintf(path,sizeof(path),"/sys/class/input/event%d/device/id/product",i);
            sys=fopen(path,"r");if(!sys)continue;if(fscanf(sys,"%x",&product)!=1)product=0;fclose(sys);
            if(product!=0x5b00 && product!=0x5b03)continue;
            device.usb_present=1;
            snprintf(device.name,sizeof(device.name),"Thrustmaster Ferrari 458 Italia · USB %04X:%04X",vendor,product);
            snprintf(path,sizeof(path),"/dev/input/event%d",i);
            fd=open(path,O_RDONLY|O_NONBLOCK);
            if(fd<0) {
                snprintf(device.detail,sizeof(device.detail),bw_text("USB detected, but cannot read %s. Check Linux permissions in the README."),path);
                continue;
            }
            if(ioctl(fd,EVIOCGID,&id)<0 || id.vendor!=0x24c6 || (id.product!=0x5b00 && id.product!=0x5b03)){close(fd);continue;}
            wheel_fd=fd;device.connected=1;
            ioctl(fd,EVIOCGNAME(sizeof(device.name)),device.name);
            snprintf(device.backend,sizeof(device.backend),"Linux evdev / xpad");
            snprintf(device.detail,sizeof(device.detail),"USB %04X:%04X · %s",id.vendor,id.product,path);
            read_snapshot();break;
        }
        if(!device.usb_present)snprintf(device.detail,sizeof(device.detail),"%s",bw_text("Connect the Ferrari 458 Italia Xbox 360 by USB. Linux input requires the xpad module."));
    }
    if(wheel_fd>=0) {
        struct input_event ev[32]; ssize_t n;
        int changed=0;
        while((n=read(wheel_fd,ev,sizeof(ev)))>0) changed=1;
        if(n==0 || (n<0 && errno!=EAGAIN && errno!=EWOULDBLOCK)) {
            bw_input_close(); memset(in,0,sizeof(*in)); return;
        }
        if(changed) read_snapshot();
        *in=device;
    } else *in=device;
}
void bw_input_close(void) { if(wheel_fd>=0)close(wheel_fd); wheel_fd=-1;next_scan=0;memset(&device,0,sizeof(device)); }
int bw_output(const BWReport *report,int enabled,char *status,int capacity) {
    (void)report; (void)enabled;
    snprintf(status,(size_t)capacity,"%s",bw_text("Linux preview · XInput output is only available on Windows")); return 0;
}
void bw_output_close(void) {}
