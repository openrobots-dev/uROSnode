#ifndef TOPICS_H_
#define TOPICS_H_

#include "Middleware.hpp"

#define LED23_ID		1001
#define LED2_ID			1012
#define LED3_ID			1013
#define LED4_ID			1014

#define PWM123_ID		2001
#define PWM1_ID			2011
#define PWM2_ID			2012
#define PWM3_ID			2013
#define QEI1_ID			2021
#define QEI2_ID			2022
#define QEI3_ID			2023
#define SPEED123_ID		2030
#define PIDSETUP_ID		2040

class Led: public BaseMessage {
public:
	uint8_t pin;
	bool_t set;
}__attribute__((packed));

class PWM: public BaseMessage {
public:
	int16_t pwm;
}__attribute__((packed));

class PWM3: public BaseMessage {
public:
	int16_t pwm1;
	int16_t pwm2;
	int16_t pwm3;
}__attribute__((packed));

class QEI: public BaseMessage {
public:
	uint16_t value;
	uint32_t timestamp;
}__attribute__((packed));

class SpeedSetpoint: public BaseMessage {
public:
	int16_t speed;
}__attribute__((packed));

class SpeedSetpoint3: public BaseMessage {
public:
	int16_t speed1;
	int16_t speed2;
	int16_t speed3;
}__attribute__((packed));

class PIDSetup: public BaseMessage {
public:
	int16_t kp;
	int16_t ki;
	int16_t kd;
}__attribute__((packed));

#endif /* TOPICS_H_ */
