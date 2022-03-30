#pragma once
#include <opencv.hpp>
#include <vector>
using namespace cv;
using namespace std;
namespace SRVision
{
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
	enum SRThreshold
	{
		Threshold_0, //�ֶ���ֵ�� ��ɫ����
		Threshold_1, //�ֶ���ֵ�� ��ɫ����
		Threshold_2, //��򷨶�ֵ�� ��ɫ����
		Threshold_3, //��򷨶�ֵ�� ��ɫ����
	};
	//ֱ��ͼ��
	class SRCalcHist
	{
	public:
		SRCalcHist(Mat image);
		SRCalcHist();
		/*
			�������ã�
			��ȡֱ��ͼ
			������
			input:����ͼ��
			output�����ֱ��ͼͼ��
		*/
		Mat calcHistImage;//�Ҷ�ͼ
		int gray[256] = { 0 };//�Ҷ�����
		double max;//���Ҷ�ֵ
		double min;//��С�Ҷ�ֵ
		double average;//ƽ���Ҷ�ֵ
		double stddev;//�Ҷ�ֵ����
	private:
		void getCalcHistImage(Mat input, Mat &output);
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

	Mat getRotationImage(cv::Mat &image, double angle);

	Mat getMirrorImage(cv::Mat &image, int type);

	/*
	�������ã�
			��ȡ��ֵ��ͼ��
		������
			input:����ͼ��
			output�����ͼ��
			type����ֵ����ʽ
			low������ֵ
			hight:����ֵ
	*/
	int getThresholdImage(Mat input, Mat &output, SRThreshold type, int low = 0, int hight = 255);

	int getThresholdOtsu(Mat input, Mat &output,SRThreshold type);
}
