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
	void on_inputClicked();//����ͼƬ��ť�¼�
	void on_reset();//���ô���
	void on_calib();//�궨
	void on_useCalib(bool);//ʹ�ñ궨
	void on_calibChanged(QString);//�л��궨�ļ�
	void on_rotation(int);//��ת
	void on_extraction();//ͨ����ȡ
	void on_threshold();//��ֵ��
	void on_deBugImage();
	void on_do();//ִ��
	void on_cancel();//ȡ��
	void on_kernel();//�Զ����˲���
	void on_kernelSet(int);//�Զ����˲���
	void clearLayout(QLayout*);//����Զ����˲���
	void on_erod();// ��ʴ
	void on_dilat();//����
	void on_openFilter();//������
	void on_closeFilter();//������
	void on_gaussianFilter();//��˹�˲�
	void on_medianFilter();//��ֵ�˲�
	void on_averageFilter();//��ֵ�˲�
	void on_fourierTransformation();//����Ҷ�任
	void on_histogramEqualization();//ֱ��ͼ����
};
