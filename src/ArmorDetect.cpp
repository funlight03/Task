#include "stdio.h"
#include<iostream> 
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;
class LightDescriptor
{	    
//在识别以及匹配到灯条的功能中需要用到旋转矩形的长宽偏转角面积中心点坐标等
public:
    float width, length, angle, area;
    cv::Point2f center;
public:
    LightDescriptor() {};
    //让得到的灯条套上一个旋转矩形，以方便之后对角度这个特殊因素作为匹配标准
    LightDescriptor(const cv::RotatedRect& light)
    {
        width = light.size.width;
        length = light.size.height;
        center = light.center;
        angle = light.angle;
        area = light.size.area();
    }
};
//重新排序矩形坐标点
void SortRotatedRectPoints(Point2f vetPoints[], RotatedRect rect, int flag)
{
  rect.points(vetPoints);
 
 
//   cout << vetPoints[0] << vetPoints[1] << vetPoints[2] << vetPoints[3] << endl;
//   cout << rect.angle << endl;
 
 
  Point2f curpoint;
  if (flag == 0) {
    //按X轴排序
    for (int i = 0; i < 4; ++i) {
      for (int k = i + 1; k < 4; ++k) {
        if (vetPoints[i].x > vetPoints[k].x) {
          curpoint = vetPoints[i];
          vetPoints[i] = vetPoints[k];
          vetPoints[k] = curpoint;
        }
      }
    }
 
 
    //判断X坐标前两个定义左上左下角
    if (vetPoints[0].y > vetPoints[1].y) {
      curpoint = vetPoints[0];
      vetPoints[0] = vetPoints[1];
      vetPoints[1] = vetPoints[3];
      vetPoints[3] = curpoint;
    }
    else {
      curpoint = vetPoints[3];
      vetPoints[3] = vetPoints[1];
      vetPoints[1] = curpoint;
    }
 
 
    //判断X坐标后两个定义右上右下角
    if (vetPoints[1].y > vetPoints[2].y) {
      curpoint = vetPoints[1];
      vetPoints[1] = vetPoints[2];
      vetPoints[2] = curpoint;
    }
  }
  else {
    //根据Rect的坐标点，Y轴最大的为P[0]，p[0]围着center顺时针旋转, 
    //旋转角度为负的话即是P[0]在左下角，为正P[0]是右下角
    //重新排序坐标点
    if (rect.angle < 0) {
      curpoint = vetPoints[0];
      vetPoints[0] = vetPoints[2];
      vetPoints[2] = curpoint;
      curpoint = vetPoints[1];
      vetPoints[1] = vetPoints[3];
      vetPoints[3] = curpoint;
    }
    else if (rect.angle > 0) {
      curpoint = vetPoints[0];
      vetPoints[0] = vetPoints[1];
      vetPoints[1] = vetPoints[2];
      vetPoints[2] = vetPoints[3];
      vetPoints[3] = curpoint;
    }
  }
 
 
}
 
 
int main()
{
    VideoCapture video; 
    video.open("ood_red.mp4");
    //变量集中定义
    Mat frame, channels[3], binary,erodee, Gaussian, dilatee,dst,draw;
    Mat element = getStructuringElement(MORPH_RECT, Size(5,5));
    Mat kernel =getStructuringElement(1,Size(3,3));
    Rect boundRect;
    RotatedRect box;
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    
    double fps;  
    char string[10];  // 用于存放帧率的字符串  
    double t = 0; 

    Point2f dst_point[4];
    dst_point[0]=Point2f(0,0);
    dst_point[1]=Point2f(500,0);
    dst_point[3]=Point2f(0,500);
    dst_point[2]=Point2f(500,500);

    Mat rotation;

    //图像预处理
    for (;;) {
        t=(double)getTickCount();
        Rect point_array[20];
        video >> frame;  //读取每帧
        draw=frame.clone();
        if (frame.empty()) {
            break;
        }
        split(frame, channels); //通道分离
        threshold(channels[0], binary, 220, 255, 0);//二值化
        erode(binary,erodee,kernel);
        GaussianBlur(erodee, Gaussian, Size(5, 5), 0);//滤波
        dilate(Gaussian, dilatee, element);
        // dilate(Gaussian, dilate, element, Point(-1, -1));//膨胀，把滤波得到的细灯条变宽
        findContours(dilatee, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);//轮廓检测
        vector<LightDescriptor> lightInfos;//创建一个灯条类的动态数组
        
        //筛选灯条
        for (int i = 0; i < contours.size(); i++) {
            // 求轮廓面积
            double area = contourArea(contours[i]);
            // 去除较小轮廓&fitEllipse的限制条件
            if (area < 5 || contours[i].size() <= 1)
                continue;//相当于就是把这段轮廓去除掉
            // 用椭圆拟合区域得到外接矩形（特殊的处理方式：因为灯条是椭圆型的，所以用椭圆去拟合轮廓，再直接获取旋转外接矩形即可）
            RotatedRect Light_Rec = fitEllipse(contours[i]);
 
            // 长宽比和轮廓面积比限制（由于要考虑灯条的远近都被识别到，所以只需要看比例即可）
            if (Light_Rec.size.width / Light_Rec.size.height > 4)
                continue;
            lightInfos.push_back(LightDescriptor(Light_Rec));
        }
        //二重循环多条件匹配灯条
        for (size_t i = 0; i < lightInfos.size(); i++) {
            for (size_t j = i + 1; (j < lightInfos.size()); j++) {
                LightDescriptor& leftLight = lightInfos[i];
                LightDescriptor& rightLight = lightInfos[j];
                float angleGap_ = abs(leftLight.angle - rightLight.angle);
                //由于灯条长度会因为远近而受到影响，所以按照比值去匹配灯条
                float LenGap_ratio = abs(leftLight.length - rightLight.length) / max(leftLight.length, rightLight.length);
                float dis = pow(pow((leftLight.center.x - rightLight.center.x), 2) + pow((leftLight.center.y - rightLight.center.y), 2), 0.5);
                //均长
                float meanLen = (leftLight.length + rightLight.length) / 2;
                float lengap_ratio = abs(leftLight.length - rightLight.length) / meanLen;
                float yGap = abs(leftLight.center.y - rightLight.center.y);
                float yGap_ratio = yGap / meanLen;
                float xGap = abs(leftLight.center.x - rightLight.center.x);
                float xGap_ratio = xGap / meanLen;
                float ratio = dis / meanLen;
                //匹配不通过的条件
                if (angleGap_ > 15 ||
                    LenGap_ratio > 1.0 ||
                    lengap_ratio > 0.8 ||
                    yGap_ratio > 1.5 ||
                    xGap_ratio > 2.2 ||
                    xGap_ratio < 0.8 ||
                    ratio > 3 ||
                    ratio < 0.8) 
                {
                    continue;
                }
                
                
                //绘制矩形
                Point center = Point((leftLight.center.x + rightLight.center.x) / 2, (leftLight.center.y + rightLight.center.y) / 2);
                RotatedRect rect = RotatedRect(center, Size(dis, meanLen), (leftLight.angle + rightLight.angle) / 2);                
                Point2f vertices[4];
                rect.points(vertices);
                
                RotatedRect rect_num = RotatedRect(center, Size(dis*0.6, meanLen*2), (leftLight.angle + rightLight.angle) / 2);
                Point2f vertices_num[4];
                rect_num.points(vertices_num);
                
                for (int i = 0; i < 4; i++) {
                    line(draw, vertices[i], vertices[(i + 1) % 4], Scalar(0, 0, 255), 2.2);
                    line(draw, vertices_num[i], vertices_num[(i + 1) % 4], Scalar(0,255,0), 2.2);
              
                }
                //重新排列坐标点
                SortRotatedRectPoints(vertices_num,rect_num,0);                  
                //截取数字
                putText(draw,"0",vertices_num[0],0,1,Scalar(0,0,255),1,1,false);
                putText(draw,"1",vertices_num[1],0,1,Scalar(0,0,255),1,1,false);
                putText(draw,"2",vertices_num[2],0,1,Scalar(0,0,255),1,1,false);
                putText(draw,"3",vertices_num[3],0,1,Scalar(0,0,255),1,1,false);                                    
                
                
                rotation=getPerspectiveTransform(vertices_num,dst_point);
                warpPerspective(channels[0],dst,rotation,Size(500,500));
                Mat binary_num;
                threshold(dst,binary_num,30,255,0);
                imshow("number",binary_num);
                
            }
        }
        // cout<<"灯条数:"<<lightInfos.size()<<endl;
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();  
        fps=1.0/t;
        sprintf(string, "%.2f", fps);      // 帧率保留两位小数  
        std::string fpsString("FPS:");  
        fpsString += string;                     
        // 将帧率信息写在输出帧上  
        putText(draw,fpsString,cv::Point(5, 20),FONT_HERSHEY_SIMPLEX,0.5,Scalar(0, 0, 255)); 
        
        
        namedWindow("video", WINDOW_FULLSCREEN);
        imshow("video", draw);
        // imshow("dilatee",dilatee);
        // imshow("erodee",erodee);
        
        
        waitKey(1);
    }
    video.release();
    cv::destroyAllWindows();
    return 0;
}