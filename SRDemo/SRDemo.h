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
	void on_inputClicked();//����ͼƬ��ť�¼�
	void on_reset();//���ô���
	void on_calib();//�궨
	void on_useCalib(bool);//ʹ�ñ궨
	void on_calibChanged(QString);//�л��궨�ļ�
	void on_rotation(int);//��ת
	void on_extraction();//ͨ����ȡ
	void on_do();
	void on_cancel();
};
