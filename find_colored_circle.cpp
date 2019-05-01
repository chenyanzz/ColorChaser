#include "opencv2/opencv.hpp"
#include <ctime>
#include <cmath>
#include <iostream>

using namespace std;
using namespace cv;

const string winname = "frame";


//范围限制
double cut(double val, double min, double max) {
    if(val > max)val = max;
    if(val < min)val = min;
    return val;
}


Mat getChannel(Mat& m, int ch) {
    vector<Mat> channels(m.channels());
    split(m, channels);
// 	for(uchar *b = channels[0].data,*g= channels[1].data,*r = channels[2].data;
// 		b!=channels[0].dataend; b++,g++,r++) {
// 		if (*r > *b / 2 || *g > *b / 2) *b = 0;
// 	}
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
    rectangle(m, Point(x0, y0), Point(x0 + w, y0 + h), Scalar(255, 0, 0));

    //for (int x = cut(x0,0,m.cols); x<cut(x0 + w,0,m.cols); x++) {
    //	for(int y=cut(y0,0,m.rows);y<cut(y0+h,0,m.rows);y++) {

    for(auto it = roi.begin<uchar>(); it != roi.end<uchar>(); ++it) {
        auto n = (*it);
        sum += n;
        //cout << (n > 200);
        k++;
    }
    //}
    int ans = sum / k;
    //cout << "\navg:" << ans << endl;
    return ans;
}


const int MIN = 40, MAX = 100; //阈值限制
int s1 = 60, s2 = 60; //四个算法阈值
int iLowH = 107, iHighH = 118;

int h = 1440, w = 1920;

int search_range = 50;


int main() {
    VideoCapture cam(0);
    if(!cam.isOpened()) {
        cerr << "ERROR: CANNOT open camera0!" << endl;
        return -1;
    }
    // cam.set(CAP_PROP_FRAME_WIDTH, w);
    // cam.set(CAP_PROP_FRAME_HEIGHT, h);
    h = cam.get(CAP_PROP_FRAME_HEIGHT);
    w = cam.get(CAP_PROP_FRAME_WIDTH);
    namedWindow(winname);
    createTrackbar("Hough1", winname, &s1, MAX);
    createTrackbar("Hough2", winname, &s2, MAX);

    Mat original_frame;
    Point last_pos(w / 2, h / 2);
    int last_radius = 50;


    createTrackbar("LowH", winname, &iLowH, 179);
    createTrackbar("HighH", winname, &iHighH, 179);

    //创建进度条
    //等待用户定位圆，按任意键确认
// 	while(cam.read(original_frame) && (waitKey(10) == -1)) {
// 		circle(original_frame, last_pos, last_radius, Scalar(0, 0, 255), 2);
// 		imshow("frame", original_frame);
// 	}

    while(cam.read(original_frame)) {
        s1 = cut(s1, 1, MAX);
        s2 = cut(s2, 1, MAX);
        setTrackbarPos("Hough1", winname, s1);
        setTrackbarPos("Hough2", winname, s2);


        Mat frame = original_frame.clone();

        //预处理
        GaussianBlur(frame, frame, Size(9, 9), 10, 10);
        cvtColor(frame, frame, COLOR_BGR2HSV);

        //inRange(frame, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS,iHighV),frame);
        frame = getChannel(frame, 0);

        for(int i = 0; i < frame.cols * frame.rows; i++) {
            uchar& n = frame.at<uchar>(i);
            if(n < iHighH && n > iLowH) n = 255;
            else n = 0;
        }

        GaussianBlur(frame, frame, Size(9, 9), 10, 10);
        Mat element = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
        /// 腐蚀操作
        dilate(frame, frame, element);
        //dilate( frame, frame, element );
        //dilate( frame, frame, element );
        //dilate( frame, frame, element );

//		equalizeHist(frame, frame);
        //连接连通域
        //morphologyEx(frame, frame, MORPH_CLOSE, element);

        //找圆，记录运行时间
        vector<Vec3f> circles;
// 		clock_t t0 = clock();
// 		Rect frameRect(0, 0, frame.cols, frame.rows);
// 		Rect roiRectRect(last_pos.x - last_radius - search_range, last_pos.y - last_radius - search_range, 2 * (last_radius + search_range), 2 * (last_radius + search_range));
// 		roiRectRect &= frameRect;
// 		Mat roi = frame(roiRectRect);
// 		Mat croi = original_frame(roiRectRect);

// 		rectangle(frame, roiRectRect, Scalar(0, 0, 255), 3);
// 		rectangle(original_frame ,roiRectRect, Scalar(0, 0, 255), 3);

        HoughCircles(frame, circles, HOUGH_GRADIENT, 1, h / 5, s1, s2, h / 10, 0);
// 		clock_t t1 = clock();

 		int cnt = circles.size();

// 		//依次在图中绘制出圆
        Mat colored;
        cvtColor(frame, colored, COLOR_GRAY2BGR);
 		bool found = false;
        for(auto vec : circles) {
            Point center(cvRound(vec[0]), cvRound(vec[1]));
            int radius = cvRound(vec[2]);
            if(everage(frame, center.x - radius / 3, center.y - radius / 3, radius / 2, radius / 2) > 200) {
                circle(colored, center, radius, Scalar(155, 50, 255), 3, 8, 0);
                circle(colored, center, 1, Scalar(155, 50, 255), 3, 8, 0);
                found=true;
                break;
            }
            else {
                //circle(colored, center, radius, Scalar(0, 0, 255), 3, 8, 0);
            }
        }
// 		for (auto vec : circles) {
// 			Point center(cvRound(vec[0]), cvRound(vec[1]));
// 			int radius = cvRound(vec[2]);
// 			circle(frame, center, radius, Scalar(155, 250, 255), 3, 8, 0);
// 			last_radius = radius;
// 			last_pos = center;
// 			break;
// 		}
// 		 //阈值参数的增减量
 		 int n = 1;
        //
        // //防止单次时间过长导致死机
        // //依据运行时间拟合曲线控制阈值
        // int k = 3; //拟合曲线指数
        // double everage_ms = 110; //基准时间
        // if(t1 - t0 > everage_ms + 50)
        // 	n = cut(pow((t1 - t0) / everage_ms, k), 1, 100);
        //
         if(cnt >= 5) {
         	n = n + (cnt - 4);
         	s1 += n;
         	s2 += n;
         }
         if(cnt <= 1 || !found) {
         	n = n + (2 - cnt);
         	s1 -= n;
         	s2 -= n;
         }

// 		 cout << "interval=" << t1 - t0 << "ms,\tcnt=" << cnt << ",\tfactor=" << n << endl;
        imshow("frame", colored);
        waitKey(1);
    }

    return 0;
}
