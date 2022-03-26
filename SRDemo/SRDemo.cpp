#include "SRDemo.h"

SRDemo::SRDemo(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	refreshCalibBox();
}
void SRDemo::on_inputClicked()
{
	imagePath = QFileDialog::getOpenFileName(this, tr("打开图片"), "../image", tr("Images (*.png *.bmp *.jpg)"));
	QFileInfo fileinfo = QFileInfo(imagePath);
	sourceImage = imread(imagePath.toStdString());
	currentImage = sourceImage;
	refreshImage(sourceImage);
	ui.line_path->setText(fileinfo.fileName());
	ui.horizontalSlider->setValue(0);
}
void SRDemo::on_reset()
{
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
	int wMax = ui.label_image->width();//显示的最大宽度
	int hMax = ui.label_image->height();//显示的最大高度
	int h = image.rows;//图片高度
	int w = image.cols;//图片宽度
	double scale = 1.0;
	if (h <= hMax && w <= wMax)
	{
		ui.label_image->setPixmap(QPixmap::fromImage(MatToQImage(image)));
	}
	else
	{
		double scalew = double(wMax) / double(w);
		double scaleh = double(hMax) / double(h);
		scale = MIN(scalew, scaleh);
		printf("scalew: %f\nscaleh: %f\nscale: %f\n", scalew, scaleh, scale);
		Mat dst;
		cv::resize(image, dst, Size(0, 0), scale, scale, INTER_LINEAR);
		ui.label_image->setPixmap(QPixmap::fromImage(MatToQImage(dst)));
	}
	statusBarMessage.setText(QString("(%1 x %2) scale :%3").arg(w).arg(h).arg(scale));
	ui.statusBar->addWidget(&statusBarMessage);
	
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
	findContours(dst, contours, hireachy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point());

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
	Mat dst = ImageRotation(debugImage, value);
	refreshImage(dst);
	currentImage = dst;
}

void SRDemo::on_extraction()
{
	ui.stackedWidget->setCurrentWidget(ui.page_extraction);
}

void SRDemo::on_deBugImage()
{
	if (currentImage.empty())
		return;
	debugImage = currentImage.clone();
	switch (ui.stackedWidget->currentIndex())
	{
	case 0: break;
	case 1://通道提取
	{
		Mat mv[3];
		if (ui.radioButton_RGB->isChecked())
		{
			qDebug() << "use RGB";
			split(debugImage, mv);
			switch (ui.comboBox_RGB->currentIndex())
			{
			case 0:refreshImage(debugImage); break;//RGB模式
			case 1:refreshImage(mv[2]); break;//R
			case 2:refreshImage(mv[1]); break;//G
			case 3:refreshImage(mv[0]); break;//B
			}
		}
		else if (ui.radioButton_HSL->isChecked())
		{
			qDebug() << "use HSL";
			cvtColor(currentImage, debugImage, COLOR_BGR2HLS);	
			split(debugImage, mv);
			switch (ui.comboBox_HSL->currentIndex())
			{
			case 0:refreshImage(debugImage); break;//HSL模式
			case 1:refreshImage(mv[0]); break;//H色调
			case 2:refreshImage(mv[2]); break;//S饱和度
			case 3:refreshImage(mv[1]); break;//L亮度
			}
		}
		else if (ui.radioButton_HSV->isChecked())
		{
			qDebug() << "use HSV";
			cvtColor(currentImage, debugImage, COLOR_BGR2HSV);
			split(debugImage, mv);
			switch (ui.comboBox_HSV->currentIndex())
			{
			case 0:refreshImage(debugImage); break;//HSL模式
			case 1:refreshImage(mv[0]); break;//H色调
			case 2:refreshImage(mv[1]); break;//S饱和度
			case 3:refreshImage(mv[2]); break;//L明度
			}
		}
		else if (ui.radioButton_HSI->isChecked())
		{
			qDebug() << "use HSI";
			debugImage = cvtColor_RGB2HSI(currentImage);
			split(debugImage, mv);
			switch (ui.comboBox_HSI->currentIndex())
			{
			case 0:refreshImage(debugImage); break;
			case 1:refreshImage(mv[0]); break;//H色调
			case 2:refreshImage(mv[1]); break;//S饱和度
			case 3:refreshImage(mv[2]); break;//L明度
			}
		}
	}
	default:break;
	}
}

void SRDemo::on_do()
{
	currentImage = debugImage;
	ui.stackedWidget->setCurrentWidget(ui.page_non);
}

void SRDemo::on_cancel()
{
	if(!currentImage.empty())
		refreshImage(currentImage);
	ui.stackedWidget->setCurrentWidget(ui.page_non);
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
