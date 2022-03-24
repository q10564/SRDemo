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
	QString imagePath;
	QMessageBox msgBox;
	QLabel statusBarMessage;
	calibResult calib = {0.0};
public slots:
	void on_inputClicked();//导入图片按钮事件
	void on_reset();//重置处理
	void on_calib();//标定
	void on_useCalib(bool);//使用标定
	void on_calibChanged(QString);//切换标定文件
	void on_rotation(int);//旋转
	void on_extraction();//通道提取
	void on_do();
	void on_cancel();
};
