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

		Mat calcHistImage;//灰度图
		Mat gray;//直方图信息
		double max;//最大灰度值
		double min;//最小灰度值
		double average;//平均灰度值
		double stddev;//灰度值方差
	   /*
			函数作用：
			获取直方图
			变量：
			input:输入直方图数据
			output：输出直方图图像
		*/

		static void getCalcHistImage(Mat input, Mat &output);
		/*
			函数作用：
				平滑直方图
			变量：
				input:输入直方图数据
				output：输出直方图数据
				time: 迭代次数
			
eg:	
				SRCalcHist::getSmoothHist(hist.gray, image,3);
				SRCalcHist::getCalcHistImage(image, debugImage);
		*/
		static void getSmoothHist(Mat input, Mat &output, int time = -1);
	};

	//相机类
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
	/*边缘点检测*/
	class SRFindPoint
	{
	public:
			SRFindPoint();
		/*
		* 边缘点检测
		* @param[in] image 输入图像
		* @param[in] roi 直线ROI
		* @param[in] strength 边缘强度
		* @param[in] polarity 边缘极性 0黑->白、1白->黑 2所有
		*/
		SRFindPoint(Mat image, SRroiLine roi, int strength, int polarity);
		vector<int> group;//剖面图数组
		vector<Point> pointGroup;//返回数组
		Mat sectional;//剖面图
		void findPoint(Mat image, SRroiLine roi, int strength, int polarity);

	};
	/*找直线*/
	class SRFindLine
	{
	public:
		SRFindLine();
		/*
		* 找直线
		* @param[in] image 输入图像
		* @param[in] roi 矩形ROI
		* @param[in] strength 边缘强度
		* @param[in] polarity 边缘极性 0黑->白、1白->黑 2所有
		* @param[in] type 边缘类型 0第一条直线 1最后一条直线 2最佳直线
		* @param[in] directionint 搜索方向 0从上到下 1从下到上 2从左到右 3从右到左
		* @param[in] distance 搜索间隔
		*/
		void findLine(Mat image, SRroiRect roi, int strength, int polarity, int type, int directionint, int distance);
		vector<cv::Point> effectivePoint;
		vector<cv::Point> linePoint;
		double angle;
	};
	/*找圆*/
	class SRFindCircle
	{

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

	void getPreprocessImage(Mat input, Mat &output,int type,int shape, int size,int time);
	/*
	函数作用：
			获取二值化图像
		变量：
			input:输入图像
			output：输出图像
			operationType：操作方式
			number：数值 / image:图片
	*/
	void getOperationImage(Mat input, Mat &output,int type, int number);
	void getOperationImage(Mat input, Mat &output,int type, Mat images);
}
