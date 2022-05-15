#pragma once
#include <opencv.hpp>
#include <vector>
#include <QObject>
#include <QList>
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
	class SRroi
	{
	public:
		/*
		0 SRroiLine
		1 SRroiRect
		2 SRroiCircle
		*/
		int type;
	};
	class SRroiLine :public SRroi
	{
	public:
		SRroiLine();
		cv::Point start;
		cv::Point end;
	};
	class SRroiRect :public SRroi
	{
	public:
		SRroiRect();
		cv::Point center;
		int width;
		int height;
	};
	class SRroiCircle :public SRroi
	{
	public:
		SRroiCircle();
		cv::Point center;
		double radius_out;
		double radius_in;
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

		Mat calcHistImage;//�Ҷ�ͼ
		Mat gray;//ֱ��ͼ��Ϣ
		double max;//���Ҷ�ֵ
		double min;//��С�Ҷ�ֵ
		double average;//ƽ���Ҷ�ֵ
		double stddev;//�Ҷ�ֵ����
	   /*
			�������ã�
			��ȡֱ��ͼ
			������
			input:����ֱ��ͼ����
			output�����ֱ��ͼͼ��
		*/

		static void getCalcHistImage(Mat input, Mat &output);
		/*
			�������ã�
				ƽ��ֱ��ͼ
			������
				input:����ֱ��ͼ����
				output�����ֱ��ͼ����
				time: ��������
			
eg:	
				SRCalcHist::getSmoothHist(hist.gray, image,3);
				SRCalcHist::getCalcHistImage(image, debugImage);
		*/
		static void getSmoothHist(Mat input, Mat &output, int time = -1);
	};

	//�����
	class SRCamera :public QObject
	{
		Q_OBJECT
	public:
		SRCamera();
		~SRCamera(); 
		QList<VideoCapture> cameraList;
		VideoCapture capture;
		Mat frame;
		bool onlineFlag = false;
		void initCamera();
		void useCamera(int index);
		void destroyCamera();
		void getImage();
		void online();

	signals:
	void sendImage(Mat image);

	};
	/*��Ե����*/
	class SRFindPoint
	{
	public:
			SRFindPoint();
		/*
		* ��Ե����
		* @param[in] image ����ͼ��
		* @param[in] roi ֱ��ROI
		* @param[in] strength ��Եǿ��
		* @param[in] polarity ��Ե���� 0��->�ס�1��->�� 2����
		*/
		SRFindPoint(Mat image, SRroiLine roi, int strength, int polarity);
		vector<int> group;//����ͼ����
		vector<Point> pointGroup;//��������
		Mat sectional;//����ͼ
		void findPoint(Mat image, SRroiLine roi, int strength, int polarity);

	};
	/*��ֱ��*/
	class SRFindLine
	{
	public:
		SRFindLine();
		/*
		* ��ֱ��
		* @param[in] image ����ͼ��
		* @param[in] roi ����ROI
		* @param[in] strength ��Եǿ��
		* @param[in] polarity ��Ե���� 0��->�ס�1��->�� 2����
		* @param[in] type ��Ե���� 0��һ��ֱ�� 1���һ��ֱ�� 2���ֱ��
		* @param[in] directionint �������� 0���ϵ��� 1���µ��� 2������ 3���ҵ���
		* @param[in] distance �������
		*/
		void findLine(Mat image, SRroiRect roi, int strength, int polarity, int type, int directionint, int distance);
		vector<cv::Point> edgePoints;
		vector<cv::Point> effectivePoints;
		vector<cv::Point> linePoint;
		double angle;

	};
	/*��Բ*/
	class SRFindCircle
	{
	public:
		/*
		* ��Բ
		* @param[in] image ����ͼ��
		* @param[in] roi ԲROI
		* @param[in] strength ��Եǿ��
		* @param[in] polarity ��Ե���� 0��->�ס�1��->�� 2����
		* @param[in] type ��Ե���� 0��һ��ֱ�� 1���һ��ֱ�� 2���ֱ��
		* @param[in] distance �������
		*/
		void findCircle(Mat image,SRroiCircle roi,int syrength, int polarity, int type, int distance);
		vector<cv::Point> edgePoints;
		vector<cv::Point> effectivePoints;
		cv::Point center;
		double radius;
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

	void getPreprocessImage(Mat input, Mat &output,int type,int shape, int size,int time);
	/*
	�������ã�
			��ȡ��ֵ��ͼ��
		������
			input:����ͼ��
			output�����ͼ��
			operationType��������ʽ
			number����ֵ / image:ͼƬ
	*/
	void getOperationImage(Mat input, Mat &output,int type, int number);
	void getOperationImage(Mat input, Mat &output,int type, Mat images);
}
