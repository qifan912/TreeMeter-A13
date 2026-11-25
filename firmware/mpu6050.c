#include "mpu6050.h"
#include "mpu_bus.h"
#include <math.h>
volatile unsigned int mpu_address,mpu_id,mpu_status;
volatile int16_t mpu_ax,mpu_ay,mpu_az,mpu_gx,mpu_gy,mpu_gz;
volatile float mpu_pitch,mpu_roll;
static float fx,fy,fz;
static unsigned int filter_ready;
void tilt_angles(float ax,float ay,float az,float *pitch,float *roll)
{
    *pitch=atan2f(-ax,sqrtf(ay*ay+az*az))*57.2957795f;
    *roll=atan2f(ay,az)*57.2957795f;
}
static int16_t signed_word(const uint8_t *p)
{
    int32_t v=(int32_t)p[0]*256+p[1];
    if (v>=32768) v-=65536;
    return (int16_t)v;
}
unsigned int mpu_init(void)
{
    uint8_t id=0U;
    unsigned int a,s;
    const uint8_t regs[]={0x6B,0x6C,0x1A,0x19,0x1B,0x1C};
    const uint8_t vals[]={0x01,0x00,0x03,0x09,0x00,0x00};
    mpu_address=0U; mpu_id=0U; filter_ready=0U;
    mpu_status=mpu_bus_init();
    if (mpu_status) return mpu_status;
    for(a=0x68U;a<=0x69U;a++) {
        s=mpu_bus_read((uint8_t)a,0x75,&id,1U);
        if (!s) {
            mpu_id=id;
            if (id==0x68U) { mpu_address=a; break; }
            mpu_status=3U;
        } else if (mpu_status!=3U) mpu_status=s;
    }
    if (!mpu_address) return mpu_status;
    mpu_status=mpu_bus_write((uint8_t)mpu_address,0x6B,0x80);
    if(mpu_status) return mpu_status;
    mpu_delay_ms(100U);
    for(a=0U;a<sizeof(regs);a++) {
        mpu_status=mpu_bus_write((uint8_t)mpu_address,regs[a],vals[a]);
        if(mpu_status) return mpu_status;
    }
    mpu_delay_ms(100U);
    return 0U;
}
unsigned int mpu_sample(void)
{
    uint8_t b[14];
    float p,r,n;
    mpu_status=mpu_bus_read((uint8_t)mpu_address,0x3B,b,14U);
    if(mpu_status) { filter_ready=0U; return mpu_status; }
    mpu_ax=signed_word(b); mpu_ay=signed_word(b+2); mpu_az=signed_word(b+4);
    mpu_gx=signed_word(b+8); mpu_gy=signed_word(b+10); mpu_gz=signed_word(b+12);
    n=(float)mpu_ax*mpu_ax+(float)mpu_ay*mpu_ay+(float)mpu_az*mpu_az;
    if(n<1000000.0f) { filter_ready=0U; mpu_status=4U; return 4U; }
    if(!filter_ready) {fx=mpu_ax;fy=mpu_ay;fz=mpu_az;filter_ready=1U;}
    else {fx+=0.2f*(mpu_ax-fx);fy+=0.2f*(mpu_ay-fy);fz+=0.2f*(mpu_az-fz);}
    tilt_angles(fx,fy,fz,&p,&r); mpu_pitch=p;mpu_roll=r;
    return 0U;
}
