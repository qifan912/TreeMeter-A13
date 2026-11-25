#ifndef LASER_H
#define LASER_H
#include <stdint.h>
extern volatile uint32_t laser_rx_bytes,laser_uart_errors,laser_overflows;
extern uint32_t laser_frames,laser_crc_errors,laser_measure_frames,laser_last_ms;
extern unsigned int laser_mm,laser_range_valid;
extern char laser_version[33];
extern unsigned int laser_last_cmd,laser_last_len,laser_start_ack;
extern unsigned int laser_attempt,laser_tx_low;
extern uint32_t laser_stop_acks;
extern unsigned int laser_stop_value;
enum {KEY_BOOT,KEY_STOPPING,KEY_READY,KEY_MEASURING,KEY_DONE,KEY_TIMEOUT,KEY_STOP_FAIL};
extern unsigned int key_state,key_result_mm,key_result_valid;
void key_input(unsigned int pressed,uint32_t now);
void laser_control_init(uint32_t now);
void laser_control_poll(uint32_t now);
uint16_t laser_crc(const uint8_t *p,unsigned int n);
void laser_feed(uint8_t b,uint32_t now);
void laser_init(void);
void laser_poll(uint32_t now);
void laser_command(uint8_t cmd,unsigned int low_first);
#endif
