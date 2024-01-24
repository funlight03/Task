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
		cout << "��ȷ��ͼ���ļ������Ƿ���ȷ" << endl;
	}
	
	
   
	// ȥ�������ֵ��
	// Mat canny;
	// Canny(img, canny, 80, 160, 3, false);
	// imshow("", canny);

	//�������㣬��ϸС��϶���
	// Mat kernel = getStructuringElement(0, Size(3, 3));
	// dilate(img, img, kernel);

    // imshow("after dilate",img);

	// �������������
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, 0, 2, Point());

	//Ѱ����������Ӿ���
	for (int n = 0; n < contours.size(); n++)
	{
		// �����Ӿ���
		Rect rect = boundingRect(contours[n]);
		if (rect.area()<10)
		{
			continue;
		}
		rectangle(img1, rect, Scalar(118, 64, 255), 2, 8, 0);

		// ��С��Ӿ���
		RotatedRect rrect = minAreaRect(contours[n]);
		Point2f points[4];
		rrect.points(points);  //��ȡ��С��Ӿ��ε��ĸ�����
		Point2f cpt = rrect.center;  //��С��Ӿ��ε�����

									 // ������ת����������λ��
		for (int i = 0; i < 4; i++)
		{
			if (i == 3)
			{
				line(img2, points[i], points[0], Scalar(118, 64, 255), 2, 8, 0);
				break;
			}
			line(img2, points[i], points[i + 1], Scalar(118, 64, 255), 2, 8, 0);
		}
		// ���ƾ��ε�����
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
	//��ȡ����֡��
	int fps = capture.get(CAP_PROP_FPS);
	
	Mat frame;
	int playspeed=1000/fps;
	while (true)
	{
		//��֡������Ƶ
		bool ret = capture.read(frame);
		if (!ret)break;
		
 
		color_follow(frame);
        // circledetect(mask,frame);
		Mat img1,img2;
		frame.copyTo(img1);  //����������������Ӿ���
		frame.copyTo(img2);  //�������������С��Ӿ���
        myRect(mask,frame,img1,img2);
		imshow("frame", frame);
	
		//���������Ӿ��εĽ��
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