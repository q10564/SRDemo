#pragma once

#include <QtWidgets/QMainWindow>
#include<ui_SRDemo.h>

#include "SRVision.h"
#include "SRTools.h"
#include<QProcess>
#include<QDebug>
#include<QFileDialog>
#include<QMessageBox>
#include<QTextCodec>
#include<QSettings>
#include<QPainter>
#include<QStandardItemModel>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
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
	SRFindBlob fblob;
	QStandardItemModel* blob_Resmodel;
	SRFindContour fcontour;
	vector<SRroi> roiGroup;
	SRMatch fmatch;
	SRCode fcode;

	//ͨѶ
	QTcpServer *_server = nullptr;
	QTcpSocket* _socket = nullptr;
	void addLog(QString value);
	void createTcpServer(QTcpServer** _server,int port);
	void closeTcpServer(QTcpServer** _server);
	bool connectServer(QTcpSocket** _socket, QString &addr, int &port);
	void disconnectServer(QTcpSocket** _socket);

private:
	void extraction(Mat input, Mat &output);	/*ͨ����ȡ*/
	void threshold(Mat input, Mat &output);//��ֵ��
	void kernel(Mat input, Mat &output);//�Զ����˲���
	void erod(Mat input, Mat &output);// ��ʴ
	void dilat(Mat input, Mat &output);//����
	void openFilter(Mat input, Mat &output);//������
	void closeFilter(Mat input, Mat &output);//������
	void gaussianFilter(Mat input, Mat &output);//��˹�˲�
	void medianFilter(Mat input, Mat &output);//��ֵ�˲�
	void averageFilter(Mat input, Mat &output);//��ֵ�˲�
	void fourierTransformation(Mat input, Mat &output);//����Ҷ�任
	void histogramEqualization(Mat input, Mat &output);//ֱ��ͼ����
	void imageOperation(Mat input, Mat &output);//ͼ�����
	void drawROI(int);//����ROI
	void drawCross(cv::Point,cv::Point,cv::Point, Mat &, Scalar);//��ʮ�ֱ�
	void drawFindLine(Mat&,SRFindLine&, SRroiRect&,int type);//����ֱ�߽��
	void drawFindCircle(Mat&, SRFindCircle&, SRroiCircle&,int distance);//����Բ���
	void drawPointGroup(Mat&, std::vector<std::vector<cv::Point>> group);//��������
	void findPoint();//�ҵ�
	void findLine();//��ֱ��
	void findCircle();//��Բ
	void findBlob();//Blob����
	void findContours();//������ȡ
	void templateMatch();//ģ��ƥ��
	void readCode();//����


	
protected:
	void closeEvent(QCloseEvent *event);//�رղ��Խ���ʱִ�С�
	
public slots:
	void on_pageChanged(int);
	void on_inputClicked();//����ͼƬ��ť�¼�
	void on_reset();//���ô���
	void on_calib();//�궨
	void on_useCalib(bool);//ʹ�ñ궨
	void on_calibChanged(QString);//�л��궨�ļ�
	void on_rotation(int);//��ת
	void on_deBugImage();
	void on_extraction();//ͨ����ȡ
	void on_threshold();//��ֵ��
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
	void on_imageOperation();//ͼ�����
	void on_cameraChanged(int);//�л�ʹ�����
	void on_cameraOnline();//ʵʱ�ɼ�
	void on_cameraGetImage();//��һ����
	void on_showCamera(Mat);
	void on_scaleUp();//�Ŵ�ͼ��
	void on_scaleDowm();//��Сͼ��
	void on_scaleReal();//ԭʼ��С
	void on_scaleAuto();//����Ӧ
	void on_showFuncImage(QModelIndex);
	void on_getImagePos(QPoint);//��ȡͼ���ϵ�����
	void on_getLeftStartPos(QPoint);//ͼ���������ʱλ��
	void on_getLeftMovePos(QPoint);//ͼ������ƶ�ʱλ��
	void on_getLeftEndPos(QPoint);//ͼ��������½���ʱλ��
	void on_getROI();//��ȡROI
	void on_roiPageChange(int);
	void on_findPoint();//ֱ��ROI�ҵ�
	void on_findLine();//����ROI��ֱ��
	void on_findCircle();//����ROI��Բ
	void on_findBlob();//ȫ��Blob
	void on_findContours();//������ȡ
	void on_templateMatch();//ģ��ƥ��
	void on_match_new();//�½�ƥ��ģ��
	void on_match_load();//����ƥ��ģ��
	void on_match_save();//�洢ƥ��ģ��
	void on_study();//���ѧϰ
	void on_readCode();//����

	//ͨѶ��������
	void on_tcpOpen();
	void on_tcpClear();
	void on_tcpSend();
	void on_tcpSave();
};
