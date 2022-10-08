#include "SRDemo.h"

SRDemo::SRDemo(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	refreshCalibBox();
	kernel_gridLayout = new QGridLayout();
	//初始化相机
	ui.camera_comboBox->clear();
	QStringList strList;
	for (int i = 0; i < camera.cameraList.length(); i++)
	{
		strList.append("camera--" + QString::number(i));
	}
	ui.camera_comboBox->addItems(strList);
	funcValueModel = new QStringListModel(this);
	connect(&camera, &SRCamera::sendImage, this, &SRDemo::on_showCamera);
}
void SRDemo::closeEvent(QCloseEvent * event)
{
	camera.onlineFlag = false;
	camera.destroyCamera();
}
void SRDemo::extraction(Mat input,Mat &output)
{
	Mat mv[3];
	Mat image = input.clone();
	if (ui.radioButton_RGB->isChecked())
	{
		qDebug() << "use RGB";
		split(input, mv);
		switch (ui.comboBox_RGB->currentIndex())
		{
		case 0:output = input; break;//RGB模式
		case 1:output = mv[2]; break;//R
		case 2:output = mv[1]; break;//G
		case 3:output = mv[0]; break;//B
		}
	}
	else if (ui.radioButton_HSL->isChecked())
	{
		qDebug() << "use HSL";
		cvtColor(input, image, COLOR_BGR2HLS);
		split(image, mv);
		switch (ui.comboBox_HSL->currentIndex())
		{
		case 0:output = input; break;//HSL模式
		case 1:output = mv[0]; break;//H色调
		case 2:output = mv[2]; break;//S饱和度
		case 3:output = mv[1]; break;//L亮度
		}
	}
	else if (ui.radioButton_HSV->isChecked())
	{
		qDebug() << "use HSV";
		cvtColor(input, image, COLOR_BGR2HSV);
		split(image, mv);
		switch (ui.comboBox_HSV->currentIndex())
		{
		case 0:output = input; break;//HSL模式
		case 1:output = mv[0]; break;//H色调
		case 2:output = mv[1]; break;//S饱和度
		case 3:output = mv[2]; break;//L明度
		}
	}
	else if (ui.radioButton_HSI->isChecked())
	{
		qDebug() << "use HSI";
		image = cvtColor_RGB2HSI(input);
		split(image, mv);
		switch (ui.comboBox_HSI->currentIndex())
		{
		case 0:output = input; break;
		case 1:output = mv[0]; break;//H色调
		case 2:output = mv[1]; break;//S饱和度
		case 3:output = mv[2]; break;//L明度
		}
	}
}
void SRDemo::threshold(Mat input, Mat & output)
{
	ui.Threshold_Slider_high->setEnabled(true);
	ui.Threshold_Slider_low->setEnabled(true);
	ui.Threshold_spinBox_high->setEnabled(true);
	ui.Threshold_spinBox_low->setEnabled(true);
	//阈值信息
	int hight, low;
	hight = ui.Threshold_spinBox_high->value();
	low = ui.Threshold_spinBox_low->value();
	//获取灰度直方图
	SRCalcHist hist(input);
	ui.Threshold_lline_min->setText(QString::number(hist.min));
	ui.Threshold_lline_max->setText(QString::number(hist.max));
	ui.Threshold_line_average->setText(QString::number(hist.average));
	ui.label_histImage->setPixmap(QPixmap::fromImage(MatToQImage(hist.calcHistImage)));
	if (ui.Threshold_radioButton_black->isChecked())//黑色对象
	{
		if (ui.Threshold_comboBox_type->currentIndex() == 0)//手动二值化
		{
			SRVision::getThresholdImage(input, output, SRVision::Threshold_0, low, hight);
		}
		if (ui.Threshold_comboBox_type->currentIndex() == 1)//大津法二值化
		{
			ui.Threshold_Slider_high->setEnabled(false);
			ui.Threshold_Slider_low->setEnabled(false);
			ui.Threshold_spinBox_high->setEnabled(false);
			ui.Threshold_spinBox_low->setEnabled(false);
			//SRVision::getThresholdImage(currentImage, debugImage, SRVision::Threshold_2);
			int threshold = getThresholdImage(input, output, Threshold_2);
			ui.Threshold_Slider_high->setValue(threshold);
			ui.Threshold_Slider_low->setValue(0);
		}

	}
	else if (ui.Threshold_radioButton_white->isChecked())//白色对象
	{
		if (ui.Threshold_comboBox_type->currentIndex() == 0)//手动二值化
		{
			SRVision::getThresholdImage(input, output, SRVision::Threshold_1, low, hight);
		}
		if (ui.Threshold_comboBox_type->currentIndex() == 1)//大津法二值化
		{

			ui.Threshold_Slider_high->setEnabled(false);
			ui.Threshold_Slider_low->setEnabled(false);
			ui.Threshold_spinBox_high->setEnabled(false);
			ui.Threshold_spinBox_low->setEnabled(false);
			//SRVision::getThresholdImage(currentImage, debugImage, SRVision::Threshold_3);
			int threshold = getThresholdImage(input, output, Threshold_3);
			ui.Threshold_Slider_high->setValue(255);
			ui.Threshold_Slider_low->setValue(threshold);
		}
	}
}
void SRDemo::kernel(Mat input, Mat & output)
{
	int kernel_size = kernel_gridLayout->rowCount();//获取行列
	int *value;
	float sum = 0.0;
	Mat kernel = (Mat_<float>(kernel_size, kernel_size));
	for (int i = 0; i < kernel_size; i++)
	{
		for (int j = 0; j < kernel_size; j++)
		{
			kernel.ptr<float>(i)[j] = ((QLineEdit*)(kernel_gridLayout->itemAtPosition(i, j)->widget()))->text().toFloat();
			sum += kernel.ptr<float>(i)[j];
			qDebug() << kernel.ptr<float>(i)[j];
		}

		//qDebug() << ((QLineEdit*)(kernel_gridLayout->itemAtPosition(i,j)->widget()))->text().toFloat();
	}
	for (int i = 0; i < kernel_size; i++)
	{
		for (int j = 0; j < kernel_size; j++)
		{
			//kernel.ptr<float>(i)[j] = kernel.ptr<float>(i)[j] / sum;
			kernel.ptr<float>(i)[j] = kernel.ptr<float>(i)[j];
		}

		//qDebug() << ((QLineEdit*)(kernel_gridLayout->itemAtPosition(i,j)->widget()))->text().toFloat();
	}
	filter2D(input, output, -1, kernel);
}
void SRDemo::erod(Mat input, Mat & output)
{
	int shape = ui.erod_comboBox_shape->currentIndex();
	int size = ui.erod_comboBox_size->currentIndex();
	int time = ui.erod_spinBox->value();
	getPreprocessImage(input, output, 4, shape, size, time);
	SRCalcHist hist(output);
	ui.erod_label_histImage->setPixmap(QPixmap::fromImage(MatToQImage(hist.calcHistImage)));
}
void SRDemo::dilat(Mat input, Mat & output)
{
	int shape = ui.dilat_comboBox_shape->currentIndex();
	int size = ui.dilat_comboBox_size->currentIndex();
	int time = ui.dilat_spinBox->value();
	getPreprocessImage(input, output, 5, shape, size, time);
	SRCalcHist hist(output);
	ui.dilat_label_histImage->setPixmap(QPixmap::fromImage(MatToQImage(hist.calcHistImage)));
}
void SRDemo::openFilter(Mat input, Mat & output)
{
	int shape = ui.open_comboBox_shape->currentIndex();
	int size = ui.open_comboBox_size->currentIndex();
	int time = ui.open_spinBox->value();
	getPreprocessImage(input, output, 6, shape, size, time);
	SRCalcHist hist(output);
	ui.open_label_histImage->setPixmap(QPixmap::fromImage(MatToQImage(hist.calcHistImage)));
}
void SRDemo::closeFilter(Mat input, Mat & output)
{
	int shape = ui.close_comboBox_shape->currentIndex();
	int size = ui.close_comboBox_size->currentIndex();
	int time = ui.close_spinBox->value();
	getPreprocessImage(input, output, 7, shape, size, time);
	SRCalcHist hist(output);
	ui.close_label_histImage->setPixmap(QPixmap::fromImage(MatToQImage(hist.calcHistImage)));
}
void SRDemo::gaussianFilter(Mat input, Mat & output)
{
	int size = ui.gauss_spinBox->value();
	Size sizes = Size((size + 1) * 2 + 1, (size + 1) * 2 + 1);
	cv::GaussianBlur(input, output, sizes, 0.0);
	SRCalcHist hist(output);
	ui.gauss_label_histImage->setPixmap(QPixmap::fromImage(MatToQImage(hist.calcHistImage)));
}
void SRDemo::medianFilter(Mat input, Mat & output)
{
	int size = ui.median_spinBox->value();
	cv::medianBlur(input, output, size);
	SRCalcHist hist(output);
	ui.median_label_histImage->setPixmap(QPixmap::fromImage(MatToQImage(hist.calcHistImage)));
}
void SRDemo::averageFilter(Mat input, Mat & output)
{
	int size = ui.average_spinBox->value();
	Size sizes = Size((size + 1) * 2 + 1, (size + 1) * 2 + 1);
	cv::boxFilter(input, output, -1, sizes);
	SRCalcHist hist(output);
	ui.average_label_histImage->setPixmap(QPixmap::fromImage(MatToQImage(hist.calcHistImage)));
}
void SRDemo::fourierTransformation(Mat input, Mat & output)
{
	output = input;
}
void SRDemo::histogramEqualization(Mat input, Mat & output)
{
	output = input;
}
void SRDemo::imageOperation(Mat input, Mat & output)
{
	int type = ui.operation_combox->currentIndex();
	if (ui.operation_radioButton_number->isChecked())
	{
		getOperationImage(input, output, type, ui.operation_spinBox->value());
	}
	else if (ui.operation_radioButton_image->isChecked())
	{
		int index = ui.operation_comboBox_image->currentIndex();
		getOperationImage(input, output, type, imageList[index]);
	}

}
/*
	在图像上画出ROI
	type:
	-1://默认，ROI生成算子用，选择的type
	0://直线 
	1://矩形 
	2://圆形 
	3://点集
*/
void SRDemo::drawROI(int type = -1)
{
	cv::Point p1(roiStart.x(), roiStart.y());
	cv::Point p2(roiEnd.x(), roiEnd.y());
	Mat image = debugImage.clone();
	if (type == -1)
		type = ui.roi_type->currentIndex();
	if (currentImage.type() == CV_8UC1)
	{
		cvtColor(debugImage, image, COLOR_GRAY2BGR);
	}
	switch (type)
	{
	case 0://直线
	{	
		arrowedLine(image, p1, p2, Scalar(255, 0, 0), 1, 8, 0, 0.025);
		ui.roi_line_startX->setValue(roiStart.x());
		ui.roi_line_startY->setValue(roiStart.y());
		ui.roi_line_endX->setValue(roiEnd.x());
		ui.roi_line_endY->setValue(roiEnd.y());
		break;
	}
	case 1://矩形
	{
		rectangle(image, p1, p2, Scalar(255, 0, 0), 1, 8);
		ui.roi_rect_centerX->setValue(double(p1.x + p2.x) / 2);
		ui.roi_rect_centerY->setValue(double(p1.y + p2.y) / 2);
		ui.roi_rect_center_width->setValue(abs(p1.x - p2.x));
		ui.roi_rect_center_height->setValue(abs(p1.y - p2.y));
		ui.roi_rect_angle->setValue(0);
		break;
	}
	case 2://圆形
	{
		double radius_in = ui.roi_circle_radius_in->value();
		double radius = sqrt(powf((p1.x - p2.x), 2) + powf((p1.y - p2.y), 2));
		if (radius_in >= radius)
			return;
		cv::circle(image, p1, radius, Scalar(255, 0, 0), 1, 8);
		cv::circle(image, p1, radius_in, Scalar(255, 0, 0), 1, 8);
		cv::circle(image, p1, 2, Scalar(0, 255, 0), -1, 8);
		ui.roi_circle_centerX->setValue(p1.x);
		ui.roi_circle_centerY->setValue(p1.y);
		ui.roi_circle_radius_out->setValue(radius);
		break;
	}
	case 3:break;//点集
	default:break;
	}
	refreshImage(image);
}
//画十字标
void SRDemo::drawCross(cv::Point p1,cv::Point p2,cv::Point point,Mat &image, Scalar color)
{
	Mat im = image.clone();
	if (currentImage.type() == CV_8UC1)
	{
		cvtColor(image, im, COLOR_GRAY2BGR);
	}
	//绘制横线
	line(im, cv::Point(point.x - 4 / 2, point.y), cv::Point(point.x + 4 / 2, point.y), color, 1, 8, 0);
	//绘制竖线
	line(im, cv::Point(point.x, point.y - 4 / 2), cv::Point(point.x, point.y + 4 / 2), color, 1, 8, 0);
	//绘制ROI
	arrowedLine(im, p1, p2, Scalar(255, 0, 0), 1, 8, 0, 0.025);
	refreshImage(im);
}

