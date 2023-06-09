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

	//通讯
	QTcpServer *_server = nullptr;
	QTcpSocket* _socket = nullptr;
	void addLog(QString value);
	void createTcpServer(QTcpServer** _server,int port);
	void closeTcpServer(QTcpServer** _server);
	bool connectServer(QTcpSocket** _socket, QString &addr, int &port);
	void disconnectServer(QTcpSocket** _socket);

private:
	void extraction(Mat input, Mat &output);	/*通道提取*/
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
	void drawCross(cv::Point,cv::Point,cv::Point, Mat &, Scalar);//画十字标
	void drawFindLine(Mat&,SRFindLine&, SRroiRect&,int type);//画找直线结果
	void drawFindCircle(Mat&, SRFindCircle&, SRroiCircle&,int distance);//画找圆结果
	void drawPointGroup(Mat&, std::vector<std::vector<cv::Point>> group);//画点数组
	void findPoint();//找点
	void findLine();//找直线
	void findCircle();//找圆
	void findBlob();//Blob分析
	void findContours();//轮廓提取
	void templateMatch();//模板匹配
	void readCode();//读码


	
protected:
	void closeEvent(QCloseEvent *event);//关闭测试界面时执行。
	
public slots:
	void on_pageChanged(int);
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
	void on_findBlob();//全局Blob
	void on_findContours();//轮廓提取
	void on_templateMatch();//模板匹配
	void on_match_new();//新建匹配模板
	void on_match_load();//加载匹配模板
	void on_match_save();//存储匹配模板
	void on_study();//深度学习
	void on_readCode();//读码

	//通讯测试助手
	void on_tcpOpen();
	void on_tcpClear();
	void on_tcpSend();
	void on_tcpSave();
};
