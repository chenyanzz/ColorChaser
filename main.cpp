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
	for(uchar *b = channels[0].data,*g= channels[1].data,*r = channels[2].data;
		b!=channels[0].dataend; b++,g++,r++) {
		if (*r > *b / 2 || *g > *b / 2) *b = 0;
	}
	return channels[ch];
}


double distanceP(Point pointO, Point pointA) {
	return sqrt(pow((pointO.x - pointA.x), 2) + pow((pointO.y - pointA.y), 2));
}

int everage(Mat m, int x0, int y0, int w, int h) {
	if (w*h == 0)return 0;
	unsigned long long sum = 0;
	int k = 0;
	for (int x = cut(x0,0,m.cols); x<cut(x0 + w,0,m.cols); x++) {
		for(int y=cut(y0,0,m.rows);y<cut(y0+h,0,m.rows);y++) {
			sum += m.at<uchar>(y, x);
			k++;
		}
	}
	int ans = sum / k;
	return ans;
}

const int MIN = 40, MAX = 100; //阈值限制
int s1 = 30, s2 = 30, s3 = 20, s4 = 80; //四个算法阈值
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
	createTrackbar("Canny1", winname, &s3, MAX);
	createTrackbar("Canny2", winname, &s4, MAX);

	Mat original_frame;
	Point last_pos(w / 2, h / 2);
	int last_radius = 50;

	//等待用户定位圆，按任意键确认
	while(cam.read(original_frame) && (waitKey(10) == -1)) {
		circle(original_frame, last_pos, last_radius, Scalar(0, 0, 255), 2);
		imshow("frame", original_frame);
	}

	while(cam.read(original_frame)) {
		s1 = cut(s1, 1, MAX);
		s2 = cut(s2, 1, MAX);
		s3 = cut(s3, 1, MAX);
		s4 = cut(s4, 1, MAX);
		setTrackbarPos("Hough1", winname, s1);
		setTrackbarPos("Hough2", winname, s2);
		setTrackbarPos("Canny1", winname, s3);
		setTrackbarPos("Canny2", winname, s4);

		Mat frame = original_frame.clone();


		//预处理
		// cvtColor(frame, frame, COLOR_BGR2GRAY);
		frame = getChannel(frame, 0);
		equalizeHist(frame, frame);
		GaussianBlur(frame, frame, Size(9, 9), 3, 3);
		//Canny(frame, frame, s3, s4);

		//找圆，记录运行时间
		vector<Vec3f> circles;
		clock_t t0 = clock();
		Rect frameRect(0, 0, frame.cols, frame.rows);
		Rect roiRectRect(last_pos.x - last_radius - search_range, last_pos.y - last_radius - search_range, 2 * (last_radius + search_range), 2 * (last_radius + search_range));
		roiRectRect &= frameRect;
		Mat roi = frame(roiRectRect);
		Mat croi = original_frame(roiRectRect);

		rectangle(frame, roiRectRect, Scalar(0, 0, 255), 3);
		rectangle(original_frame ,roiRectRect, Scalar(0, 0, 255), 3);

		HoughCircles(roi, circles, HOUGH_GRADIENT, 1, search_range, s1, s2, h / 10, 0);
		clock_t t1 = clock();

		int cnt = circles.size();
		//依次在图中绘制出圆
		cvtColor(roi, roi, COLOR_GRAY2BGR);
		bool found = false;
		for (auto vec : circles) {
			Point center(cvRound(vec[0]), cvRound(vec[1]));
			int radius = cvRound(vec[2]);
			circle(croi, center, radius, Scalar(155, 50, 255), 3, 8, 0);
			circle(roi, center, radius, Scalar(155, 50, 255), 3, 8, 0);
		}
		// waitKey(0);
		for (auto vec : circles) {
			Point center(cvRound(vec[0]), cvRound(vec[1]));
			int radius = cvRound(vec[2]);
			if(everage(croi,center.x-radius/1.5, center.y - radius / 1.5, radius*1.4,radius*1.4)<50)continue;
			circle(croi, center, radius, Scalar(155, 250, 255), 3, 8, 0);
			circle(roi, center, radius, Scalar(155, 250, 255), 3, 8, 0);
			last_radius = radius;
			last_pos = center + roiRectRect.tl();
			found = true;
			break;
		}
		if(cnt>=5){
			s1++; s2++; s3++; s4++;
		}
		else if (!found) {
			s1--; s2--; s3--; s4--;
		}
			// //在上一次圆心周围找到目标
			// if(distanceP(center, last_pos) <= search_range && abs(last_radius - radius) <= search_range) {
			// 	circle(frame, center, radius, Scalar(155, 50, 255), 3, 8, 0);
			// 	cout << "==> Found My Circle at (" << center.x << "," << center.y << "),\tradius=" << radius <<
			// 		",\tdisplacement=" << distanceP(center, last_pos) << "px." << endl;
			// 	last_pos = center;
			// 	last_radius = radius;
			// 	found = true;
			// } else 
			// 	circle(roi, center, radius, Scalar(0, 255, 0), 3, 8, 0);
		// }
		// if(!found) {
			// s3 -= 1;
			// s4 -= 1;
		// }
  //
		 //阈值参数的增减量
		 int n = 1;
   //
		 // //防止单次时间过长导致死机
		 // //依据运行时间拟合曲线控制阈值
		 // int k = 3; //拟合曲线指数
		 // double everage_ms = 110; //基准时间
		 // if(t1 - t0 > everage_ms + 50)
		 // 	n = cut(pow((t1 - t0) / everage_ms, k), 1, 100);
   //
		 // if(cnt >= 5) {
		 // 	n = n + (cnt - 4);
		 // 	s1 += n;
		 // 	s2 += n;
		 // 	s3 += n;
		 // 	s4 += n;
		 // }
		 // if(cnt <= 2) {
		 // 	n = n + (2 - cnt);
		 // 	s1 -= n;
		 // 	s2 -= n;
		 // 	s3-=n; 
		 // 	s4-=n;
		 // }
  
		 circle(frame, last_pos, 3, Scalar(255, 255, 0), 3);
		 cout << "interval=" << t1 - t0 << "ms,\tcnt=" << cnt << ",\tfactor=" << n << endl;
		imshow("frame", frame);
		waitKey(1);
	}

	return 0;
}