void SRDemo::drawFindLine(Mat &image, SRFindLine &srLine, SRroiRect &roi, int type)
{
	Mat im = image.clone();
	if (currentImage.type() == CV_8UC1)
	{
		cvtColor(image, im, COLOR_GRAY2BGR);
	}
	cv::Point LeftTop = cv::Point(roi.center.x - round(roi.height / 2), roi.center.y - round(roi.width / 2));
	cv::Point RightDown = cv::Point(roi.center.x + round(roi.height / 2), roi.center.y + round(roi.width / 2));
	line(im, srLine.linePoint[0], srLine.linePoint[1], Scalar(0, 255, 0), 1, 8, 0);
	rectangle(im, LeftTop, RightDown, Scalar(255, 0, 0), 1, 8);
	switch (type)
	{
	case 0://从上到下
	{	
		if (ui.findLine_show->isChecked())
		{
			for (int i = LeftTop.x; i < RightDown.x; i += ui.findLine_distance->value())
			{
				//绘制ROI
				arrowedLine(im, cv::Point(i, LeftTop.y), cv::Point(i, RightDown.y), Scalar(255, 0, 0), 1, 8, 0, 0.025);
			}
		}
		break;
	}
	case 1://从下到上
	{
		if (ui.findLine_show->isChecked())
		{
			for (int i = RightDown.x; i > LeftTop.x; i -= ui.findLine_distance->value())
			{
				//绘制ROI
				arrowedLine(im, cv::Point(i, RightDown.y), cv::Point(i, LeftTop.y), Scalar(255, 0, 0), 1, 8, 0, 0.025);
			}
		}
		break;
	}
	case 2://从左到右
	{
		if (ui.findLine_show->isChecked())
		{
			for (int i = LeftTop.y; i < RightDown.y; i += ui.findLine_distance->value())
			{
				//绘制ROI
				arrowedLine(im, cv::Point(LeftTop.x, i), cv::Point(RightDown.x, i), Scalar(255, 0, 0), 1, 8, 0, 0.025);
			}
		}
		break;
	}
	case 3://从右到左
	{
		if (ui.findLine_show->isChecked())
		{
			for (int i = RightDown.y; i > LeftTop.y; i -= ui.findLine_distance->value())
			{
				//绘制ROI
				arrowedLine(im, cv::Point(RightDown.x, i), cv::Point(LeftTop.x, i), Scalar(255, 0, 0), 1, 8, 0, 0.025);
			}
		}
		break;
	}
	default:
		break;
	}
	for (size_t j = 0; j < srLine.edgePoints.size(); ++j)
	{

		if (count(srLine.effectivePoints.begin(), srLine.effectivePoints.end(), srLine.edgePoints.at(j)))
		{
			//绘制横线
			line(im, cv::Point(srLine.edgePoints.at(j).x - 4 / 2, srLine.edgePoints.at(j).y), cv::Point(srLine.edgePoints.at(j).x + 4 / 2, srLine.edgePoints.at(j).y), Scalar(0, 255, 0), 1, 8, 0);
			//绘制竖线
			line(im, cv::Point(srLine.edgePoints.at(j).x, srLine.edgePoints.at(j).y - 4 / 2), cv::Point(srLine.edgePoints.at(j).x, srLine.edgePoints.at(j).y + 4 / 2), Scalar(0, 255, 0), 1, 8, 0);
		}
		else
		{
			//绘制横线
			line(im, cv::Point(srLine.edgePoints.at(j).x - 4 / 2, srLine.edgePoints.at(j).y), cv::Point(srLine.edgePoints.at(j).x + 4 / 2, srLine.edgePoints.at(j).y), Scalar(0, 0, 255), 1, 8, 0);
			//绘制竖线
			line(im, cv::Point(srLine.edgePoints.at(j).x, srLine.edgePoints.at(j).y - 4 / 2), cv::Point(srLine.edgePoints.at(j).x, srLine.edgePoints.at(j).y + 4 / 2), Scalar(0, 0, 255), 1, 8, 0);
		}
	}
	refreshImage(im);
}

