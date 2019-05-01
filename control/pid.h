#pragma once

enum PID_MODE
{
    PID_POSITION = 0,
    PID_DELTA
};

typedef struct
{
    PID_MODE mode;
    //PID 三参数
    double Kp;
    double Ki;
    double Kd;

    double max_out;  //最大输出
    double max_iout; //最大积分输出

    double set;
    double fdb;

    double out;
    double Pout;
    double Iout;
    double Dout;
    double Dbuf[3];  //微分项 0最新 1上一次 2上上次
    double error[3]; //误差项 0最新 1上一次 2上上次

} PidTypeDef;

void PID_Init(PidTypeDef *pid, PID_MODE mode, double kp, double ki , double kd, double max_out, double max_iout);
double PID_Calc(PidTypeDef *pid, double ref, double set);
void PID_clear(PidTypeDef *pid);
