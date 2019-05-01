#include "motor.h"

#include <wiringPi.h>
#include <softPwm.h>
using namespace std;

void Motor::init(const string &name, pin_t pinA, pin_t pinB, pin_t pinEn) {
    this->name = name;
    this->pinA = pinA;
    this->pinB = pinB;
    this->pinEn = pinEn;

    pinMode(pinA, OUTPUT);
    pinMode(pinB, OUTPUT);
    pinMode(pinEn, OUTPUT);

    digitalWrite(pinA, LOW);
    digitalWrite(pinB, LOW);
    digitalWrite(pinEn, HIGH);

    if (softPwmCreate(pinA, 0, pwmRange))
      LOG(FATAL) << "Cannot create soft pwm on pin" << pinA;
    
    if (softPwmCreate(pinB, 0, pwmRange))
      LOG(FATAL) << "Cannot create soft pwm on pin" << pinA;

    LOG(INFO) << "Motor '" << name << "' initialized cucessfully with pinA=" << pinA << ", pinB=" << pinB << ", pinEn=" << pinEn;
  }

  void Motor::setPwm(int val) {
    if (val > pwmRange) {
      val = pwmRange;
      LOG(WARNING) << "Motor " << name << " pwm range exceeded (param " << val << " > " << pwmRange <<")";
    }
    if (val < -pwmRange) {
      val = -pwmRange;
      LOG(WARNING) << "Motor " << name << " pwm range exceeded (param " << val << " < " << -pwmRange <<")";
    }
    softPwmWrite(pinA, val>0 ? val : 0);
    softPwmWrite(pinB, val<0 ? -val : 0);
  }

  void Motor::stop() {
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, LOW);
    digitalWrite(pinEn, LOW);
  }