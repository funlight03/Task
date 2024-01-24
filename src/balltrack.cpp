#include<iostream>
#include<vector>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;
class QuickDemo{
    public: 
    Mat hsv, mask,erodee;
	Mat kernel=getStructuringElement(1,Size(3,3));
    void color_follow(Mat& image);
    void video_read();
    void myRect(Mat mask,Mat& frame,Mat& img1,Mat& img2);
};

void QuickDemo::myRect(Mat mask,Mat& frame,Mat& img1,Mat& img2){
    Mat img=mask;
	if (img.empty())
	{
		cout << "请确认图像文件名称是否正确" << endl;
	}
	
	
   
	// 去噪声与二值化
	// Mat canny;
	// Canny(img, canny, 80, 160, 3, false);
	// imshow("", canny);

	//膨胀运算，将细小缝隙填补上
	// Mat kernel = getStructuringElement(0, Size(3, 3));
	// dilate(img, img, kernel);

    // imshow("after dilate",img);

	// 轮廓发现与绘制
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, 0, 2, Point());

	//寻找轮廓的外接矩形
	for (int n = 0; n < contours.size(); n++)
	{
		// 最大外接矩形
		Rect rect = boundingRect(contours[n]);
		if (rect.area()<10)
		{
			continue;
		}
		rectangle(img1, rect, Scalar(118, 64, 255), 2, 8, 0);

		// 最小外接矩形
		RotatedRect rrect = minAreaRect(contours[n]);
		Point2f points[4];
		rrect.points(points);  //读取最小外接矩形的四个顶点
		Point2f cpt = rrect.center;  //最小外接矩形的中心

									 // 绘制旋转矩形与中心位置
		for (int i = 0; i < 4; i++)
		{
			if (i == 3)
			{
				line(img2, points[i], points[0], Scalar(118, 64, 255), 2, 8, 0);
				break;
			}
			line(img2, points[i], points[i + 1], Scalar(118, 64, 255), 2, 8, 0);
		}
		// 绘制矩形的中心
        circle(img2, cpt, 2, Scalar(118, 64, 255), 2, 8, 0);
	}
}
void QuickDemo::color_follow(Mat& image)
{
	
	cvtColor(image, hsv, COLOR_BGR2HSV);
	namedWindow("hsv", WINDOW_FREERATIO);
	imshow("hsv", hsv);
	inRange(hsv, Scalar(0,91, 98), Scalar(39, 255, 223), erodee);
	// namedWindow("mask", WINDOW_FREERATIO);
	// imshow("mask", mask);
	erode(erodee,mask,kernel);
	morphologyEx(mask, mask, MORPH_OPEN, kernel);
	// namedWindow("mask2", WINDOW_FREERATIO);
	// imshow("mask2", mask);
}
void QuickDemo::video_read()
{
	VideoCapture capture("kunkunplayball.mp4");
	if (!capture.isOpened()) {
		cout << "Can't open file!" << endl;
	}
	//获取合适帧率
	int fps = capture.get(CAP_PROP_FPS);
	
	Mat frame;
	int playspeed=1000/fps;
	while (true)
	{
		//逐帧传入视频
		bool ret = capture.read(frame);
		if (!ret)break;
		
 
		color_follow(frame);
        // circledetect(mask,frame);
		Mat img1,img2;
		frame.copyTo(img1);  //深拷贝用来绘制最大外接矩形
		frame.copyTo(img2);  //深拷贝用来绘制最小外接矩形
        myRect(mask,frame,img1,img2);
		imshow("frame", frame);
	
		//输出绘制外接矩形的结果
	    imshow("max", img1);
	    imshow("min", img2);
		// char c = waitKey(10);
		// if (c == 30)
		// 	break;
		waitKey(playspeed);
 
 
	}
	capture.release();
	
}
int main(){
    QuickDemo a;
    a.video_read();
}