void SRDemo::drawFindCircle(Mat &image, SRFindCircle &circle, SRroiCircle &roi, int distance)
{
	Mat im = image.clone();
	if (currentImage.type() == CV_8UC1)
	{
		cvtColor(image, im, COLOR_GRAY2BGR);
	}
	if (ui.findCircle_show->isChecked())
	{
		for (int i = 0; i < 360; i += distance)
		{
			SRroiLine lineroi;
			lineroi.start.x = roi.radius_in*cos(PI / 180.0 * i) + roi.center.x;
			lineroi.start.y = roi.radius_in*sin(PI / 180.0 * i) + roi.center.y;

			lineroi.end.x = roi.radius_out*cos(PI / 180.0 * i) + roi.center.x;
			lineroi.end.y = roi.radius_out*sin(PI / 180.0 * i) + roi.center.y;

			//绘制ROI
			arrowedLine(im, lineroi.start, lineroi.end, Scalar(255, 0, 0), 1, 8, 0, 0.025);
		}
	}
	for (size_t j = 0; j < circle.edgePoints.size(); ++j)
	{

		if (count(circle.effectivePoints.begin(), circle.effectivePoints.end(), circle.edgePoints.at(j)))
		{
			//绘制横线
			line(im, cv::Point(circle.edgePoints.at(j).x - 4 / 2, circle.edgePoints.at(j).y), cv::Point(circle.edgePoints.at(j).x + 4 / 2, circle.edgePoints.at(j).y), Scalar(0, 255, 0), 1, 8, 0);
			//绘制竖线
			line(im, cv::Point(circle.edgePoints.at(j).x, circle.edgePoints.at(j).y - 4 / 2), cv::Point(circle.edgePoints.at(j).x, circle.edgePoints.at(j).y + 4 / 2), Scalar(0, 255, 0), 1, 8, 0);
		}
		else
		{
			//绘制横线
			line(im, cv::Point(circle.edgePoints.at(j).x - 4 / 2, circle.edgePoints.at(j).y), cv::Point(circle.edgePoints.at(j).x + 4 / 2, circle.edgePoints.at(j).y), Scalar(0, 0, 255), 1, 8, 0);
			//绘制竖线
			line(im, cv::Point(circle.edgePoints.at(j).x, circle.edgePoints.at(j).y - 4 / 2), cv::Point(circle.edgePoints.at(j).x, circle.edgePoints.at(j).y + 4 / 2), Scalar(0, 0, 255), 1, 8, 0);
		}
	}
	cv::circle(im,circle.center, circle.radius,Scalar(0, 255, 0), 1,8);
	refreshImage(im);
}
/*绘制二维点数组*/
void SRDemo::drawPointGroup(Mat &image, std::vector<std::vector<cv::Point>> group)
{
	Mat im = image.clone();
	if (currentImage.type() == CV_8UC1)
	{
		cvtColor(image, im, COLOR_GRAY2BGR);
	}
	/*
	for (int j = 0; j < group.size(); j++)
	{
		int R = rand() % 256;
		int G = rand() % 256;
		int B = rand() % 256;
		cv::Scalar color(R, G, B);
		cv::drawContours(im, group, j, color, 2);
	}
	*/
	for (vector<vector<cv::Point>>::iterator it = group.begin(); it != group.end(); it++)
	{
		int R = rand() % 256;
		int G = rand() % 256;
		int B = rand() % 256;
		cv::Scalar color(R, G, B);
		for (vector<cv::Point>::iterator itt = (*it).begin(); itt != (*it).end(); itt++)
		{
			cv::circle(im, (*itt), 2, color, -1);
		}
	}
	refreshImage(im);
}


