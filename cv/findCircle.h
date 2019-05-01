#pragma once

#include "opencv2/opencv.hpp"

//找到的圆的数据
extern cv::Point circle_center;
extern int circle_radius;
extern bool circle_hasFound;

// extern int s1, s2; //四个算法阈值
// extern int iLowH, iHighH;
// extern int dilate_times;
extern int h, w;
// extern const cv::Scalar avg_rect_color, found_circle_color, other_circle_color;

void initCv();
void vCtini();
void findCircle();