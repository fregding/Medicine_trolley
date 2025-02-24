#ifndef __MOTIONCONTROL_H
#define __MOTIONCONTROL_H


#define KP_mpu (float)1.5
#define KI_mpu (float)0.01
#define KD_mpu (float)0.5

extern float yaw, pitch, roll;

float getYawAngle(void);

void updateMotionControl(float targetYaw,float *current_leftPWM, float *current_rightPWM);

#endif