void SRDemo::findPoint()
{
	int strength = ui.findPoint_strength->value();
	int polarity = ui.findPoint_polarity->currentIndex();
	int type = ui.findPoint_type->currentIndex();
	SRroiLine line;
	line.start = cv::Point(roiStart.x(), roiStart.y());
	line.end = cv::Point(roiEnd.x(), roiEnd.y());
	fpoint.findPoint(currentImage, line, strength, polarity);
	if (fpoint.pointGroup.empty())
		return;
	ui.findPoint_label->setPixmap(QPixmap::fromImage(MatToQImage(fpoint.sectional)));
	drawCross(cv::Point(roiStart.x(), roiStart.y()), cv::Point(roiEnd.x(), roiEnd.y()),fpoint.pointGroup[0], debugImage, Scalar(0, 255, 0));
}

void SRDemo::findLine()
{
	int strength = ui.findLine_strength->value();//边缘强度
	int polarity = ui.findLine_polarity->currentIndex();//边缘极性
	int type = ui.findLine_type->currentIndex();//边缘类型
	int direction = ui.findLine_direction->currentIndex();//搜索方向
	int distance = ui.findLine_distance->value();//搜索间隔
	SRroiRect rect;
	rect.center = cv::Point(double(roiStart.x() + roiEnd.x()) / 2, double(roiStart.y() + roiEnd.y()) / 2);
	rect.height = abs(roiStart.x() - roiEnd.x());
	rect.width = abs(roiStart.y() - roiEnd.y());
	fline.findLine(currentImage, rect, strength, polarity, type, direction,distance);
	if (fline.linePoint.empty())
		return;
	ui.findLine_firstX->setText(QString::number(fline.linePoint[0].x));
	ui.findLine_firstY->setText(QString::number(fline.linePoint[0].y));
	ui.findLine_secondX->setText(QString::number(fline.linePoint[1].x));
	ui.findLine_secondY->setText(QString::number(fline.linePoint[1].y));
	ui.findLine_angle->setText(QString::number(fline.angle));
	drawFindLine(currentImage, fline, rect, direction);
}

void SRDemo::findCircle()
{
	int strength = ui.findCircle_strength->value();//边缘强度
	int polarity = ui.findCircle_polarity->currentIndex();//边缘极性
	int type = ui.findCircle_type->currentIndex();//边缘类型
	int distance = ui.findCircle_distance->value();//搜索间隔

	cv::Point p1(roiStart.x(), roiStart.y());
	cv::Point p2(roiEnd.x(), roiEnd.y());
	SRroiCircle cir;
	cir.center = cv::Point(roiStart.x(), roiStart.y());
	cir.radius_in = ui.findCircle_radius_in->value();
	cir.radius_out = sqrt(powf((p1.x - p2.x), 2) + powf((p1.y - p2.y), 2));

	fcircle.findCircle(currentImage, cir, strength, polarity, type, distance);
	ui.findCircle_radius->setText(QString::number(fcircle.radius));
	ui.findCircle_CenterX->setText(QString::number(fcircle.center.x));
	ui.findCircle_CenterY->setText(QString::number(fcircle.center.y));
	
	drawFindCircle(currentImage, fcircle, cir, distance);

}

void SRDemo::findBlob()
{
	SRVision::BlobControl c;

	fblob.findBlob(currentImage, currentImage, c);
}

void SRDemo::findContours()
{
	SRroiRect rect;
	int filter = ui.findContours_filter->currentIndex();
	int size = ui.findContours_filterNum->value();
	int min = ui.findContours_low->value();
	int max = ui.findContours_high->value();
	int minLength = ui.findContours_min->value();
	int maxLength = ui.findContours_max->value();
	int zoom = ui.findContours_zoom->value();//缩放/外扩
	int step = ui.findContours_step->value();
	int smooth = ui.findContours_smooth->value();
	rect.center = cv::Point(int(roiStart.x() + roiEnd.x()) / 2, int(roiStart.y() + roiEnd.y()) / 2);
	rect.height = abs(roiStart.x() - roiEnd.x());
	rect.width = abs(roiStart.y() - roiEnd.y());
	fcontour.findContour(currentImage, rect, filter, size, min, max, minLength, maxLength, zoom, step, smooth);
	drawPointGroup(currentImage,fcontour.result);
	
}
//模板匹配
void SRDemo::templateMatch()
{
	int level = ui.match_combox_level->currentIndex();
	int startAngle = ui.match_box_startAngle->value();
	int endAngle = ui.match_box_endAngle->value();
	int angleStep = ui.match_box_angleStep->value();
	int grade = ui.match_box_grade->value();
	int count = ui.match_box_count
->value();
	if (fmatch.templateImg.empty())
		return;
	fmatch.templateMatch(currentImage, fmatch.templateImg, level, startAngle, endAngle, angleStep, grade, count);
	
	refreshImage(fmatch.resultImg);
}

