#include "bridge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);exit(1);} } while(0)
#define CLOSE(a,b) CHECK(fabsf((a)-(b))<.0001f)
static void calibration(void) {
    BWConfig c;BWInput in={0};BWReport out;bw_defaults(&c);in.connected=1;
    CHECK(bw_config_valid(&c));bw_map(&in,&c,&out);CHECK(out.lx==0&&out.lt==0&&out.rt==0);
    in.steering=-1;in.throttle=in.brake=1;in.buttons=0xffff;bw_map(&in,&c,&out);
    CHECK(out.lx==-32768&&out.rt==255&&out.lt==255&&out.buttons==0xf3ff);
    c.axis[0].deadzone=.1f;c.axis[0].limit=.5f;
    CLOSE(bw_steering(.1f,&c),0);CLOSE(bw_steering(.10001f,&c),.000025f);
    CLOSE(bw_steering(.3f,&c),.5f);CLOSE(bw_steering(.5f,&c),1);CLOSE(bw_steering(1,&c),1);
    c.axis[0].invert=1;CLOSE(bw_steering(.3f,&c),-.5f);c.axis[0].invert=0;
    c.axis[0].center=.15f;c.axis[0].limit=1;CLOSE(bw_steering(.15f,&c),0);
    CLOSE(bw_steering(-1,&c),-1);CLOSE(bw_steering(1,&c),1);c.axis[0].center=-.15f;CLOSE(bw_steering(-1,&c),-1);CLOSE(bw_steering(1,&c),1);c.axis[0].center=0;
    for(int gamma=2;gamma<=30;gamma++) {
        c.axis[0].gamma=gamma/10.f;float previous=-2;
        for(int i=-1000;i<=1000;i++) {float x=i/1000.f,y=bw_steering(x,&c);CHECK(y>=-1&&y<=1&&y>=previous);CLOSE(y,-bw_steering(-x,&c));previous=y;}
    }
    bw_defaults(&c);c.axis[4].deadzone=.2f;c.axis[4].limit=.6f;c.axis[5].deadzone=.1f;
    in.throttle=.4f;in.brake=.1f;bw_map(&in,&c,&out);CHECK(out.rt>=127&&out.rt<=128&&out.lt==0);
    in.throttle=.6f;bw_map(&in,&c,&out);CHECK(out.rt==255);
    c.axis[4].invert=1;in.throttle=1;bw_map(&in,&c,&out);CHECK(out.rt==0);
    in.throttle=0;bw_map(&in,&c,&out);CHECK(out.rt==255);
    c.axis[4].source=-1;bw_map(&in,&c,&out);CHECK(out.rt==0);
    in.connected=0;bw_map(&in,&c,&out);BWReport neutral={0};CHECK(!memcmp(&out,&neutral,sizeof(out)));
}
static void routing(void) {
    BWConfig c;BWInput in={0};BWReport out;bw_defaults(&c);in.connected=1;
    c.axis[4].source=5;c.axis[5].source=4;
    in.brake=1;bw_map(&in,&c,&out);CHECK(out.rt==255&&out.lt==0);
    in.brake=0;in.throttle=1;bw_map(&in,&c,&out);CHECK(out.rt==0&&out.lt==255);
    in.brake=1;bw_map(&in,&c,&out);CHECK(out.rt==255&&out.lt==255);
    bw_defaults(&c);in.throttle=in.brake=0;in.ly=.25f;in.rx=-.5f;in.ry=1;
    c.axis[0].source=3;bw_map(&in,&c,&out);CHECK(out.lx==32767&&out.ry==32767&&out.rx<0&&out.ly>0);
    c.axis[0].source=6+9;c.axis[0].negative=6+8;in.buttons=1<<8;bw_map(&in,&c,&out);CHECK(out.lx==-32768);
    in.buttons=1<<9;bw_map(&in,&c,&out);CHECK(out.lx==32767);
    in.buttons=(1<<8)|(1<<9);bw_map(&in,&c,&out);CHECK(out.lx==0);
    c.axis[4].source=6+12;in.buttons=1<<12;bw_map(&in,&c,&out);CHECK(out.rt==255);
    c.axis[0].source=18;c.axis[0].negative=-1;c.axis[0].invert=1;
    in.buttons=0;bw_map(&in,&c,&out);CHECK(out.lx==0);
    in.buttons=1<<12;bw_map(&in,&c,&out);CHECK(out.lx==-32768);
    c.axis[0].source=4;in.throttle=0;bw_map(&in,&c,&out);CHECK(out.lx==0);
    in.throttle=1;bw_map(&in,&c,&out);CHECK(out.lx==-32768);c.axis[0].invert=0;
    c.button[13].source=2;c.button[13].direction=-1;c.button[13].threshold=.5f;
    in.rx=-.49f;bw_map(&in,&c,&out);CHECK(!(out.buttons&(1<<13)));
    in.rx=-.5f;bw_map(&in,&c,&out);CHECK(out.buttons&(1<<13));
    in.rx=.8f;bw_map(&in,&c,&out);CHECK(!(out.buttons&(1<<13)));
    c.axis[0].source=23;c.axis[0].negative=-1;in.axes[1]=-.7f;
    bw_map(&in,&c,&out);CHECK(out.lx<0);CLOSE(bw_source(&in,23),-.7f);
    c.axis[5].source=24;c.axis[5].invert=1;in.axes[2]=-.5f;bw_map(&in,&c,&out);CHECK(out.lt>0&&out.lt<255);
    for(int bit=0;bit<16;bit++)if(bit!=10&&bit!=11){bw_defaults(&c);memset(&in,0,sizeof(in));in.connected=1;in.buttons=(uint16_t)(1u<<bit);bw_map(&in,&c,&out);CHECK(out.buttons==(1u<<bit));}
}
static void validation(void) {
    BWConfig c;bw_defaults(&c);
    const char *keys[]={"deadzone","axis_limit_0","axis_gamma_3","button_threshold_12","axis_source_1","axis_source_2","axis_negative_4","button_direction_3","button_source_10","button_16","button_","button_2","unknown","center"};
    const char *bad[]={"nan","0","inf","0","16","28","0","0","6","12","1","10","1","1e300"};
    for(unsigned i=0;i<sizeof(keys)/sizeof(keys[0]);i++)CHECK(!bw_setting(&c,keys[i],bad[i]));
    CHECK(bw_setting(&c,"axis_deadzone_0","0.3"));CHECK(bw_setting(&c,"axis_limit_0","0.31"));CHECK(!bw_config_valid(&c));
    CHECK(bw_setting(&c,"axis_limit_0","0.32"));CHECK(bw_config_valid(&c));
    CHECK(!bw_setting(&c,"axis_source_00","2"));CHECK(!bw_setting(&c,"axis_source_0","0.5"));CHECK(!bw_setting(&c,"slot","4"));CHECK(!bw_setting(&c,"poll_hz","999"));
    CHECK(bw_setting(&c,"pedal_deadzone","0.12"));CLOSE(c.axis[4].deadzone,.12f);CLOSE(c.axis[5].deadzone,.12f);
    CHECK(bw_setting(&c,"button_12","13"));CHECK(c.button[13].source==18&&c.button[12].source==-1);
}
int main(void){calibration();routing();validation();puts("Mapping: continuous normalized calibration, monotonic curves, endpoints, independent pedals, swaps, all axes/buttons, digital pairs, analog thresholds, legacy profiles and validation passed.");return 0;}
