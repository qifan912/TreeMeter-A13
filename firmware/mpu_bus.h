#ifndef MPU_BUS_H
#define MPU_BUS_H
#include <stdint.h>
unsigned int mpu_bus_init(void);
void mpu_delay_ms(unsigned int ms);
unsigned int mpu_bus_write(uint8_t addr,uint8_t reg,uint8_t value);
unsigned int mpu_bus_read(uint8_t addr,uint8_t reg,uint8_t *data,unsigned int n);
#endif