void SRDemo::on_inputClicked()
{
	imagePath = QFileDialog::getOpenFileName(this, tr("打开图片"), "../image", tr("Images (*.png *.bmp *.jpg)"));
	QFileInfo fileinfo = QFileInfo(imagePath);
	sourceImage = imread(imagePath.toStdString());
	for (int i = 0; i < funcList.length(); i++)
	{
		(this->*funcList[i])(sourceImage, sourceImage);
		imageList[i] = sourceImage;
	}
	currentImage = sourceImage;
	refreshImage(sourceImage);
	ui.line_path->setText(fileinfo.fileName());
	ui.horizontalSlider->setValue(0);
}
void SRDemo::on_reset()
{
	funcList.clear();
	funcValueList.clear();
	imageList.clear();
	funcValueModel->setStringList(funcValueList);
	ui.func_list->setModel(funcValueModel);
	if (sourceImage.empty())
	{
		return;
	}
	currentImage = sourceImage;
	refreshImage(sourceImage);
	ui.horizontalSlider->setValue(0);
}
void SRDemo::refreshImage(Mat &image)
{
	if (image.empty())
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"图像为空");
		msgBox.exec();
		return;
	}
	ui.label_image->clear();
	int wMax = ui.label_image->width();//显示的最大宽度
	int hMax = ui.label_image->height();//显示的最大高度
	image_h = image.rows;//图片高度
	image_w = image.cols;//图片宽度
	//QPainter painter(ui.label_image);
	if (image_h <= hMax && image_w <= wMax)
	{
		image_scale = 1.0;
		//ui.label_image->setPixmap(QPixmap::fromImage(MatToQImage(image)));
		Mat dst;
		cv::resize(image, dst, Size(0, 0), image_scale + image_scale_offset, image_scale + image_scale_offset, INTER_LINEAR);
		ui.label_image->setPixmap(QPixmap::fromImage(MatToQImage(dst)));
	}
	else
	{
		double scalew = double(wMax) / double(image_w);
		double scaleh = double(hMax) / double(image_h);
		image_scale = MIN(scalew, scaleh);
		printf("scalew: %f\nscaleh: %f\nscale: %f\n", scalew, scaleh, image_scale);
		Mat dst;
		cv::resize(image, dst, Size(0, 0), image_scale + image_scale_offset, image_scale + image_scale_offset, INTER_LINEAR);
		ui.label_image->setPixmap(QPixmap::fromImage(MatToQImage(dst)));
		//painter.drawImage(QPoint(0, 0), MatToQImage(dst));
	}
	statusBarMessage.setText(QString("(%1 x %2) scale :%3 (%4,%5)").arg(image_w).arg(image_h).arg(image_scale + image_scale_offset).arg(0).arg(0));
	ui.statusBar->addWidget(&statusBarMessage);
	
}
/*获取到控件坐标后首先调用refreshImagePos将控件坐标转换到图像坐标*/
void SRDemo::refreshImagePos(QPoint input , QPoint &output)
{
	double h = (image_h * (image_scale + image_scale_offset) - ui.label_image->height()) / 2;
	double w = (image_w * (image_scale + image_scale_offset) - ui.label_image->width()) / 2;
	double x_in_label = (input.x() + round(w)) / (image_scale + image_scale_offset);
	double y_in_label = (input.y() + round(h)) / (image_scale + image_scale_offset);
	output.setX(round(x_in_label));
	output.setY(round(y_in_label));
}
void SRDemo::refreshCalibBox()
{
	QDir dir("..//calibration");
	if (!dir.exists())
		return;

	//查看路径中后缀为.cfg格式的文件
	QStringList filters;
	filters << QString("*.xml");
	dir.setFilter(QDir::Files | QDir::NoSymLinks); //设置类型过滤器，只为文件格式
	dir.setNameFilters(filters);  //设置文件名称过滤器，只为filters

	int dir_count = dir.count();
	if (dir_count <= 0)
		return;

	QStringList string_list;
	for (int i = 0; i < dir_count; i++)
	{
		QString file_name = dir[i];  //文件名称
		//ts<<file_name<<"\r\n"<<"\r\n";
		string_list.append(file_name);
	}
	ui.comboBox_calib->clear();
	ui.comboBox_calib->addItems(string_list);
}
void SRDemo::on_calib()
{
	if (currentImage.empty())
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"源图像异常");
		msgBox.exec();
		return;
	}
	vector<cv::Point2f> pix;
	vector<cv::Point2f> world;
	calibResult calib = { 0.0 };
	/*标定测试开始*/
	//////////////////////
	cv::Mat dst, gray_src, src;
	cv::Point2f cc;

	src = currentImage.clone();
	//src = cv::imread("..\\image\\calib.bmp");
	//二值化图像
	cv::cvtColor(src, gray_src, cv::COLOR_BGRA2GRAY);
	cv::threshold(gray_src, gray_src, 0, 255, cv::THRESH_OTSU | cv::THRESH_BINARY);
	//开运算处理图像噪点		
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(-1, -1));
	cv::morphologyEx(gray_src, dst, cv::MORPH_OPEN, kernel, cv::Point(-1, -1));
	//求图像中轮廓
	vector <vector<cv::Point>> contours;
	vector<cv::Vec4i> hireachy;
	cv::findContours(dst, contours, hireachy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point());

	//循环轮廓，将符合条件得轮廓拟合圆
	for (size_t i = 0; i < contours.size(); i++)
	{
		double area = cv::contourArea(contours[i]);//面积
		if (area < 1000 || contours[i].size() < 30) continue;//面积或点数过小得排除
		cv::RotatedRect rect = cv::fitEllipse(contours[i]);//拟合椭圆
		float w = rect.size.width;
		float h = rect.size.height;
		float ratio = w / h;//求长宽比，剔除掉不圆的轮廓
		if (ratio > 1.1 || ratio < 0.9) continue;
		cc = cv::Point2f(rect.center.x, rect.center.y);
		printf("ratio: %f\n", ratio);
		printf("x1: %f,y1: %f \n", cc.x, cc.y);
		//绘图
		cv::ellipse(src, rect, cv::Scalar(0, 255, 0), 5, 8);
		cv::circle(src, cc, 2, cv::Scalar(0, 255, 0), 3, 8, 0);
		pix.push_back(cc);//将点坐标存入pix变量中
	}
	//9点排序并显示//
	vector < cv::Point2f> pixcopy = pix;
	int count = 0;
	for (int i = 0; i < pix.size(); i++)
	{
		pix[i] = pixcopy[pix.size() - i - 1];
		stringstream ss;
		string str;
		ss << count;
		ss >> str;
		cv::putText(src, str, pix[i], cv::FONT_HERSHEY_COMPLEX, 1.8, (0, 255, 0), 2);
		count++;
	}

	//世界坐标//
	float x = 0;
	float y = 0;
	float step = 10;
	world.push_back(cv::Point2f(x - step, y + step));//00
	world.push_back(cv::Point2f(x, y + step));//01
	world.push_back(cv::Point2f(x + step, y + step));//02
	world.push_back(cv::Point2f(x - step, y));//03
	world.push_back(cv::Point2f(x, y));//04
	world.push_back(cv::Point2f(x + step, y));//05
	world.push_back(cv::Point2f(x - step, y - step));//06
	world.push_back(cv::Point2f(x, y - step));//07
	world.push_back(cv::Point2f(x + step, y - step));//08
	//标定
	if (!calibration(world, pix, calib))
	{
		printf("calib error!!!!!!!!!");
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"标定失败");
		msgBox.exec();
		return;
	}
	refreshImage(src);
	currentImage = src;
	//写入Xml文件
	writeCalib(world, pix, calib);
	refreshCalibBox();
	/*
	cv::namedWindow("image", cv::WINDOW_GUI_NORMAL);
	imshow("image", src);
	refreshImage(src);
	cv::waitKey(0);
	*/
	//////////////////////
	/*标定测试结束*/
}

