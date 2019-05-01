#include "underpan.h"

Underpan underpan;

void Underpan::init() {
    motor_left.init("motor_left", motor_left_pinA, motor_left_pinB,
                    motor_left_pinEn);
    motor_right.init("motor_right", motor_right_pinA, motor_right_pinB,
                     motor_right_pinEn);
    LOG(INFO) << "Underpan created.";
}

void Underpan::setPower(int l, int r){
	motor_left.setPwm(l);
	motor_right.setPwm(r);
    VLOG(1) << "motor power: " << l<<", "<<r;
}

void Underpan::stop() {
	motor_left.stop();
	motor_right.stop();
}