#pragma once

#include <QtWidgets/QMainWindow>
#include<ui_SRDemo.h>

#include "SRVision.h"
#include "SRTools.h"

#include<QDebug>
#include<QFileDialog>
#include<QMessageBox>
#include<QTextCodec>
#include<QSettings>
#include<QPainter>

using namespace SRVision;
cv::Mat QImageToMat(QImage image);
QImage MatToQImage(const cv::Mat& mat);
class SRDemo : public QMainWindow
{
    Q_OBJECT
public:
    SRDemo(QWidget *parent = Q_NULLPTR);
	void refreshImage(Mat &image);
	void refreshImagePos(QPoint, QPoint&);
	void refreshCalibBox();
private:
    Ui::SRDemoClass ui;
	SRCamera camera;
	double image_scale = 1.0;
	double image_scale_offset = 0.0;
	int image_h;
	int image_w;
	QPoint roiStart;
	QPoint roiEnd;
	bool debugFlag = false;
	Mat sourceImage;
	Mat currentImage;
	Mat debugImage;
	QString imagePath;
	QMessageBox msgBox;
	QLabel statusBarMessage;
	calibResult calib = {0.0};
	QGridLayout * kernel_gridLayout;
	typedef void (SRDemo::*func)(Mat, Mat&);
	QList<func> funcList;
	QList<Mat> imageList;
	QStringList funcValueList;
	QStringListModel *funcValueModel;
	SRFindPoint fpoint;
	SRFindLine fline;
	SRFindCircle fcircle;
	vector<SRroi> roiGroup;
private:
	/*通道提取*/
	void extraction(Mat input, Mat &output);
	void threshold(Mat input, Mat &output);//二值化
	void kernel(Mat input, Mat &output);//自定义滤波核
	void erod(Mat input, Mat &output);// 腐蚀
	void dilat(Mat input, Mat &output);//膨胀
	void openFilter(Mat input, Mat &output);//开运算
	void closeFilter(Mat input, Mat &output);//闭运算
	void gaussianFilter(Mat input, Mat &output);//高斯滤波
	void medianFilter(Mat input, Mat &output);//中值滤波
	void averageFilter(Mat input, Mat &output);//均值滤波
	void fourierTransformation(Mat input, Mat &output);//傅里叶变换
	void histogramEqualization(Mat input, Mat &output);//直方图均衡
	void imageOperation(Mat input, Mat &output);//图像操作
	void drawROI(int);//画出ROI
	void drawCross(cv::Point, Mat &);//画十字标
	void findPoint();//找点
	void findLine();//找直线
	void findCircle();//找圆

	
protected:
	void closeEvent(QCloseEvent *event);//关闭测试界面时执行。
	
public slots:
	void on_inputClicked();//导入图片按钮事件
	void on_reset();//重置处理
	void on_calib();//标定
	void on_useCalib(bool);//使用标定
	void on_calibChanged(QString);//切换标定文件
	void on_rotation(int);//旋转
	void on_deBugImage();
	void on_extraction();//通道提取
	void on_threshold();//二值化
	void on_do();//执行
	void on_cancel();//取消
	void on_kernel();//自定义滤波核
	void on_kernelSet(int);//自定义滤波核
	void clearLayout(QLayout*);//清除自定义滤波核
	void on_erod();// 腐蚀
	void on_dilat();//膨胀
	void on_openFilter();//开运算
	void on_closeFilter();//闭运算
	void on_gaussianFilter();//高斯滤波
	void on_medianFilter();//中值滤波
	void on_averageFilter();//均值滤波
	void on_fourierTransformation();//傅里叶变换
	void on_histogramEqualization();//直方图均衡
	void on_imageOperation();//图像操作
	void on_cameraChanged(int);//切换使用相机
	void on_cameraOnline();//实时采集
	void on_cameraGetImage();//拍一张照
	void on_showCamera(Mat);
	void on_scaleUp();//放大图像
	void on_scaleDowm();//缩小图像
	void on_scaleReal();//原始大小
	void on_scaleAuto();//自适应
	void on_showFuncImage(QModelIndex);
	void on_getImagePos(QPoint);//获取图像上的坐标
	void on_getLeftStartPos(QPoint);//图像左键按下时位置
	void on_getLeftMovePos(QPoint);//图像左键移动时位置
	void on_getLeftEndPos(QPoint);//图像左键按下结束时位置
	void on_getROI();//获取ROI
	void on_roiPageChange(int);
	void on_findPoint();//直线ROI找点
	void on_findLine();//矩形ROI找直线
	void on_findCircle();//环形ROI找圆

};