void SRDemo::on_useCalib(bool flag)
{
	if (!flag)
		return;
	else
	{
		QString name = ui.comboBox_calib->currentText();
		QString path = "../calibration//" + name;
		readCalib(path,calib);
		qDebug() << "[" << QString::number(calib.A) << "," << QString::number(calib.B) << "," << QString::number(calib.C) << "," << QString::number(calib.D) << "," << QString::number(calib.E) << "," << QString::number(calib.F) << "]";
	}
}

void SRDemo::on_calibChanged(QString name)
{
	if (!ui.checkBox_useCalib->isChecked())
		return;
	QString path = "../calibration//" + name;
	readCalib(path, calib);
	qDebug() << "[" << QString::number(calib.A) << "," << QString::number(calib.B) << "," << QString::number(calib.C) << "," << QString::number(calib.D) << "," << QString::number(calib.E) << "," << QString::number(calib.F) << "]";
}

void SRDemo::on_rotation(int value)
{
	if (sourceImage.empty())
		return;
	debugImage = sourceImage;
	Mat dst = getRotationImage(debugImage, value);
	refreshImage(dst);
	currentImage = dst;
}

void SRDemo::on_deBugImage()
{
	if (!debugFlag)
		return;
	if (currentImage.empty())
		return;
	debugImage = currentImage.clone();

	switch (ui.stackedWidget->currentIndex())
	{

	case 0: break;
	case 1://通道提取
	{

		qDebug() << u8"通道提取";
		extraction(debugImage,debugImage);
		refreshImage(debugImage);
		break;
	}
	case 2://二值化
	{
		qDebug() << u8"二值化";
		if (currentImage.type() != CV_8UC1)
			return;
		threshold(debugImage, debugImage);
		refreshImage(debugImage);
		break;
			
	}
	case 3://自定义滤波
	{
		qDebug() << "自定义滤波";
		kernel(debugImage, debugImage);
		refreshImage(debugImage);
		break;
	}
	case 4://腐蚀
	{
		qDebug() << "腐蚀";
		erod(debugImage, debugImage);
		refreshImage(debugImage);
		break;
	}
	case 5://膨胀
	{
		qDebug() << "膨胀";
		dilat(debugImage, debugImage);
		refreshImage(debugImage);
		break;
	}
	case 6://开运算
	{
		qDebug() << "开运算";
		openFilter(debugImage, debugImage);
		refreshImage(debugImage);
		break;
	}
	case 7://闭运算
	{
		qDebug() << "闭运算";
		closeFilter(debugImage, debugImage);
		refreshImage(debugImage);
		break;
	}
	case 8://高斯滤波
	{
		qDebug() << "高斯滤波";
		gaussianFilter(debugImage, debugImage);
		refreshImage(debugImage);
		break;
	}
	case 9://中值滤波
	{
		qDebug() << "中值滤波";
		medianFilter(debugImage, debugImage);
		refreshImage(debugImage);
		break;
	}
	case 10://均值滤波
	{
		qDebug() << "均值滤波";
		averageFilter(debugImage, debugImage);
		refreshImage(debugImage);
		break;
	}
	case 11://傅里叶变换
	{
		fourierTransformation(debugImage, debugImage);
		refreshImage(debugImage);
		break;
	}
	case 12://直方图均衡化
	{
		histogramEqualization(debugImage, debugImage);
		refreshImage(debugImage);
		break;
	}
	case 13://图像操作
	{
		if (ui.operation_comboBox_image->count() == 0)
			return;
		imageOperation(debugImage, debugImage);
		refreshImage(debugImage);
		break;
	}
	case 15://找点
	{
		findPoint();
		break;
	}
	case 16://找直线‘
	{
		findLine();
		break;
	}
	case 17://找圆
	{
		findCircle();
		break;
	}
	case 18://Blob分析
	{
		findBlob();
		break;
	}
	case 19://轮廓提取
	{
		findContours();
		break;
	}
	case 20://模板匹配
	{
		templateMatch();
		break;
	}
	default:break;
	}
}

void SRDemo::on_extraction()
{
	if (currentImage.type() != CV_8UC3)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为三通道图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	debugFlag = true;
	ui.stackedWidget->setCurrentWidget(ui.page_extraction);
}

void SRDemo::on_threshold()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_Threshold);
	debugFlag = true;
}

void SRDemo::on_do()
{
	currentImage = debugImage;
	switch (ui.stackedWidget->currentIndex())
	{
	case 1:	funcList.append(&SRDemo::extraction); funcValueList.append(u8"通道提取"); break;
	case 2: funcList.append(&SRDemo::threshold); funcValueList.append(u8"二值化"); break;
	case 3: funcList.append(&SRDemo::kernel); funcValueList.append(u8"自定义滤波"); break;
	case 4: funcList.append(&SRDemo::erod); funcValueList.append(u8"腐蚀"); break;
	case 5: funcList.append(&SRDemo::dilat); funcValueList.append(u8"膨胀"); break;
	case 6: funcList.append(&SRDemo::openFilter); funcValueList.append(u8"开运算"); break;
	case 7: funcList.append(&SRDemo::closeFilter); funcValueList.append(u8"闭运算"); break;
	case 8: funcList.append(&SRDemo::gaussianFilter); funcValueList.append(u8"高斯滤波"); break;
	case 9: funcList.append(&SRDemo::medianFilter); funcValueList.append(u8"中值滤波"); break;
	case 10:funcList.append(&SRDemo::averageFilter); funcValueList.append(u8"均值滤波"); break;
	case 11:funcList.append(&SRDemo::fourierTransformation); funcValueList.append(u8"傅里叶变换"); break;
	case 12:funcList.append(&SRDemo::histogramEqualization); funcValueList.append(u8"直方图均衡"); break;
	case 13:funcList.append(&SRDemo::imageOperation); funcValueList.append(u8"图像操作"); break;
	case 14: //生成ROI
	case 15: //找点
	case 16: //直线
	case 17: //圆
	case 18: //blob
	case 19: //轮廓
	{
		disconnect(ui.label_image, &SRLabel::sendLeftStartPos, this, &SRDemo::on_getLeftStartPos);
		disconnect(ui.label_image, &SRLabel::sendLeftEndPos, this, &SRDemo::on_getLeftEndPos);
		disconnect(ui.label_image, &SRLabel::sendLeftMovePos, this, &SRDemo::on_getLeftMovePos);
	}
	default:
		break;
	}
	imageList.append(currentImage);
	funcValueModel->setStringList(funcValueList);
	ui.func_list->setModel(funcValueModel);
	ui.stackedWidget->setCurrentWidget(ui.page_non);

}

