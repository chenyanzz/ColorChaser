#include "cv/findCircle.h"
#include "opencv2/opencv.hpp"
using namespace std;
using namespace cv;

//初始变量配置
const string win_cfg = "config";
const int cam_index = 0;
const int MIN = 40, MAX = 100; //阈值限制
int s1 = 30, s2 = 30; //四个算法阈值
int iLowH = 100, iHighH = 128;
int dilate_times = 0;
int h = 120, w = 160;
int color_range_mode=0;
int everage_hue_min = 220;
int s_min = 30, v_min=30;
const Scalar avg_rect_color(150,150,100,50),
  found_circle_color(221,186,153,200),
  other_circle_color(150,150,100,50);

//范围限制
double cut(double val, double min, double max) {
    if(val > max)val = max;
    if(val < min)val = min;
    return val;
}

Mat getChannel(Mat& m, int ch) {
    vector<Mat> channels(m.channels());
    split(m, channels);
    return channels[ch];
}

double distanceP(Point pointO, Point pointA) {
    return sqrt(pow((pointO.x - pointA.x), 2) + pow((pointO.y - pointA.y), 2));
}

int everage(Mat& m, int x0, int y0, int w, int h) {
    if(w * h == 0)return 0;
    unsigned long long sum = 0;
    int k = 0;

    Mat roi = m(Rect(x0, y0, w, h) & Rect(0, 0, m.cols, m.rows));

    for(auto it = roi.begin<uchar>(); it != roi.end<uchar>(); ++it) {
        auto n = (*it);
        sum += n;
        k++;
    }
    int ans = sum / k;
    //cout << "\navg:" << ans << endl;
    return ans;
}

//找到的圆的数据
Point circle_center;
int circle_radius;
bool circle_hasFound;
VideoCapture cam;

void initCv(){
	if(!cam.open(cam_index))
        LOG(FATAL) << "CANNOT open camera" << cam_index;
    cam.set(CAP_PROP_FOURCC,CAP_V4L2);
    cam.set(CAP_PROP_FPS,30);
    cam.set(CAP_PROP_BUFFERSIZE, 1);
    cam.set(CAP_PROP_FRAME_WIDTH, w);
    cam.set(CAP_PROP_FRAME_HEIGHT, h);
    h = cam.get(CAP_PROP_FRAME_HEIGHT);
    w = cam.get(CAP_PROP_FRAME_WIDTH);

    LOG(INFO) << "Camera frame size = (" << w << ", " << h << ")";

    //创建进度条
    namedWindow(win_cfg);
    Mat wxy = imread("wxy.jpg");
    imshow(win_cfg,wxy);
    createTrackbar("Hough1", win_cfg, &s1, MAX);
    createTrackbar("Hough2", win_cfg, &s2, MAX);
    createTrackbar("LowH", win_cfg, &iLowH, 179);
    createTrackbar("HighH", win_cfg, &iHighH, 179);
    createTrackbar("dilate_times", win_cfg, &dilate_times, 10);
	createTrackbar("color_range_mode", win_cfg, &color_range_mode, 1);
    createTrackbar("everage_hue_min",win_cfg, &everage_hue_min, 256);
    createTrackbar("s_min", win_cfg, &s_min, 256);
    createTrackbar("v_min", win_cfg, &v_min, 256);
}

void vCtini(){
    cam.release();
}

Mat changeWhiteBalance(const Mat& imageSource){
    vector<Mat> imageRGB;
 
	//RGB三通道分离
	split(imageSource, imageRGB);
 
	//求原始图像的RGB分量的均值
	double R, G, B;
	B = mean(imageRGB[0])[0];
	G = mean(imageRGB[1])[0];
	R = mean(imageRGB[2])[0];
 
	//需要调整的RGB分量的增益
	double KR, KG, KB;
	KB = (R + G + B) / (3 * B);
	KG = (R + G + B) / (3 * G);
	KR = (R + G + B) / (3 * R);
 
	//调整RGB三个通道各自的值
	imageRGB[0] = imageRGB[0] * KB;
	imageRGB[1] = imageRGB[1] * KG;
	imageRGB[2] = imageRGB[2] * KR;
    Mat ans;
	//RGB三通道图像合并
	merge(imageRGB, ans);

    return ans;
}

