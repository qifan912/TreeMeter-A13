#include "laser.h"
#include <string.h>
uint32_t laser_frames,laser_crc_errors,laser_measure_frames,laser_last_ms;
unsigned int laser_mm,laser_range_valid;
char laser_version[33];
uint32_t laser_stop_acks;
unsigned int laser_stop_value=0xFFU;
unsigned int laser_last_cmd,laser_last_len,laser_start_ack=0xFFU;
static uint8_t frame[73];
static unsigned int used;
static uint32_t byte_ms;
uint16_t laser_crc(const uint8_t *p,unsigned int n)
{
    uint16_t crc=0xFFFFU;
    unsigned int i;
    while(n--) {crc^=*p++;for(i=0;i<8U;i++) crc=(uint16_t)((crc>>1)^((crc&1U)?0xA001U:0U));}
    return crc;
}
static void drop(unsigned int n)
{
    used-=n; memmove(frame,frame+n,used);
}
void laser_feed(uint8_t b,uint32_t now)
{
    unsigned int len,total,i;
    uint16_t crc,be,le;
    if((uint32_t)(now-byte_ms)>100U) used=0U;
    byte_ms=now;
    if(used==sizeof(frame)) drop(1U);
    frame[used++]=b;
    for(;;) {
        if(!used) return;
        if(frame[0]!=0xA5U) {drop(1U);continue;}
        if(used<3U) return;
        if(frame[1]!=3U || frame[2]!=0x20U) {drop(1U);continue;}
        if(used<7U) return;
        len=(unsigned int)frame[5]*256U+frame[6];
        if(frame[4]!=0U || len>64U) {drop(1U);continue;}
        total=len+9U;
        if(used<total) return;
        crc=laser_crc(frame,total-2U);
        be=(uint16_t)((unsigned int)frame[total-2U]*256U+frame[total-1U]);
        le=(uint16_t)((unsigned int)frame[total-1U]*256U+frame[total-2U]);
        if(crc!=be && crc!=le) {laser_crc_errors++;drop(1U);continue;}
        laser_frames++;
        if(frame[3]==2U && len==1U) {laser_stop_acks++;laser_stop_value=frame[7];}
        laser_last_cmd=frame[3];laser_last_len=len;
        if(frame[3]==1U && len==1U) laser_start_ack=frame[7];
        if(frame[3]==1U && len==14U) {
            laser_mm=(unsigned int)frame[13]+(unsigned int)frame[14]*256U;
            laser_range_valid=(laser_mm>=200U && laser_mm<=20000U);
            laser_measure_frames++;laser_last_ms=now;
        } else if(frame[3]==0x0AU) {
            for(i=0U;i<len && i<32U;i++) laser_version[i]=(frame[7U+i]>=32U && frame[7U+i]<=126U)?(char)frame[7U+i]:'?';
            laser_version[i]='\0';
        }
        drop(total);
    }
}
