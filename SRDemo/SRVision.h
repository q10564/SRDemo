#pragma once
#include <opencv.hpp>
#include <vector>
using namespace cv;
using namespace std;
#define PI 3.1415926
/*
	�궨�������
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
9��궨������calibration(vector<cv::Point2f> world, vector<cv::Point2f> pix, calibResult calib)
������
world����������9��
pix����������9��
calib: �궨���
�����˳��
	0	1	2
	3	4	5
	6	7	8
*/
bool calibration(vector<Point2f> world, vector<Point2f> pix, calibResult &calib);

Mat cvtColor_RGB2HSI(Mat &input);

Mat ImageRotation(cv::Mat &image, double angle);

Mat ImageMirror(cv::Mat &image, int type);

void GetCalcHistImage(Mat input, Mat &output);