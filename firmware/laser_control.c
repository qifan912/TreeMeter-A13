#include "laser.h"
unsigned int laser_attempt,laser_tx_low;
unsigned int key_state,key_result_mm,key_result_valid;
static uint32_t phase_ms,start_ms,seen_frames,quiet_ms,ack_base;
static unsigned int stop_step,after_stop,candidate,stable,debounce,armed;
static void stop_begin(uint32_t now,unsigned int next)
{
    key_state=KEY_STOPPING;after_stop=next;phase_ms=now;quiet_ms=now;
    ack_base=laser_stop_acks;seen_frames=laser_measure_frames;stop_step=0U;
    laser_command(2U,0U);
}
void laser_control_init(uint32_t now)
{
    key_state=KEY_BOOT;key_result_valid=0U;key_result_mm=0U;
    laser_attempt=0U;laser_tx_low=0U;phase_ms=now;
    candidate=stable=debounce=armed=0U;
}
/* Called every 10ms: require 3 equal samples; release before each press.
   Boot-held and busy-held buttons cannot trigger a later measurement. */
void key_input(unsigned int pressed,uint32_t now)
{
    pressed=pressed?1U:0U;
    if(pressed!=candidate) {candidate=pressed;debounce=1U;}
    else if(debounce<3U) debounce++;
    if(debounce<3U) return;
    if(!pressed) {stable=0U;armed=1U;return;}
    if(stable) return;
    stable=1U;
    if(!armed) return;
    armed=0U;
    if(key_state==KEY_STOP_FAIL) {stop_begin(now,KEY_READY);return;}
    if(key_state!=KEY_READY && key_state!=KEY_DONE && key_state!=KEY_TIMEOUT) return;
    key_result_valid=0U;seen_frames=laser_measure_frames;start_ms=now;
    key_state=KEY_MEASURING;laser_tx_low=0U;laser_attempt=1U;
    laser_command(1U,0U);
}
void laser_control_poll(uint32_t now)
{
    uint32_t elapsed;
    if(key_state==KEY_BOOT) {
        if((uint32_t)(now-phase_ms)>=1000U) stop_begin(now,KEY_READY);
        return;
    }
    if(key_state==KEY_STOPPING) {
        elapsed=(uint32_t)(now-phase_ms);
        if(laser_measure_frames!=seen_frames) {seen_frames=laser_measure_frames;quiet_ms=now;}
        /* Two separated attempts for each documented CRC order. */
        if(stop_step==0U && elapsed>=250U) {laser_command(2U,1U);stop_step=1U;}
        else if(stop_step==1U && elapsed>=750U) {laser_command(2U,0U);stop_step=2U;}
        else if(stop_step==2U && elapsed>=1000U) {laser_command(2U,1U);stop_step=3U;}
        if(elapsed>=750U && laser_stop_acks!=ack_base && laser_stop_value==0U &&
           (uint32_t)(now-quiet_ms)>=500U) {key_state=after_stop;return;}
        if(elapsed>=2500U) key_state=KEY_STOP_FAIL;
        return;
    }
    if(key_state!=KEY_MEASURING) {
        /* Late/unexpected streaming after a stop triggers another stop attempt. */
        if(key_state!=KEY_STOP_FAIL && laser_measure_frames!=seen_frames)
            stop_begin(now,key_state);
        return;
    }
    if(laser_measure_frames!=seen_frames) {
        seen_frames=laser_measure_frames;
        if(laser_range_valid) {
            key_result_mm=laser_mm;key_result_valid=1U;
            stop_begin(now,KEY_DONE);return;
        }
    }
    elapsed=(uint32_t)(now-start_ms);
    if(elapsed>=4000U) {stop_begin(now,KEY_TIMEOUT);return;}
    if(elapsed>=1500U && laser_attempt==1U) {
        laser_tx_low=1U;laser_attempt=2U;laser_command(1U,1U);
    }
}