void SRDemo::on_cancel()
{
	switch (ui.stackedWidget->currentIndex())
	{
	case 14: //生成ROI
	case 15://找点
	{
		disconnect(ui.label_image, &SRLabel::sendLeftStartPos, this, &SRDemo::on_getLeftStartPos);
		disconnect(ui.label_image, &SRLabel::sendLeftEndPos, this, &SRDemo::on_getLeftEndPos);
		disconnect(ui.label_image, &SRLabel::sendLeftMovePos, this, &SRDemo::on_getLeftMovePos);
		break;
	}
	default:
		break;
	}
	if (!currentImage.empty())
		refreshImage(currentImage);
	ui.stackedWidget->setCurrentWidget(ui.page_non);
}

void SRDemo::on_kernel()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_kernel);
	on_kernelSet(ui.kernel_spinBox->value());
	debugFlag = true;
}

void SRDemo::clearLayout(QLayout *layout)
{
	QLayoutItem *item;
	while ((item = layout->takeAt(0)) != 0) {
		//删除widget
		if (item->widget()) {
			delete item->widget();
			//item->widget()->deleteLater();
		}
		//删除子布局
		QLayout *childLayout = item->layout();
		if (childLayout) {
			clearLayout(childLayout);
		}
		delete item;
	}
}

void SRDemo::on_erod()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_erod);
	debugFlag = true;
}

void SRDemo::on_dilat()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_dilat);
	debugFlag = true;
}

void SRDemo::on_openFilter()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_open);
	debugFlag = true;
}

void SRDemo::on_closeFilter()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_close);
	debugFlag = true;
}

void SRDemo::on_gaussianFilter()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_guassianFilter);
	debugFlag = true;
}

void SRDemo::on_medianFilter()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_medianFilter);
	debugFlag = true;
}

void SRDemo::on_averageFilter()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_averageFilter);
	debugFlag = true;
}

void SRDemo::on_fourierTransformation()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_fouriergramEqualization);
	debugFlag = true;
}

void SRDemo::on_histogramEqualization()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_histogramTransformation);
	debugFlag = true;
}
void SRDemo::on_imageOperation()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_ImageOperation);
	ui.operation_comboBox_image->clear();
	ui.operation_comboBox_image->addItems(funcValueList);
	debugFlag = true;
}
void SRDemo::on_cameraChanged(int index)
{
	if (ui.camera_radioButton->isChecked())
	{
		camera.destroyCamera();
		camera.useCamera(index);
	}

}
void SRDemo::on_cameraOnline()
{
	if (ui.camera_btn_ontime->isEnabled())
	{
		ui.camera_btn_online->setText(u8"关闭采集");
		ui.camera_btn_ontime->setEnabled(false);
		ui.groupBox->setEnabled(false);
		ui.groupBox_2->setEnabled(false);
		ui.func_list->setEnabled(false);
		camera.online();
	}
	else
	{
		ui.camera_btn_online->setText(u8"实时采集");
		ui.camera_btn_ontime->setEnabled(true);
		ui.groupBox->setEnabled(true);
		ui.groupBox_2->setEnabled(true);
		ui.func_list->setEnabled(true);
		camera.onlineFlag = false;
	}

}
void SRDemo::on_cameraGetImage()
{
	camera.getImage();
}
void SRDemo::on_showCamera(Mat image)
{
	for (int i = 0; i < funcList.length(); i++)
	{
		(this->*funcList[i])(image, image);
		imageList[i] = image.clone();
	}
	currentImage = image;
	refreshImage(image);
}
void SRDemo::on_scaleUp()
{
	image_scale_offset += 0.1;
	refreshImage(currentImage);
}
void SRDemo::on_scaleDowm()
{
	if ((image_scale_offset + image_scale) > 0.2)
	{
		image_scale_offset -= 0.1;
	}
	refreshImage(currentImage);
}
void SRDemo::on_scaleReal()
{
	image_scale_offset = 1 - image_scale;
	refreshImage(currentImage);
}
void SRDemo::on_scaleAuto()
{
	image_scale_offset = 0;
	refreshImage(currentImage);
}
void SRDemo::on_showFuncImage(QModelIndex index)
{
	qDebug() << index.row();
	refreshImage(imageList[index.row()]);
}

