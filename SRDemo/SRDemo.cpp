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
	Mat kernel = (Mat_<float>(kernel_size, kernel_size));
	for (int i = 0; i < kernel_size; i++)
	{
		for (int j = 0; j < kernel_size; j++)
		{
			kernel.ptr<float>(i)[j] = ((QLineEdit*)(kernel_gridLayout->itemAtPosition(i, j)->widget()))->text().toFloat();
			qDebug() << kernel.ptr<float>(i)[j];
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
		Mat image;
		getOperationImage(input, output, type, image);
	}

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
	funcList.clear();
	funcValueList.clear();
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
	int wMax = ui.label_image->width();//显示的最大宽度
	int hMax = ui.label_image->height();//显示的最大高度
	int h = image.rows;//图片高度
	int w = image.cols;//图片宽度
	//QPainter painter(ui.label_image);
	if (h <= hMax && w <= wMax)
	{
		image_scale = 1.0;
		//ui.label_image->setPixmap(QPixmap::fromImage(MatToQImage(image)));
		Mat dst;
		cv::resize(image, dst, Size(0, 0), image_scale + image_scale_offset, image_scale + image_scale_offset, INTER_LINEAR);
		ui.label_image->setPixmap(QPixmap::fromImage(MatToQImage(dst)));
	}
	else
	{
		double scalew = double(wMax) / double(w);
		double scaleh = double(hMax) / double(h);
		image_scale = MIN(scalew, scaleh);
		printf("scalew: %f\nscaleh: %f\nscale: %f\n", scalew, scaleh, image_scale);
		Mat dst;
		cv::resize(image, dst, Size(0, 0), image_scale + image_scale_offset, image_scale + image_scale_offset, INTER_LINEAR);
		ui.label_image->setPixmap(QPixmap::fromImage(MatToQImage(dst)));
		//painter.drawImage(QPoint(0, 0), MatToQImage(dst));
	}
	statusBarMessage.setText(QString("(%1 x %2) scale :%3").arg(w).arg(h).arg(image_scale + image_scale_offset));
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
		imageOperation(debugImage, debugImage);
		refreshImage(debugImage);
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
	default:
		break;
	}
	funcValueModel->setStringList(funcValueList);
	ui.func_list->setModel(funcValueModel);
	ui.stackedWidget->setCurrentWidget(ui.page_non);

}

void SRDemo::on_cancel()
{
	if(!currentImage.empty())
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
		camera.online();
	}
	else
	{
		ui.camera_btn_online->setText(u8"实时采集");
		ui.camera_btn_ontime->setEnabled(true);
		ui.groupBox->setEnabled(true);
		ui.groupBox_2->setEnabled(true);
		camera.onlineFlag = false;
	}

}
void SRDemo::on_cameraGetImage()
{
	camera.getImage();
}
void SRDemo::on_showCamera(Mat image)
{
	//extraction(image, image);
	for (int i = 0; i < funcList.length(); i++)
	{
		(this->*funcList[i])(image, image);
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
