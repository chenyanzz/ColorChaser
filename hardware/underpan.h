#pragma once

#include "motor.h"

class Underpan {
protected:
  const pin_t motor_left_pinA = 21, motor_left_pinB = 26, motor_left_pinEn = 20,
              motor_right_pinA = 19, motor_right_pinB = 16,
              motor_right_pinEn = 13;

  Motor motor_left, motor_right;

public:
  Underpan()=default;

  void init();
  void setPower(int l,int r);
  void stop();

};

extern Underpan underpan;