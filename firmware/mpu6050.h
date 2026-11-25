#ifndef MPU6050_H
#define MPU6050_H
#include <stdint.h>
extern volatile unsigned int mpu_address, mpu_id, mpu_status;
extern volatile int16_t mpu_ax,mpu_ay,mpu_az,mpu_gx,mpu_gy,mpu_gz;
extern volatile float mpu_pitch,mpu_roll;
/* status: 0 OK, 1 NACK, 2 bus stuck, 3 wrong ID, 4 invalid acceleration */
unsigned int mpu_init(void);
unsigned int mpu_sample(void);
void tilt_angles(float ax,float ay,float az,float *pitch,float *roll);
#endif