bool in(double x, double min, double max){
    return x<=max && x>=min;
}

void findCircle(){
	Mat original_frame;
	cam >> original_frame;
    cam >> original_frame;
    cam >> original_frame;
    // original_frame = changeWhiteBalance(original_frame);
	s1 = cut(s1, 20, MAX);
	s2 = cut(s2, 20, MAX);
	setTrackbarPos("Hough1", win_cfg, s1);
	setTrackbarPos("Hough2", win_cfg, s2);

	Mat frame = original_frame.clone();

    //预处理
    GaussianBlur(frame, frame, Size(9, 9), 10, 10);
    cvtColor(frame, frame, COLOR_BGR2HSV); 

    //HUE Channel
    Mat H,S,V;
    S = getChannel(frame, 1);
    V = getChannel(frame, 2);
    frame = getChannel(frame, 0);
	for(int i = 0; i < frame.cols * frame.rows; i++) {
		uchar& h = frame.at<uchar>(i);
        //range in hue
		if(color_range_mode==1)
			h = 255*(h > iHighH || h < iLowH);
		else
			h = 255*(h < iHighH && h > iLowH);

        //range in s,v for pure black or white
        if(S.at<uchar>(i)<s_min) h = 0;
        if(V.at<uchar>(i)<v_min) h = 0;
        
	}

    GaussianBlur(frame, frame, Size(9, 9), 10, 10);
    /// 腐蚀操作
    Mat element = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
    for(int i = 0; i < dilate_times; ++i) dilate(frame, frame, element);
    //连接连通域
    //morphologyEx(frame, frame, MORPH_CLOSE, element);

    //找圆，记录运行时间
    vector<Vec3f> circles;
    HoughCircles(frame, circles, HOUGH_GRADIENT, 1, h / 5, s1, s2, 10, 0);
    int cnt = circles.size();

    //依次在图中绘制出圆
    Mat colored;
    cvtColor(frame, colored, COLOR_GRAY2BGR);
    circle_hasFound = false;
    for(auto vec : circles) {
        circle_center = Point(cvRound(vec[0]), cvRound(vec[1]));
        circle_radius = cvRound(vec[2]);
        const double everage_divisor = 0.5;

        int x0 = circle_center.x - circle_radius * everage_divisor;
        int y0 = circle_center.y - circle_radius * everage_divisor;
        int dim = circle_radius * 2 * everage_divisor;
        rectangle(colored, Point(x0, y0), Point(x0 + dim, y0 + dim), avg_rect_color);
        if(everage(frame, x0, y0, dim, dim) >= everage_hue_min) {
            circle(colored, circle_center, circle_radius, found_circle_color, 3, 8, 0);
            circle(colored, circle_center, 1, found_circle_color, 3, 8, 0);
            circle(original_frame, circle_center, circle_radius, found_circle_color, 3, 8, 0);
            circle(original_frame, circle_center, 1, found_circle_color, 3, 8, 0);
            circle_hasFound = true;
            break;
        } else {
            circle(colored, circle_center, circle_radius, other_circle_color, 1, 8, 0);
            circle(colored, circle_center, 1, other_circle_color, 3, 8, 0);
        }
    }

    //阈值参数的增减量
    int n = 1;

    if(cnt >= 5) {
        n = n + (cnt - 4);
        s1 += n;
        s2 += n;
    }
    if(cnt <= 1 || !circle_hasFound) {
        s1 -= n;
        s2 -= n;
    }
    // pyrUp(colored, colored, Size(w*2, h*2));
    // pyrUp(original_frame, original_frame, Size(w*2, h*2));
	imshow("processed", colored);
    imshow("original", original_frame);
	waitKey(30);
}