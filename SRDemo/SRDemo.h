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

using namespace SRVision;
cv::Mat QImageToMat(QImage image);
QImage MatToQImage(const cv::Mat& mat);
class SRDemo : public QMainWindow
{
    Q_OBJECT
public:
    SRDemo(QWidget *parent = Q_NULLPTR);
	void refreshImage(Mat &image);
	void refreshCalibBox();
private:
    Ui::SRDemoClass ui;
	Mat sourceImage;
	Mat currentImage;
	Mat debugImage;
	QString imagePath;
	QMessageBox msgBox;
	QLabel statusBarMessage;
	calibResult calib = {0.0};
	QGridLayout * kernel_gridLayout;
public slots:
	void on_inputClicked();//导入图片按钮事件
	void on_reset();//重置处理
	void on_calib();//标定
	void on_useCalib(bool);//使用标定
	void on_calibChanged(QString);//切换标定文件
	void on_rotation(int);//旋转
	void on_extraction();//通道提取
	void on_threshold();//二值化
	void on_deBugImage();
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
};
