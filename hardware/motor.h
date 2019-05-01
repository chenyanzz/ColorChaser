#pragma once
#include <string>

class Motor {
protected:
  std::string name;
  pin_t pinA, pinB, pinEn;
  const int pwmRange = 256;

public:
  Motor() = default;
  void init(const std::string &name, pin_t pinA, pin_t pinB, pin_t pinEn);
  void setPwm(int val);// $val in [-pwmRange, pwmRange]
  void stop();
};