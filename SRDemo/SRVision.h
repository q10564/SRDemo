#pragma once
#include <opencv.hpp>
#include <vector>
using namespace cv;
using namespace std;
#define PI 3.1415926
/*
	标定结果矩阵
*/
struct calibResult
{
	double A;
	double B;
	double C;
	double D;
	double E;
	double F;
};
/*
9点标定函数：calibration(vector<cv::Point2f> world, vector<cv::Point2f> pix, calibResult calib)
参数：
world：世界坐标9点
pix：像素坐标9点
calib: 标定结果
坐标点顺序：
	0	1	2
	3	4	5
	6	7	8
*/
bool calibration(vector<Point2f> world, vector<Point2f> pix, calibResult &calib);

Mat cvtColor_RGB2HSI(Mat &input);

Mat ImageRotation(cv::Mat &image, double angle);

Mat ImageMirror(cv::Mat &image, int type);

void GetCalcHistImage(Mat input, Mat &output);