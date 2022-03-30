#pragma once
#include <opencv.hpp>
#include <vector>
using namespace cv;
using namespace std;
namespace SRVision
{
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
	enum SRThreshold
	{
		Threshold_0, //手动二值化 黑色对象
		Threshold_1, //手动二值化 白色对象
		Threshold_2, //大津法二值化 黑色对象
		Threshold_3, //大津法二值化 白色对象
	};
	//直方图类
	class SRCalcHist
	{
	public:
		SRCalcHist(Mat image);
		SRCalcHist();
		/*
			函数作用：
			获取直方图
			变量：
			input:输入图像
			output：输出直方图图像
		*/
		Mat calcHistImage;//灰度图
		int gray[256] = { 0 };//灰度数组
		double max;//最大灰度值
		double min;//最小灰度值
		double average;//平均灰度值
		double stddev;//灰度值方差
	private:
		void getCalcHistImage(Mat input, Mat &output);
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

	Mat getRotationImage(cv::Mat &image, double angle);

	Mat getMirrorImage(cv::Mat &image, int type);

	/*
	函数作用：
			获取二值化图像
		变量：
			input:输入图像
			output：输出图像
			type：二值化方式
			low：低阈值
			hight:高阈值
	*/
	int getThresholdImage(Mat input, Mat &output, SRThreshold type, int low = 0, int hight = 255);

	int getThresholdOtsu(Mat input, Mat &output,SRThreshold type);
}
