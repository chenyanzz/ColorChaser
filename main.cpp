#include <ctime>
#include <string>
#include <wiringPi.h>
#include "cv/findCircle.h"
#include "hardware/underpan.h"
#include "control/pid.h"
#include <signal.h>
using namespace std;
using namespace cv;

const int min_speed_to_run = 20;

int enlarge(int val){
    if(val>0)
        return val + min_speed_to_run;
    else
        return val - min_speed_to_run;
}

void sigint_handler(int){
    underpan.stop();
    vCtini();
    exit(-1);
}

int main() {
    sigset( SIGINT, sigint_handler );

    google::InitGoogleLogging("ColorChaser");
    FLAGS_colorlogtostderr = true;
    FLAGS_logtostderr = true;  //输出到控制台
    if (-1 == wiringPiSetupGpio())
        LOG(FATAL) << "WiringPi setup failed";
    underpan.init();
    initCv();

    PidTypeDef pid_turn;
    int pid_turn_max = 50;
    PID_Init(&pid_turn,PID_POSITION,1.5,0.1,0.3,pid_turn_max,pid_turn_max*0.2);
    PidTypeDef pid_speed;
    int pid_speed_max = 50;
    PID_Init(&pid_speed,PID_POSITION,1.5,0.1,0.5,pid_speed_max,pid_speed_max*0.3);
    while(true) {
        clock_t t0 = clock();

        findCircle();

	    LOG(INFO) << "time: " << (clock()-t0)*1000/CLOCKS_PER_SEC << "ms";
        if(!circle_hasFound){
            // underpan.stop();
            continue;
        }
        
        // int speed = 0;
        int speed = PID_Calc(&pid_speed, circle_radius, h/2/3);
        int turn = PID_Calc(&pid_turn, w/2, circle_center.x);
        // int turn = 0;
        LOG(INFO) << "circle.x=" << circle_center.x << ", pid_turn = " << turn << ", pid_speed = " << speed;
        underpan.setPower(enlarge(speed+turn), enlarge(speed-turn));
    }
    vCtini();
    return 0;
}