//获取图像上的坐标
void SRDemo::on_getImagePos(QPoint pos)
{
	QPoint imagePos;
	refreshImagePos(pos, imagePos);
	statusBarMessage.setText(QString("(%1 x %2) scale :%3 (%4,%5)").arg(image_w).arg(image_h).arg(image_scale + image_scale_offset).arg(imagePos.x()).arg(imagePos.y()));
	ui.statusBar->addWidget(&statusBarMessage);

}
/*获取左键按下的第一个坐标*/
void SRDemo::on_getLeftStartPos(QPoint pos)
{
	refreshImagePos(pos, roiStart);
}
void SRDemo::on_getLeftMovePos(QPoint pos)
{
	debugImage = currentImage.clone();
	refreshImagePos(pos, roiEnd);
	if (ui.stackedWidget->currentWidget() == ui.page_GetROI)
	{
		drawROI();
	}
	if (ui.stackedWidget->currentWidget() == ui.page_findPoint)
	{
		drawROI(0);
	}
	if (ui.stackedWidget->currentWidget() == ui.page_findLine)
	{
		drawROI(1);
	}
	if (ui.stackedWidget->currentWidget() == ui.page_findCircle)
	{
		cv::Point p1(roiStart.x(), roiStart.y());
		cv::Point p2(roiEnd.x(), roiEnd.y());
		Mat image = debugImage.clone();
		if (currentImage.type() == CV_8UC1)
		{
			cvtColor(debugImage, image, COLOR_GRAY2BGR);
		}
		double radius_in = ui.findCircle_radius_in->value();
		double radius = sqrt(powf((p1.x - p2.x), 2) + powf((p1.y - p2.y), 2));
		ui.findCircle_radius_out->setValue(round(radius));
		if (radius_in >= radius)
			return;
		cv::circle(image, p1, radius, Scalar(255, 0, 0), 1, 8);
		cv::circle(image, p1, radius_in, Scalar(255, 0, 0), 1, 8);
		cv::circle(image, p1, 2, Scalar(0, 255, 0), -1, 8);
		ui.roi_circle_centerX->setValue(p1.x);
		ui.roi_circle_centerY->setValue(p1.y);
		ui.roi_circle_radius_out->setValue(radius);
		refreshImage(image);
	}
	if (ui.stackedWidget->currentWidget() == ui.page_findContours)
	{
		drawROI(1);
	}
}
/*获取左键按下的左后一个坐标*/
void SRDemo::on_getLeftEndPos(QPoint pos)
{
	debugImage = currentImage.clone();
	if (ui.stackedWidget->currentWidget() == ui.page_findPoint)
	{
		findPoint();
	}
	if (ui.stackedWidget->currentWidget() == ui.page_findLine)
	{
		findLine();
	}
	if (ui.stackedWidget->currentWidget() == ui.page_findCircle)
	{
		findCircle();
	}
	if (ui.stackedWidget->currentWidget() == ui.page_findContours)
	{
		findContours();
	}

}
//获取ROI
void SRDemo::on_getROI()
{
	if (currentImage.empty())
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"图像为空");
		msgBox.exec();
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_GetROI);
	ui.stackedWidget_roi->setCurrentIndex(ui.roi_type->currentIndex());
	connect(ui.label_image, &SRLabel::sendLeftStartPos, this, &SRDemo::on_getLeftStartPos);
	connect(ui.label_image, &SRLabel::sendLeftEndPos, this, &SRDemo::on_getLeftEndPos);
	connect(ui.label_image, &SRLabel::sendLeftMovePos, this, &SRDemo::on_getLeftMovePos);
}
void SRDemo::on_roiPageChange(int value)
{
	ui.stackedWidget_roi->setCurrentIndex(value);
}
void SRDemo::on_findPoint()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_findPoint);
	connect(ui.label_image, &SRLabel::sendLeftStartPos, this, &SRDemo::on_getLeftStartPos);
	connect(ui.label_image, &SRLabel::sendLeftEndPos, this, &SRDemo::on_getLeftEndPos);
	connect(ui.label_image, &SRLabel::sendLeftMovePos, this, &SRDemo::on_getLeftMovePos);

}
void SRDemo::on_findLine()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_findLine);
	connect(ui.label_image, &SRLabel::sendLeftStartPos, this, &SRDemo::on_getLeftStartPos);
	connect(ui.label_image, &SRLabel::sendLeftEndPos, this, &SRDemo::on_getLeftEndPos);
	connect(ui.label_image, &SRLabel::sendLeftMovePos, this, &SRDemo::on_getLeftMovePos);
}
void SRDemo::on_findCircle()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_findCircle);
	connect(ui.label_image, &SRLabel::sendLeftStartPos, this, &SRDemo::on_getLeftStartPos);
	connect(ui.label_image, &SRLabel::sendLeftEndPos, this, &SRDemo::on_getLeftEndPos);
	connect(ui.label_image, &SRLabel::sendLeftMovePos, this, &SRDemo::on_getLeftMovePos);
}
void SRDemo::on_findBlob()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_findBlob);
}
void SRDemo::on_findContours()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_findContours);
	connect(ui.label_image, &SRLabel::sendLeftStartPos, this, &SRDemo::on_getLeftStartPos);
	connect(ui.label_image, &SRLabel::sendLeftEndPos, this, &SRDemo::on_getLeftEndPos);
	connect(ui.label_image, &SRLabel::sendLeftMovePos, this, &SRDemo::on_getLeftMovePos);
}
void SRDemo::on_templateMatch()
{
	if (currentImage.type() != CV_8UC1)
	{
		msgBox.setWindowTitle(tr("error"));
		msgBox.setText(u8"该图像不为灰度图像");
		msgBox.exec();
		debugFlag = false;
		return;
	}
	ui.stackedWidget->setCurrentWidget(ui.page_match);
}
//新建匹配模板
void SRDemo::on_match_new()
{
}
//加载匹配模板
void SRDemo::on_match_load()
{
	imagePath = QFileDialog::getOpenFileName(this, tr("选择模板"), "../image", tr("Images (*.png *.bmp *.jpg)"));
	QFileInfo fileinfo = QFileInfo(imagePath);
	fmatch.templateImg = imread(imagePath.toStdString());
	cvtColor(fmatch.templateImg, fmatch.templateImg, COLOR_BGR2GRAY);
	ui.match_image->setPixmap(QPixmap::fromImage(MatToQImage(fmatch.templateImg)));
}
//存储匹配模板
void SRDemo::on_match_save()
{
}

//自定义滤波核界面设置
void SRDemo::on_kernelSet(int num)
{
	clearLayout(kernel_gridLayout);
	kernel_gridLayout->update();
	int w = ui.groupBox_4->width()/num;
	int h = ui.groupBox_4->height()/num;
	int value = min(min(50,w), h);
	for (int i = 0;i < num; i++)
	{
		for (int j = 0;j < num; j++)
		{
			QLineEdit* line = new QLineEdit();
			line->setMaximumWidth(value);
			line->setMaximumHeight(value);
			line->setMinimumWidth(value);
			line->setMinimumHeight(value);
			line->setAlignment(Qt::AlignmentFlag::AlignCenter);
			line->setText("0");
			connect(line, &QLineEdit::textChanged, this, &SRDemo::on_deBugImage);
			kernel_gridLayout->addWidget(line,i,j,1,1);
		}
	}
	ui.groupBox_4->setLayout(kernel_gridLayout);

}

cv::Mat QImageToMat(QImage image)
{
	cv::Mat mat;
	//qDebug() << image.format();
	switch (image.format())
	{
	case QImage::Format_ARGB32:
	case QImage::Format_RGB32:
	case QImage::Format_ARGB32_Premultiplied:
		mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
		break;
	case QImage::Format_RGB888:
		mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
		cv::cvtColor(mat, mat, COLOR_BGR2RGB);
		break;
	case QImage::Format_Indexed8:
		mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
		break;
	}
	return mat;
}

QImage MatToQImage(const cv::Mat & mat)
{
	QImage image;
	switch (mat.type())
	{
	case CV_8UC1:// 8-bits unsigned, NO. OF CHANNELS = 1
	{
		image = QImage(mat.cols, mat.rows, QImage::Format_Indexed8);
		// Set the color table (used to translate colour indexes to qRgb values)
		//image.setNumColors(256);
		image.setColorCount(256);
		for (int i = 0; i < 256; i++)
		{
			image.setColor(i, qRgb(i, i, i));
		}
		// Copy input Mat
		uchar *pSrc = mat.data;
		for (int row = 0; row < mat.rows; row++)
		{
			uchar *pDest = image.scanLine(row);
			memcpy(pDest, pSrc, mat.cols);
			pSrc += mat.step;
		}
		return image;
	}
	case CV_8UC3:// 8-bits unsigned, NO. OF CHANNELS = 3
	{
		// Copy input Mat
		const uchar *pSrc = (const uchar*)mat.data;
		// Create QImage with same dimensions as input Mat
		image = QImage(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
		return image.rgbSwapped();
	}
	case CV_8UC4:
	{
		// Copy input Mat
		const uchar *pSrc = (const uchar*)mat.data;
		// Create QImage with same dimensions as input Mat
		image = QImage(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
		return image.copy();
	}
	default:
	{
		return QImage();
	}
	}
	return image;
}
