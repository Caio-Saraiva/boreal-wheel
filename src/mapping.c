#include "bridge.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
static float clampf(float x,float lo,float hi) { return x<lo?lo:x>hi?hi:x; }
static int valid_bit(int bit) { return bit>=0 && bit<16 && bit!=10 && bit!=11; }
static int valid_source(int source) {
    return source>=-1 && source<BW_SOURCES && source!=16 && source!=17;
}
void bw_defaults(BWConfig *c) {
    memset(c,0,sizeof(*c));
    c->poll_hz=250;c->slot=-1;c->pedal_axis=2;
    for(int i=0;i<BW_AXES;++i) {
        c->axis[i].source=i;c->axis[i].negative=-1;
        c->axis[i].deadzone=i<4?.02f:.01f;
        c->axis[i].limit=1;c->axis[i].gamma=1;
    }
    for(int i=0;i<BW_BUTTONS;++i) {
        c->button_map[i]=valid_bit(i)?i:-1;
        c->button[i].source=valid_bit(i)?6+i:-1;
        c->button[i].direction=1;c->button[i].threshold=.5f;
    }
}
int bw_config_valid(const BWConfig *c) {
    for(int i=0;i<BW_AXES;++i) {
        const BWAxisConfig *a=&c->axis[i];
        if(a->limit-a->deadzone<.01999f)return 0;
    }
    return 1;
}
float bw_source(const BWInput *in,int source) {
    if(!in->connected)return 0;
    switch(source) {
    case 0:return in->steering;case 1:return in->ly;
    case 2:return in->rx;case 3:return in->ry;
    case 4:return in->throttle;case 5:return in->brake;
    default:
        if(source>=6 && source<22)return (in->buttons & (1u<<(source-6)))?1.f:0.f;
        if(source>=22 && source<BW_SOURCES)return in->axes[source-22];
        return 0;
    }
}
float bw_axis_input(const BWInput *in,const BWConfig *c,int index) {
    const BWAxisConfig *a=&c->axis[index];
    if(!in->connected || a->source<0)return 0;
    float x=bw_source(in,a->source);
    if(a->negative>=0)x-=bw_source(in,a->negative);
    /* Pedal sources and digital buttons have a rest position at zero. */
    int unipolar=a->negative<0 && a->source>=4 && a->source<22;
    if(a->invert)x=index>=4&&unipolar?1-x:-x;
    x-=a->center;
    /* Preserve both endpoints even when the captured center is offset. */
    x/=x<0?1+a->center:1-a->center;
    return clampf(x,index<4?-1.f:0.f,1.f);
}
float bw_axis_map(float value,const BWAxisConfig *a) {
    float magnitude=fabsf(value),span=a->limit-a->deadzone;
    if(magnitude<=a->deadzone || span<=0)return 0;
    magnitude=powf(clampf((magnitude-a->deadzone)/span,0,1),a->gamma);
    return copysignf(magnitude,value);
}
float bw_steering(float value,const BWConfig *c) {
    BWInput in={0};in.connected=1;in.steering=value;
    return bw_axis_map(bw_axis_input(&in,c,0),&c->axis[0]);
}
static int16_t stick(float x) { return (int16_t)lroundf(clampf(x,-1,1)*(x<0?32768.f:32767.f)); }
void bw_map(const BWInput *in,const BWConfig *c,BWReport *out) {
    float a[BW_AXES];
    memset(out,0,sizeof(*out));
    if(!in->connected)return;
    for(int i=0;i<BW_AXES;++i)a[i]=c->axis[i].source<0?0:bw_axis_map(bw_axis_input(in,c,i),&c->axis[i]);
    out->lx=stick(a[0]);out->ly=stick(a[1]);out->rx=stick(a[2]);out->ry=stick(a[3]);
    out->rt=(uint8_t)lroundf(a[4]*255);out->lt=(uint8_t)lroundf(a[5]*255);
    for(int i=0;i<BW_BUTTONS;++i) {
        const BWButtonConfig *b=&c->button[i];
        if(valid_bit(i) && b->source>=0 && bw_source(in,b->source)*b->direction>=b->threshold)
            out->buttons|=(uint16_t)(1u<<i);
    }
}
/* Exact suffix parsing rejects malformed/negative array indexes. */
static int index_key(const char *key,const char *prefix,int count) {
    size_t len=strlen(prefix);char *tail;long index;
    if(strncmp(key,prefix,len)||key[len]<'0'||key[len]>'9')return -1;
    if(key[len]=='0'&&key[len+1])return -1;
    index=strtol(key+len,&tail,10);
    return !*tail && index>=0 && index<count?(int)index:-1;
}
int bw_setting(BWConfig *c,const char *key,const char *value) {
    char *end;double d;int n,index;
    if(!*value)return 0;
    d=strtod(value,&end);if(*end || !isfinite(d))return 0;
    /* Read profiles from the first release, including its button mappings. */
    if(!strcmp(key,"deadzone"))key="axis_deadzone_0";
    else if(!strcmp(key,"gamma"))key="axis_gamma_0";
    else if(!strcmp(key,"center"))key="axis_center_0";
    else if(!strcmp(key,"invert_steering"))key="axis_invert_0";
    else if(!strcmp(key,"invert_throttle"))key="axis_invert_4";
    else if(!strcmp(key,"invert_brake"))key="axis_invert_5";
    else if(!strcmp(key,"pedal_deadzone")) {
        if(d<0||d>.6)return 0;
        c->axis[4].deadzone=c->axis[5].deadzone=(float)d;return 1;
    }
    if((index=index_key(key,"axis_deadzone_",6))>=0 && d>=0 && d<=.6)c->axis[index].deadzone=(float)d;
    else if((index=index_key(key,"axis_limit_",6))>=0 && d>=.05 && d<=1)c->axis[index].limit=(float)d;
    else if((index=index_key(key,"axis_gamma_",6))>=0 && d>=.2 && d<=3)c->axis[index].gamma=(float)d;
    else if((index=index_key(key,"axis_center_",6))>=0 && d>=-.5 && d<=.5)c->axis[index].center=(float)d;
    else if((index=index_key(key,"button_threshold_",16))>=0 && valid_bit(index) && d>=.01 && d<=1)c->button[index].threshold=(float)d;
    else {
        if(d < -1 || d > 1000 || d!=floor(d))return 0;
        n=(int)d;
        if(!strcmp(key,"poll_hz") && (n==125||n==250||n==500||n==1000))c->poll_hz=n;
        else if(!strcmp(key,"slot") && n>=-1 && n<=3)c->slot=n;
        else if(!strcmp(key,"directinput") && n>=0 && n<=1)c->directinput=n;
        else if(!strcmp(key,"steering_axis") && n>=0 && n<6)c->steering_axis=n;
        else if(!strcmp(key,"pedal_axis") && n>=0 && n<6)c->pedal_axis=n;
        else if((index=index_key(key,"axis_source_",6))>=0 && valid_source(n))c->axis[index].source=n;
        else if((index=index_key(key,"axis_negative_",6))>=0 && index<4 && valid_source(n))c->axis[index].negative=n;
        else if((index=index_key(key,"axis_invert_",6))>=0 && n>=0 && n<=1)c->axis[index].invert=n;
        else if((index=index_key(key,"button_source_",16))>=0 && valid_bit(index) && valid_source(n))c->button[index].source=n;
        else if((index=index_key(key,"button_direction_",16))>=0 && valid_bit(index) && (n==-1||n==1))c->button[index].direction=n;
        else if((index=index_key(key,"button_",16))>=0 && valid_bit(index) && (n==-1||valid_bit(n))) {
            for(int i=0;i<16;++i)if(c->button[i].source==index+6)c->button[i].source=-1;
            if(n>=0)c->button[n].source=index+6;
            c->button_map[index]=n;
        } else return 0;
    }
    return 1;
}
