#include "SRVision.h"

bool SRVision::calibration(vector<Point2f> world, vector<Point2f> pix, calibResult &calib)
{
	if (world.size() != 9 || pix.size() != 9)
	{
		return false;
	}
	else
	{
		printf("pix is: \n");
		for (int i = 0; i < pix.size(); i += 3)
		{
			printf("(%f,%f),(%f,%f),(%f,%f) \n", pix[i].x, pix[i].y, pix[i + 1].x, pix[i + 1].y, pix[i + 2].x, pix[i + 2].y);
		}
		printf("world is: \n");
		for (int i = 0; i < world.size(); i += 3)
		{
			printf("(%f,%f),(%f,%f),(%f,%f) \n", world[i].x, world[i].y, world[i + 1].x, world[i + 1].y, world[i + 2].x, world[i + 2].y);
		}
	}
	cv::Mat cal;
	cal = cv::estimateRigidTransform(pix, world, true);//opencv求仿射变换矩阵函数
	calib.A = cal.ptr<double>(0)[0];
	calib.B = cal.ptr<double>(0)[1];
	calib.C = cal.ptr<double>(0)[2];
	calib.D = cal.ptr<double>(1)[0];
	calib.E = cal.ptr<double>(1)[1];
	calib.F = cal.ptr<double>(1)[2];
	printf("calib is: \n");
	printf("%f,%f,%f\n%f,%f,%f\n", calib.A, calib.B, calib.C, calib.D, calib.E, calib.F);
	printf("验证开始: \n");
	double Xmax = 0;
	double Ymax = 0;
	for (int i = 0; i < pix.size(); i++)
	{
		double x = pix[i].x*calib.A + pix[i].y*calib.B + calib.C;
		double y = pix[i].x*calib.D + pix[i].y*calib.E + calib.F;

		double xx = abs(x - world[i].x);
		if (xx > Xmax)
			Xmax = xx;
		double yy = abs(y - world[i].y);
		if (yy > Ymax)
			Ymax = yy;
		printf("world is(%f,%f)\n计算得(%f,%f)\n", world[i].x, world[i].y, x, y);
	}
	printf("x方向误差为%f\ny方向误差为%f\n", Xmax, Ymax);
	return true;
}
/*
	函数作用：
		RGB图像转为HSI图像
	变量：
		input:输入图像
		output：输出图像
*/
Mat SRVision::cvtColor_RGB2HSI(Mat &input)
{
	/*
			RGB  --->  HSI
		   theta = acos{[(r-g)+(r-b)]/2} / sqrt{(r-g)²+(r-b)(g-b)}
				{ 0  b<=g  }
		   H =  {          }
				{360-theta b>g }	 
		   S = 1-{3*min(r,g,b)/(r+g+b)}
		   I = (r+g+b)/3
					 
	*/
	Mat output = Mat(input.rows, input.cols, input.type());
	float R, G, B, H, S, I, theta, sum, minRGB, den , num;
	for (int i = 0; i < input.rows; ++i)
	{
		for (int j = 0; j < input.cols; ++j)
		{
			B = input.at<Vec3b>(i, j)[0];
			G = input.at<Vec3b>(i, j)[1];
			R = input.at<Vec3b>(i, j)[2];
			//归一化
			B = B / 255.0;
			G = G / 255.0;
			R = R / 255.0;

			sum = B + G + R;
			minRGB = min(min(R, G), B);
			num = ((R - G) + (R - B)) / 2;
			den = sqrt(R * R + B * B + G * G - R * B - R * G - B * G);// = sqrt((R - G)*(R - G) + (R - B)*(G - B));
			if (den == 0)
			{
				H = 0;
			}
			else
			{
				theta = acos(num/den);
				if (B <= G)
				{
					H = theta;
				}
				else
				{
					
					H = 2*PI - theta;
				}
			}
			if (sum == 0)
			{
				S = 1;
			}
			else
			{
				S = 1 - (3 * minRGB) / sum;
			}
			
			I = sum / 3.0;
			H = H / (2*PI);//H在[0-360]之间，将其归一化
			//h,s,i分量扩充到255用来显示
			output.at<Vec3b>(i, j)[0] = int(H * 255);
			output.at<Vec3b>(i, j)[1] = int(S * 255);
			output.at<Vec3b>(i, j)[2] = int(I * 255);
		}
	}
	return output;
}
/*
	函数作用：
		旋转图像
	变量：
		image:待旋转图像
		angle：旋转角度
*/
Mat SRVision::getRotationImage(cv::Mat &image, double angle)
{
	cv::Mat src = image.clone();
	cv::Mat dst;
	cv::Size size = src.size();
	cv::Point2f center(src.cols / 2, src.rows / 2);//旋转中心;
	//获取旋转矩阵
	cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);

	//进行仿射变换
	cv::warpAffine(src, dst, rot, size);
	return dst;
}
/*
	函数作用：
		镜像图像
	变量：
		image:待旋转图像
		type：0 x轴镜像  <0 y轴镜像 >0同时翻转
*/
cv::Mat SRVision::getMirrorImage(cv::Mat &image, int type)
{
	cv::Mat dst;
	cv::Mat src = image.clone();
	cv::flip(src, dst, type);//0 x轴镜像  <0 y轴镜像 >0同时翻转
	return dst;
}

SRVision::SRCalcHist::SRCalcHist(Mat image)
{
	Mat debug = image.clone();
	Mat std, avg;
	minMaxLoc(debug, &min, &max);
	meanStdDev(debug, std, avg);
	stddev = std.at<double>(0, 0);
	average = avg.at<double>(0, 0);

	int numbers = 256;
	//定义变量范围，并定义三个矩阵来存储每个直方图
	float range[] = { 0, 256 };
	const float *histRange = { range };
	calcHist(&image, 1, 0, Mat(), gray, 1, &numbers, &histRange);
	/*
	for (int i = 0; i < 256; i++)
	{
		printf("%f\n", gray.at<float>(i));
	}
	*/
	getCalcHistImage(this->gray, this->calcHistImage);

}
/*
	无参构造函数
*/
SRVision::SRCalcHist::SRCalcHist()
{
	this->average = 0.0;
	this->max = 255.0;
	this->min = 0.0;
	this->stddev = 0.0;
	this->calcHistImage = Mat::zeros(Size(1,1), CV_8UC1);
}
/*
	函数作用：
		平滑直方图
	变量：
		input:输入直方图数据
		output：输出直方图数据
		time: 迭代次数
		
*/
void SRVision::SRCalcHist::getSmoothHist(Mat input,Mat &output, int time)
{
	try
	{
		output = input.clone();
		output.at<float>(0) = 0;
		output.at<float>(255) = 0;
		if (time != -1)
		{
			for (int i = 0; i < time; i++)
			{
				for (int j = 1; j < 255; j++)
				{
					output.at<float>(j) = (output.at<float>(j - 1) + output.at<float>(j) + output.at<float>(j + 1))/3;
				}
			}
		}
		else
		{
			while(1)
			{

			}
		}
	}
	catch (cv::Exception e)
	{
		printf("%s\n", e.msg.c_str());
	}
	
}

/*
函数作用：
		获取直方图
	变量：
		input:输入图像
		output：输出直方图图像
*/
void SRVision::SRCalcHist::getCalcHistImage(Mat input, Mat &output)
{
	/*
	* calcHist用来计算图像直方图：
	* 参数详解：
	   const Mat* images：输入图像
	   int nimages：输入图像的个数
	   const int* channels：需要统计直方图的第几通道
	   InputArray mask：掩膜，，计算掩膜内的直方图  ...Mat()
	   OutputArray hist:输出的直方图数组
	   int dims：需要统计直方图通道的个数
	   const int* histSize：指的是直方图分成多少个区间，就是 bin的个数
	   const float** ranges： 统计像素值得区间
	   bool uniform=true::是否对得到的直方图数组进行归一化处理
	   bool accumulate=false：在多个图像时，是否累计计算像素值得个数
	*/
	//计算出每个通道的直方图后绘制直方图，并显示给用户
	Mat hist;
	int width = 256;
	int height = 160;
	Mat histImage(height, width, CV_8UC3, Scalar(255, 255, 255));
	//设置直方图有256个区间,因为图像的灰度值变化为0~255
	int numbers = 256;
	int binStep = cvRound((float)width / (float)numbers);
	/*
	for (int i = 0; i < 256; i++)
	{
		printf("%f\n", input.at<float>(i));
	}
	*/
	normalize(input, hist, 0, height, NORM_MINMAX);
	for (int i = 0; i < numbers; i++)
	{
		try
		{
			line(
				histImage,
				Point(binStep*(i - 1), height - cvRound(hist.at<float>(i - 1))),
				Point(binStep*(i), height - cvRound(hist.at<float>(i))),
				Scalar(255, 0, 0)
			);
		}
		catch (Exception exception)
		{
			printf(exception.err.c_str());
		}
	}
	output = histImage;
}
/*
函数作用：
		获取二值化图像
	变量：
		input:输入图像
		output：输出直方图图像
		low: 低阈值
		hight:高阈值
		type: Threshold_0, //手动二值化 黑色对象
			  Threshold_1, //手动二值化 白色对象
			  Threshold_2, //大津法二值化 黑色对象
			  Threshold_3, //大津法二值化 白色对象
*/
int SRVision::getThresholdImage(Mat input, Mat & output, SRThreshold type, int low, int hight)
{
	switch (type)
	{
	case Threshold_0://手动二值化黑色对象
	{
		/*高于高阈值的设255，低于低阈值的设0，两图运算*/
		Mat dst1, dst2;
		threshold(input, dst1, hight, 255, THRESH_BINARY);
		threshold(input, dst2, low, 255, THRESH_BINARY_INV);
		output = dst1 | dst2;
		return 0;
	}
	case Threshold_1://手动二值化白色对象
	{
		/*高于高阈值的设255，低于低阈值的设0，两图运算*/
		Mat dst1, dst2;
		threshold(input, dst1, hight, 255, THRESH_BINARY_INV);
		threshold(input, dst2, low, 255, THRESH_BINARY);
		output = dst1 & dst2;
		return 0;
	}
	case Threshold_2://大津法二值化 黑色对象
	{
		//threshold(input, output, low, hight, THRESH_BINARY | THRESH_OTSU);
		int treshold = getThresholdOtsu(input, output, Threshold_2);
		return treshold;
	}
	case Threshold_3://大津法二值化 白色对象
	{

		//threshold(input, output, low, hight, THRESH_BINARY_INV | THRESH_OTSU);
		int treshold = getThresholdOtsu(input, output, Threshold_3);
		return treshold;
	}
	default:
		return -1;
	}
}

int SRVision::getThresholdOtsu(Mat input, Mat &output, SRThreshold type)
{
	SRCalcHist hist(input);
	//获取灰度概率直方图
	double histogram[256];
	for (int i = 0; i <= 255; i++) {
		histogram[i] = double(hist.gray.at<float>(i)) / double(input.rows * input.cols);
	}
	double w1 = 0.0, w2 = 1.0; //类概率w1,w2
	double u1 = 0.0, u2 = 0.0; //类均值u1,u2
	double max = 0.0; //最大类间方差
	double sum = 0.0;//总均值
	for (int i = 0; i <= 255; i++) {
		sum += i * histogram[i];
	}
	printf("sum is %lf",sum);
	double sum1 = 0.0, sum2 = sum;
	double threshold1 = 0.0, threshold2 = 0.0; //有一个最大则取一个，两个最大取平均
	for (int t = 1; t <= 255; t++) {
		w1 += histogram[t];
		w2 = 1 - w1;
		if (w1 == 0) { //还没出现点时不会是阈值
			continue;
		}
		else if (w2 == 0) { //后面没点时已获得最佳阈值
			break;
		}
		sum1 += t * histogram[t];
		sum2 = sum - sum1;
		u1 = sum1 / w1;
		u2 = sum2 / w2;
		double v = w1 * w2 * (u1 - u2) * (u1 - u2); //类间方差
		if (v >= max) {
			threshold1 = t; //大于等于，等于时，有两个最大
			if (v > max) {
				threshold2 = t; //大于时，有唯一最大，threshold1、threshold2统一
			}
			max = v; //替换max
		}
	}
	double value = (threshold1 + threshold2) / 2;
	switch (type)
	{
	case Threshold_2:
	{
		threshold(input, output, value, 255, THRESH_BINARY);
		break;
	}
	case Threshold_3:
	{
		threshold(input, output, value, 255, THRESH_BINARY_INV);
		break;
	}
	}
	return round(value);
}

void SRVision::getPreprocessImage(Mat input, Mat & output, int type, int shape, int size,int time)
{
	MorphShapes shapes;
	Size sizes;
	Mat element;
	Mat debugImage = input.clone();
	switch (shape)
	{
	case 0:shapes = MORPH_RECT; break;
	case 1:shapes = MORPH_ELLIPSE; break;
	case 2:shapes = MORPH_CROSS; break;
	default:
		break;
	}
	sizes = Size((size + 1) * 2 + 1, (size + 1) * 2 + 1);
	element = getStructuringElement(shapes, sizes);
	switch (type)
	{
	case 4://腐蚀
	{
		cv::morphologyEx(debugImage, debugImage, cv::MORPH_ERODE, element, Point(-1, -1), time);
	}
	case 5://膨胀
	{
		cv::morphologyEx(debugImage, debugImage, cv::MORPH_DILATE, element, Point(-1, -1), time);
		break;
	}
	case 6://开运算
	{
		cv::morphologyEx(debugImage, debugImage, cv::MORPH_OPEN, element, Point(-1, -1), time);
		break;
	}
	case 7://闭运算
	{
		cv::morphologyEx(debugImage, debugImage, cv::MORPH_CLOSE, element, Point(-1, -1), time);
	}
	}
	output = debugImage;
}
void SRVision::getOperationImage(Mat input, Mat & output, int type, int number)
{
	switch (type)
	{
	case 0:output = input + number; break;//加
	case 1:output = input - number; break;//减
	case 2:output = input * number; break;//乘
	case 3:output = input / number; break;//除
	case 4:output = input & number; break;//与
	case 5:output = input | number; break;//或
	case 6:output = 255-(input & number); break;//与非
	case 7:output = 255-(input | number); break;//或非
	case 8:absdiff(input, number, output); break;//绝对差值
	case 9:output = input / 2 + double(number/2); break;//平均值
	case 10:output = min(input, number); break;//最小值
	case 11:output = max(input, number); break;//最大值
	default:
		break;
	}
}
void SRVision::getOperationImage(Mat input, Mat & output, int type, Mat images)
{
	switch (type)
	{
	case 0:output = input + images; break;//加
	case 1:output = input - images; break;//减
	case 2:output = input * images; break;//乘
	case 3:output = input / images; break;//除
	case 4:output = input & images; break;//与
	case 5:output = input | images; break;//或
	case 6:output = 255 - (input & images); break;//与非
	case 7:output = 255 - (input | images); break;//或非
	case 8:absdiff(input, images, output); break;//绝对差值
	case 9:output = (input / 2) + (images / 2); break;//平均值
	case 10:output = min(input, images); break;//最小值
	case 11:output = max(input, images); break;//最大值
	default:
		break;
	}
}
SRVision::SRCamera::SRCamera()
{
	initCamera();
}
SRVision::SRCamera::~SRCamera()
{

}

void SRVision::SRCamera::initCamera()
{
	int i = 0;
	while (1)
	{
		VideoCapture cap(i++);
		if (cap.isOpened())
		{
			this->cameraList.append(cap);
			cap.release();
		}
		else
		{
			break;
		}
	}
}

void SRVision::SRCamera::useCamera(int index)
{
	capture = cameraList[index]; 
	capture.isOpened();
}

void SRVision::SRCamera::destroyCamera()
{
	capture.release();
}

void SRVision::SRCamera::getImage()
{
	capture >> frame;
	emit sendImage(frame);
}

void SRVision::SRCamera::online()
{
	onlineFlag = true;
	while (onlineFlag)
	{
		capture >> frame;
		emit sendImage(frame);
		waitKey(10);
	}
}
SRVision::SRFindPoint::SRFindPoint()
{
	this->pointGroup = { cv::Point(0,0) };
	this->group = { 0 };
	this->sectional = Mat();
}
/**
* 边缘点检测构造函数
* @param[in] start 起始点
* @param[in] end 终止点
* @param[in] strength 边缘强度
* @param[in] polarity 边缘极性 0黑->白、1白->黑 2所有
* @param[in] type 边缘类型 0第一个点、1最后一个点、2最佳点、3所有点
*/
SRVision::SRFindPoint::SRFindPoint(Mat image, SRroiLine roi, int strength, int polarity)
{
	this->findPoint(image, roi, strength,  polarity);
}
/**
* 边缘点检测实际函数
* @param[in] start 起始点
* @param[in] end 终止点
* @param[in] strength 边缘强度
* @param[in] polarity 边缘极性 0黑->白、1白->黑 2所有
*/
void SRVision::SRFindPoint::findPoint(Mat image, SRroiLine roi, int strength = 30,  int polarity = 0)
{
	if (image.empty())
		return;
	cv::Point start = roi.start;
	cv::Point end = roi.end;
	//清除数组并回收空间
	vector<int>().swap(this->group);
	vector<cv::Point>().swap(this->pointGroup);
	//直线小于10pix直接退出
	if(sqrt(pow((start.x-end.x),2) + pow((start.y-end.y),2)) < 10)
		return;
	if (abs(start.x - end.x) < 1)//直线垂直时
	{
		switch (polarity)
		{
		case 0://黑->白
		{
			if (start.y < end.y)
			{
				for (int i = start.y+1; i < end.y; i++)
				{
					int value = (image.ptr<uchar>(i)[start.x] - image.ptr<uchar>(i-1)[start.x]);
					this->group.push_back(value);
					if (value > strength)
						this->pointGroup.push_back(cv::Point(start.x, i));
				}
			}
			else
			{
				for (int i = start.y - 1; i > end.y; i--)
				{
					int value = (image.ptr<uchar>(i)[start.x] - image.ptr<uchar>(i + 1)[start.x]);
					this->group.push_back(value);
					if (value > strength)
						this->pointGroup.push_back(cv::Point(start.x, i));
				}
			}
			break;
		}
		case 1://白->黑
		{
			if (start.y < end.y)
			{
				for (int i = start.y + 1; i < end.y; i++)
				{
					int value = -(image.ptr<uchar>(i)[start.x] - image.ptr<uchar>(i - 1)[start.x]);
					this->group.push_back(value);
					if (value > strength)
						this->pointGroup.push_back(cv::Point(start.x, i));
				}
			}
			else
			{
				for (int i = start.y - 1; i > end.y; i--)
				{
					int value = -(image.ptr<uchar>(i)[start.x] - image.ptr<uchar>(i + 1)[start.x]);
					this->group.push_back(value);
					if (value > strength)
						this->pointGroup.push_back(cv::Point(start.x, i));
				}
			}
			break;
		}
		case 2://所有极性
		{
			if (start.y < end.y)
			{
				for (int i = start.y + 1; i < end.y; i++)
				{
					int value = abs(image.ptr<uchar>(i)[start.x] - image.ptr<uchar>(i - 1)[start.x]);
					this->group.push_back(value);
					if (value > strength)
						this->pointGroup.push_back(cv::Point(start.x, i));
				}
			}
			else
			{
				for (int i = start.y - 1; i > end.y; i--)
				{
					int value = abs(image.ptr<uchar>(i)[start.x] - image.ptr<uchar>(i + 1)[start.x]);
					this->group.push_back(value);
					if (value > strength)
						this->pointGroup.push_back(cv::Point(start.x, i));
				}
			}
			break;
		}
		default:
			break;
		}
	}
	else
	{
		double k, b;
		k = (end.y - start.y) * 1.0 / (end.x - start.x); 
		b = end.y - k * end.x;
		switch (polarity)
		{
		case 0://黑->白
		{
			if (start.x < end.x)
			{
				for (int i = start.x + 1; i < end.x; i++)
				{
					int y1 = round(k * (i)+b);
					int y2 = round(k * (i + 1) + b);
					int value = -(image.ptr<uchar>(y1)[i] - image.ptr<uchar>(y2)[i+1]);
					this->group.push_back(value);
					if (value > strength)
						this->pointGroup.push_back(cv::Point(i, y1));
				}
			}
			else
			{
				for (int i = start.x - 1; i > end.x; i--)
				{
					int y1 = round(k * (i)+b);
					int y2 = round(k * (i - 1) + b);
					int value = -(image.ptr<uchar>(y1)[i] - image.ptr<uchar>(y2)[i - 1]);
					this->group.push_back(value);
					if (value > strength)
						this->pointGroup.push_back(cv::Point(i, y1));
				}
			}
			break;
			
		}
		case 1://白->黑
		{
			if (start.x < end.x)
			{
				for (int i = start.x + 1; i < end.x; i++)
				{
					int y1 = round(k * (i)+b);
					int y2 = round(k * (i + 1) + b);
					int value = (image.ptr<uchar>(y1)[i] - image.ptr<uchar>(y2)[i + 1]);
					this->group.push_back(value);
					if (value > strength)
						this->pointGroup.push_back(cv::Point(i, y1));
				}
			}
			else
			{
				for (int i = start.x - 1; i > end.x; i--)
				{
					int y1 = round(k * (i)+b);
					int y2 = round(k * (i - 1) + b);
					int value = (image.ptr<uchar>(y1)[i] - image.ptr<uchar>(y2)[i - 1]);
					this->group.push_back(value);
					if (value > strength)
						this->pointGroup.push_back(cv::Point(i, y1));
				}
			}
			break;
		}
		case 2://所有极性
		{
			if (start.x < end.x)
			{
				for (int i = start.x + 1; i < end.x; i++)
				{
					int y1 = round(k * (i)+b);
					int y2 = round(k * (i + 1) + b);
					int value = abs(image.ptr<uchar>(y1)[i] - image.ptr<uchar>(y2)[i + 1]);
					this->group.push_back(value);
					if (value > strength)
						this->pointGroup.push_back(cv::Point(i, y1));
				}
			}
			else
			{
				for (int i = start.x - 1; i > end.x; i--)
				{
					int y1 = round(k * (i)+b);
					int y2 = round(k * (i - 1) + b);
					int value = abs(image.ptr<uchar>(y1)[i] - image.ptr<uchar>(y2)[i - 1]);
					this->group.push_back(value);
					if (value > strength)
						this->pointGroup.push_back(cv::Point(i, y1));
				}
			}
			break;
		}
		default:
			break;
		}
	}


	//计算出每个通道的直方图后绘制直方图，并显示给用户
	Mat hist;
	int width = 256;
	int height = 160;
	Mat Image(height, width, CV_8UC3, Scalar(255, 255, 255));
	//设置直方图有256个区间,因为图像的灰度值变化为0~255
	size_t numbers = this->group.size();
	int binStep = cvRound((float)width / (float)numbers);
	/*
	for (int i = 0; i < 256; i++)
	{
		printf("%f\n", input.at<float>(i));
	}
	*/
	//normalize(input, hist, 0, height, NORM_MINMAX);
	vector<double> normalizeGroup;
	double maxValue = 0.0;
	vector<int>::iterator max = max_element(this->group.begin(), this->group.end());
	vector<int>::iterator min = min_element(this->group.begin(), this->group.end());
	double distance = double(max - min);
	for (size_t i = 0; i < this->group.size(); i++)
	{
		normalizeGroup.push_back(double((abs(this->group[i]) / distance) * height));
	}

	for (size_t i = 1; i < numbers; i++)
	{
		try
		{
			line(
				Image,
				Point(binStep*(i - 1), height - normalizeGroup[i-1]),
				Point(binStep*(i), height - normalizeGroup[i]),
				Scalar(255, 0, 0)
			);
		}
		catch (Exception exception)
		{
			printf(exception.err.c_str());
		}
	}
	line(
		Image,
		Point(0, height - round((strength/ distance) * height)),
		Point(256, height - round((strength / distance) * height)),
		Scalar(0, 0, 0)
	);
	this->sectional = Image;
}

SRVision::SRroiLine::SRroiLine()
{
	type = 0;
}

SRVision::SRroiRect::SRroiRect()
{
	type = 1;
}

SRVision::SRroiCircle::SRroiCircle()
{
	type = 2;
}

SRVision::SRFindLine::SRFindLine()
{
	this->angle = 0.0;
	this->effectivePoints = { cv::Point(0, 0) };
	this->edgePoints = { cv::Point(0, 0) };
	this->linePoint = { cv::Point(0, 0) };
}
/*
* 找直线
* @param[in] image 输入图像
* @param[in] roi 矩形ROI
* @param[in] strength 边缘强度
* @param[in] polarity 边缘极性 0黑->白、1白->黑 2所有
* @param[in] type 边缘类型 0第一条直线 1最后一条直线 2最佳直线
* @param[in] directionint 搜索方向 0从上到下 1从下到上 2从左到右 3从右到左
* @param[in] distance 搜索间隔
*/
void SRVision::SRFindLine::findLine(Mat image, SRroiRect roi, int strength, int polarity, int type, int directionint, int distance)
{
	if (image.empty())
		return;
	//清除数组并回收空间
	this->effectivePoints.clear();
	this->edgePoints.clear();
	this->linePoint.clear();

	cv::Point LeftTop = cv::Point(roi.center.x - round(roi.height / 2), roi.center.y - round(roi.width / 2));
	cv::Point RightDown = cv::Point(roi.center.x + round(roi.height / 2), roi.center.y + round(roi.width / 2));
	switch (directionint)
	{
	case 0://从上到下
	{
		for (int i = LeftTop.x; i < RightDown.x; i += distance)
		{
			SRroiLine lineroi;
			lineroi.start = cv::Point(i, LeftTop.y);
			lineroi.end = cv::Point(i, RightDown.y);
			SRFindPoint fpoint;
			fpoint.findPoint(image, lineroi, strength, polarity);
			if (fpoint.pointGroup.empty())
				continue;
			switch (type)
			{
			case 0://第一条直线
			{
				this->edgePoints.push_back(fpoint.pointGroup.front());
				break;
			}
			case 1://最后一条直线
			{
				this->edgePoints.push_back(fpoint.pointGroup.back());
				break;
			}
			case 2://最佳直线
			{
				this->edgePoints.push_back(fpoint.pointGroup.front());
				break;
			}
			default:
				break;
			}
		}
		break;
	}
	case 1://从下到上
	{
		for (int i = LeftTop.x; i < RightDown.x; i += distance)
		{
			SRroiLine lineroi;
			lineroi.end = cv::Point(i, LeftTop.y);
			lineroi.start = cv::Point(i, RightDown.y);
			SRFindPoint fpoint;
			fpoint.findPoint(image, lineroi, strength, polarity);
			if (fpoint.pointGroup.empty())
				continue;
			switch (type)
			{
			case 0://第一条直线
			{
				this->edgePoints.push_back(fpoint.pointGroup.front());
				break;
			}
			case 1://最后一条直线
			{
				this->edgePoints.push_back(fpoint.pointGroup.back());
				break;
			}
			case 2://最佳直线
			{
				this->edgePoints.push_back(fpoint.pointGroup.front());
				break;
			}
			default:
				break;
			}
		}
		break;
	}
	case 2://从左到右
	{
		for (int i = LeftTop.y; i < RightDown.y; i += distance)
		{
			SRroiLine lineroi;
			lineroi.start = cv::Point(LeftTop.x, i);
			lineroi.end = cv::Point(RightDown.x, i);
			SRFindPoint fpoint;
			fpoint.findPoint(image, lineroi, strength, polarity);
			if (fpoint.pointGroup.empty())
				continue;
			switch (type)
			{
			case 0://第一条直线
			{
				this->edgePoints.push_back(fpoint.pointGroup.front());
				break;
			}
			case 1://最后一条直线
			{
				this->edgePoints.push_back(fpoint.pointGroup.back());
				break;
			}
			case 2://最佳直线
			{
				this->edgePoints.push_back(fpoint.pointGroup.front());
				break;
			}
			default:
				break;
			}
		}
		break;
	}
	case 3://从右到左
	{
		for (int i = LeftTop.y; i < RightDown.y; i += distance)
		{
			SRroiLine lineroi;
			lineroi.end = cv::Point(LeftTop.x, i);
			lineroi.start = cv::Point(RightDown.x, i);
			SRFindPoint fpoint;
			fpoint.findPoint(image, lineroi, strength, polarity);
			if (fpoint.pointGroup.empty())
				continue;
			switch (type)
			{
			case 0://第一条直线
			{
				this->edgePoints.push_back(fpoint.pointGroup.front());
				break;
			}
			case 1://最后一条直线
			{
				this->edgePoints.push_back(fpoint.pointGroup.back());
				break;
			}
			case 2://最佳直线
			{
				this->edgePoints.push_back(fpoint.pointGroup.front());
				break;
			}
			default:
				break;
			}
		}
		break;
	}
	default:
		break;
	}
	if (this->edgePoints.empty())
		return;
	//最小二乘法拟合直线
	//存在问题：容易受杂点干扰，准确性低
	//优化方向：ransac(随机采样一致算法)，

	/*RANSAC直线拟合*/
	/*
	算法基本思想和流程
	RANSAC是通过反复选择数据集去估计出模型，一直迭代到估计出认为比较好的模型。
	具体的实现步骤可以分为以下几步：
	1.选择出可以估计出模型的最小数据集；(对于直线拟合来说就是两个点，对于计算Homography矩阵就是4个点)
	2.使用这个数据集来计算出数据模型；
	3.将所有数据带入这个模型，计算出“内点”的数目；(累加在一定误差范围内的适合当前迭代推出模型的数据)
	4.比较当前模型和之前推出的最好的模型的“内点“的数量，记录最大“内点”数的模型参数和“内点”数；
	5.重复1-4步，直到迭代结束或者当前模型已经足够好了(“内点数目大于一定数量”)。
	迭代次数k = log(1-P)/log(1-t^n)
	P:期望找到合适直线概率
	t:内点占比
	n:模型需要点的个数
	*/

	//迭代开始(当符合条件的点大于80%退出迭代||迭代次数小于0时退出，每次迭代优化迭代值)
	//点到拟合直线距离小于2pix视为有效点
	int iters = round(log(1-0.99)/log(1-pow(0.7,2)));//迭代次数
	int size = round(this->edgePoints.size() * 0.7);
	int maxsize = 0;
	vector<cv::Point> pgroup;
	while (iters)//迭代直至符合条件
	{
		pgroup.clear();//清除内容并回收空间
		//1.随机获取两个点
		int index1 = round(rand()%this->edgePoints.size());
		int index2 = round(rand()%this->edgePoints.size());
		while (abs(index2 - index1) < 2)//排除随机点相邻的情况
		{
			index2 = round(rand()%this->edgePoints.size());
		}
		cv::Point p1 = this->edgePoints.at(index1);
		cv::Point p2 = this->edgePoints.at(index2);
		//2.通过两点确定直线一般式 Ax+By+C = 0
		double A, B, C;
		A = (p2.y - p1.y);
		B = (p1.x - p2.x);
		C = p2.x * p1.y - p1.x * p2.y;
		//3.开始计算内点数目
		for (int i = 0; i < this->edgePoints.size(); i++)
		{
			int dis = abs(A * (this->edgePoints.at(i).x) + B * (this->edgePoints.at(i).y) + C) / sqrt(pow(A, 2) + pow(B, 2));
			if (dis <= 2)
			{
				pgroup.push_back(this->edgePoints.at(i));
			}
		}
		if (pgroup.size() >= size)
		{
			this->effectivePoints.assign(pgroup.begin(),pgroup.end());//清空并复制
			break;
		}
		else if (pgroup.size() > maxsize)
		{
			this->effectivePoints.assign(pgroup.begin(), pgroup.end());//清空并复制
			maxsize = pgroup.size();
		}
		iters--;
	}

	//拟合直线
	Vec4f line;
	cv::fitLine(this->effectivePoints, line, cv::DIST_HUBER, 0, 0.01, 0.01);

	//获取点斜式的点和斜率 y-y₁=k（x-x₁）
	cv::Point point0, lpoint, rpoint;
	point0.x = line[2];
	point0.y = line[3];
	double k = line[1] / line[0];

	//计算直线与ROI的交点
	lpoint.x = LeftTop.x;
	rpoint.x = RightDown.x;
	lpoint.y = k * (LeftTop.x - point0.x) + point0.y;
	rpoint.y = k * (RightDown.x - point0.x) + point0.y;
	if (lpoint.y < LeftTop.y)
	{
		lpoint.y = LeftTop.y;
		lpoint.x = (lpoint.y - point0.y) / k + point0.x;
	}
	else if (lpoint.y > RightDown.y)
	{
		lpoint.y = RightDown.y;
		lpoint.x = (lpoint.y - point0.y) / k + point0.x;
	}

	if (rpoint.y < LeftTop.y)
	{
		rpoint.y = LeftTop.y;
		rpoint.x = (rpoint.y - point0.y) / k + point0.x;
	}
	else if (rpoint.y > RightDown.y)
	{
		rpoint.y = RightDown.y;
		rpoint.x = (rpoint.y - point0.y) / k + point0.x;
	}

	this->linePoint.push_back(lpoint);
	this->linePoint.push_back(rpoint);
	this->angle = -atan2((rpoint.y - lpoint.y),(rpoint.x - lpoint.x)) * 180 / PI;
}
/*
		* 找圆
		* @param[in] image 输入图像
		* @param[in] roi 圆ROI
		* @param[in] strength 边缘强度
		* @param[in] polarity 边缘极性 0黑->白、1白->黑 2所有
		* @param[in] type 边缘类型 0第一条直线 1最后一条直线 2最佳直线
		* @param[in] distance 搜索间隔
		*/
void SRVision::SRFindCircle::findCircle(Mat image, SRroiCircle roi, int strength, int polarity, int type, int distance)
{
	if (image.empty())
		return;
	//清除数组并回收空间
	this->effectivePoints.clear();
	this->edgePoints.clear();
	cv::Point start;
	cv::Point end;
	//循环找点
	for (int i = 0; i < 360; i += distance)
	{
		SRroiLine lineroi;
		lineroi.start.x = roi.radius_in*cos(PI / 180.0 * i) + roi.center.x;
		lineroi.start.y = roi.radius_in*sin(PI / 180.0 * i) + roi.center.y;

		lineroi.end.x = roi.radius_out*cos(PI / 180.0 * i) + roi.center.x;
		lineroi.end.y = roi.radius_out*sin(PI / 180.0 * i) + roi.center.y;
	
		SRFindPoint fpoint;
		fpoint.findPoint(image, lineroi, strength, polarity);
		if (fpoint.pointGroup.empty())
			continue;
		switch (type)
		{
		case 0://第一条直线
		{
			this->edgePoints.push_back(fpoint.pointGroup.front());
			break;
		}
		case 1://最后一条直线
		{
			this->edgePoints.push_back(fpoint.pointGroup.back());
			break;
		}
		case 2://最佳直线
		{
			this->edgePoints.push_back(fpoint.pointGroup.front());
			break;
		}
		case 3:
		{
			this->edgePoints.push_back(fpoint.pointGroup.front());
			break;
		}
		default:
			break;
		}
	}
	if (this->edgePoints.empty())
		return;

	/*RANSAC圆拟合*/
	/*
	算法基本思想和流程
	RANSAC是通过反复选择数据集去估计出模型，一直迭代到估计出认为比较好的模型。
	具体的实现步骤可以分为以下几步：
	1.选择出可以估计出模型的最小数据集；(对于圆拟合来说就是3个点，对于计算Homography矩阵就是4个点)
	2.使用这个数据集来计算出数据模型；
	3.将所有数据带入这个模型，计算出“内点”的数目；(累加在一定误差范围内的适合当前迭代推出模型的数据)
	4.比较当前模型和之前推出的最好的模型的“内点“的数量，记录最大“内点”数的模型参数和“内点”数；
	5.重复1-4步，直到迭代结束或者当前模型已经足够好了(“内点数目大于一定数量”)。
	迭代次数k = log(1-P)/log(1-t^n)
	P:期望找到合适直线概率
	t:内点占比
	n:模型需要点的个数
	*/
	int iters = round(log(1 - 0.99) / log(1 - pow(0.7, 3)));//迭代次数
	int size = round(this->edgePoints.size() * 0.7);
	int maxsize = 0;
	vector<cv::Point> pgroup;
	while (iters)//迭代直至符合条件
	{
		pgroup.clear();//清除内容并回收空间
		cv::Point p1, p2, p3;
		//1.随机获取两个点
		int index1 = round(rand() % this->edgePoints.size());
		int index2 = round(rand() % this->edgePoints.size());
		int index3 = round(rand() % this->edgePoints.size());
		p1 = this->edgePoints.at(index1);
		p2 = this->edgePoints.at(index2);
		p3 = this->edgePoints.at(index3);
		while (abs(index2 - index1) < 2 || abs(index2 - index3) < 2)//排除随机点相邻的情况
		{
			index2 = round(rand() % this->edgePoints.size());
			p2 = this->edgePoints.at(index2);
		}
		while (abs(index3 - index1) < 2 || abs(index3 - index2) < 2 || (p2.x - p1.x)*(p3.y - p1.y) - (p3.x - p1.x)*(p2.y - p1.y) == 0)//排除随机点相邻的情况且三点共线
		{
			index3 = round(rand() % this->edgePoints.size());
			p3 = this->edgePoints.at(index3);
		}
		//三点求圆心(一般式：x²+y²+Dx+Ey+F=0) 圆心：(-D/2,-E/2),半径：sqrt((D²+E²-4F)/4)。
		//[x1 y1 1]   [D]    -[x1^2 + y1^2]
		//[x2 y2 1] * [E] =  -[x2^2 + y2^2]   矩阵乘法(A * B = C) => (B = A^-1 * C) //未知矩阵 = 矩阵A的逆*结果矩阵
		//[x3 y3 1]   [F]    -[x3^2 + y3^2]
		double centerX = 0.0, centerY = 0.0, r = 0.0, D = 0.0, E = 0.0, F = 0.0;
		vector<vector<double>> A(3), invA(3), withA(3), C(3);//矩阵A、矩阵A的逆矩阵、伴随矩阵 C结果矩阵
		for (int i = 0; i < 3; i++)//给矩阵列开辟空间
		{
			A[i].resize(3);
			invA[i].resize(3);
			withA[i].resize(3);
			C[i].resize(1);
		}
		A[0][0] = p1.x; A[0][1] = p1.y; A[0][2] = 1;
		A[1][0] = p2.x; A[1][1] = p2.y; A[1][2] = 1;
		A[2][0] = p3.x; A[2][1] = p3.y; A[2][2] = 1;

		C[0][0] = -(pow(p1.x, 2) + pow(p1.y, 2));
		C[1][0] = -(pow(p2.x, 2) + pow(p2.y, 2));
		C[2][0] = -(pow(p3.x, 2) + pow(p3.y, 2));
		if (A[0][0] == 0)//等式不成立时跳过
			continue;
		//求解A的行列式
		//1.求矩阵A化下三角形式行列式
		double determinant;//行列式
		double i, j, k;//下三角行列式主对角线的值
		double A11, A12, A21, A22;//化第一列时矩阵各行的值
		//求行列式
		i = A[0][0];
		A11 = A[1][1] - (A[1][0] / A[0][0] * A[0][1]);
		A12 = A[1][2] - (A[1][0] / A[0][0] * A[0][2]);
		j = A11;
		A21 = A[2][1] - (A[2][0] / A[0][0] * A[0][1]);
		A22 = A[2][2] - (A[2][0] / A[0][0] * A[0][2]);
		if (A11 == 0)//等式不成立时跳过
			continue;
		k = A22 - A21 / A11 * A12;
		determinant = i * j * k;//矩阵行列式的值等于下三角主对角线的乘积
		if (determinant == 0)//行列式不成立时跳过
			continue;
		//求伴随矩阵 withAij = 去除i,j行后的剩余部分求行列式 * -1^(i+j) 也就是A01 A10 A12 A21 需要乘-1 结果再转置就是伴随矩阵
		withA[0][0] = (A[1][1] * A[2][2] - A[1][2] * A[2][1]);
		withA[1][0] = (A[1][0] * A[2][2] - A[1][2] * A[2][0]) * -1;
		withA[2][0] = (A[1][0] * A[2][1] - A[1][1] * A[2][0]);

		withA[0][1] = (A[0][1] * A[2][2] - A[0][2] * A[2][1]) * -1;
		withA[1][1] = (A[0][0] * A[2][2] - A[0][2] * A[2][0]);
		withA[2][1] = (A[0][0] * A[2][1] - A[0][1] * A[2][0]) * -1;

		withA[0][2] = (A[0][1] * A[1][2] - A[0][2] * A[1][1]);
		withA[1][2] = (A[0][0] * A[1][2] - A[0][2] * A[1][0]) * -1;
		withA[2][2] = (A[0][0] * A[1][1] - A[0][1] * A[1][0]);

		//求逆矩阵
		invA[0][0] = withA[0][0] / determinant; invA[0][1] = withA[0][1] / determinant; invA[0][2] = withA[0][2] / determinant;
		invA[1][0] = withA[1][0] / determinant; invA[1][1] = withA[1][1] / determinant; invA[1][2] = withA[1][2] / determinant;
		invA[2][0] = withA[2][0] / determinant; invA[2][1] = withA[2][1] / determinant; invA[2][2] = withA[2][2] / determinant;

		//求解未知矩阵B: A * B = C) => withA * C = B

		D = invA[0][0] * C[0][0] + invA[0][1] * C[1][0] + invA[0][2] * C[2][0];
		E = invA[1][0] * C[0][0] + invA[1][1] * C[1][0] + invA[1][2] * C[2][0];
		F = invA[2][0] * C[0][0] + invA[2][1] * C[1][0] + invA[2][2] * C[2][0];

		centerX = -D / 2;
		centerY = -E / 2;
		if ((pow(D, 2) + pow(E, 2) - 4 * F) < 0)
			continue;
		r = sqrt((pow(D,2) + pow(E, 2) - 4 * F)) / 2;
		//循环判断有效点
		for (int i = 0; i < this->edgePoints.size(); i++)
		{
			int dir = abs(sqrt(pow(this->edgePoints.at(i).x - centerX, 2) + pow(this->edgePoints.at(i).y - centerY, 2)) - r);
			if (dir < 2)
			{
				pgroup.push_back(this->edgePoints.at(i));
			}
		}
		//判断是否符合条件
		if (pgroup.size() >= size)
		{
			this->effectivePoints.assign(pgroup.begin(), pgroup.end());//清空并复制
			break;
		}
		else if (pgroup.size() > maxsize)
		{
			this->effectivePoints.assign(pgroup.begin(), pgroup.end());//清空并复制
			maxsize = pgroup.size();
		}
		--iters;
	}
	if (this->effectivePoints.size() < 3)
		return;
	cv::RotatedRect rec = fitEllipse(this->effectivePoints);
	this->center = rec.center;
	this->radius = (rec.size.height/2 + rec.size.width/2)/2;
}

void SRVision::SRFindBlob::findBlob(cv::Mat &img,cv::Mat &out, BlobControl params)
{
	this->result.clear();
	Mat output  = Mat::zeros(img.size(), CV_32S);
	Mat deImage = img.clone();
	if (deImage.type() == CV_8UC1)
	{
		cvtColor(deImage, deImage, COLOR_GRAY2BGR);
	}
	Mat dst = Mat::zeros(deImage.size(), deImage.type());
	Mat stats, centroids;
	int num_labels = cv::connectedComponentsWithStats(img, output, stats, centroids, 8, 4);

	for (size_t row = 0; row < deImage.rows; row++) 
	{
		for (size_t col = 0; col < deImage.cols; col++) 
		{
			if (output.at<int>(row, col) == 0)
				continue;
			dst.at<Vec3b>(row, col) = Vec3b(0, 255, 0);
		}
	}
	//获取统计信息
	/*
	其中stats包括以下枚举类型数据信息： 
    CC_STAT_LEFT   组件的左上角点像素点坐标的X位置
    CC_STAT_TOP    组件的左上角点像素点坐标的Y位置
    CC_STAT_WIDTH  组件外接矩形的宽度 
    CC_STAT_HEIGHT 组件外接矩形的高度
    CC_STAT_AREA   当前连通组件的面积（像素单位）
	*/
	for (int i = 1; i < num_labels; i++)
	{
		Vec2d pt = centroids.at<Vec2d>(i, 0);
		int x = stats.at<int>(i, CC_STAT_LEFT);
		int y = stats.at<int>(i, CC_STAT_TOP);
		int width = stats.at<int>(i, CC_STAT_WIDTH);
		int height = stats.at<int>(i, CC_STAT_HEIGHT);
		int area = stats.at<int>(i, CC_STAT_AREA);
		circle(dst, Point(pt[0], pt[1]), 2, Scalar(0, 0, 255), -1, 8, 0);
		rectangle(dst, Rect(x, y, width, height), Scalar(0, 0, 255), 1, 8, 0);
	}
	imshow("连通域标记图像", dst);

}

/*
* 轮廓提取
* @param[in] image 输入图像
* @param[in] roi 矩形ROI
* @param[in] filter 滤波处理
* @param[in] size 滤波系数
* @param[in] min 低阈值
* @param[in] max 高阈值
* @param[in] minLength 最小轮廓
* @param[in] maxLength 最大轮廓
* @param[in] step 取点间隔
* @param[in] smooth 平滑次数
*/
void SRVision::SRFindContour::findContour(cv::Mat & image, SRroiRect roi, int filter, int size, int min, int max, int minLength, int maxLength,int zoom, int step, int smooth)
{
	this->result.resize(0);
	if (roi.width == 0 || roi.height == 0)
	{
		return;
	}
	cv::Mat src = image(Rect(roi.center.x- roi.width / 2, roi.center.y - roi.height / 2, roi.width, roi.height));
	//cv::imshow("src", src);
	Mat resultImg;
	this->result.clear();
	switch (filter)//图形滤波
	{
	case 0://canny滤波
	{
		cv::Canny(src, resultImg, min, max, size);
		break;
	}
	case 1://sobel滤波
	{
		Mat rX, rY;
		cv::Sobel(src, rX, src.depth(), 1, 0, size);
		cv::Sobel(src, rY, src.depth(), 0, 1, size);
		//resultImg = rX + rY;
		//梯度相加
		cv::addWeighted(rX, 0.5, rY, 0.5, 0, resultImg);
		break;
	}
	default:
		break;
	}
	this->outImage = resultImg.clone();
	cv::findContours(resultImg, this->result, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
	for (vector<vector<cv::Point>>::iterator it = result.begin(); it != result.end();/**/)
	{
		if ((*it).size() > maxLength || (*it).size() < minLength)
		{
			it = result.erase(it);
		}
		else
		{
			++it;
		}
	}
	//间隔取点
	for (vector<vector<cv::Point>>::iterator it = result.begin(); it != result.end(); it++)
	{
		int i = 0;
		for (vector<cv::Point>::iterator itt = (*it).begin(); itt != (*it).end(); /**/)
		{
			if (i % step != 0)
			{
				itt = (*it).erase(itt);
			}
			else
			{
				(*itt).x = (*itt).x + int(roi.center.x - roi.width / 2);
				(*itt).y = (*itt).y + int(roi.center.y - roi.height / 2);
				++itt;
			}
			i++;
		}
	}
	//调用轮廓平滑
	int i = smooth;
	while (i--)
	{
		FilterContour();
	}
	//调用缩放轮廓
	ContoursZomm(zoom);
	//点集优化(主要优化缩放后点集扎堆问题)
	OptimizeContour();//待完成
}
//缩放轮廓
void SRVision::SRFindContour::ContoursZomm(int zoom)
{
	if (zoom == 0)
		return;
	double A, B, C, xf, yf;
	cv::Point p1, p2, p3, p4;
	//-------------------------------//
	// ↑  ↑  ↑////p1,p2,p3在数据中的位置///
	// p1  p2  p3/////////////////
	vector<cv::Point> endResult;
	for (vector<vector<cv::Point>>::iterator it = result.begin(); it != result.end(); it++)
	{
		endResult.resize(0);
		for (vector<cv::Point>::iterator itt = (*it).begin(); itt != (*it).end(); itt++)
		{
			if (itt == (*it).begin())
			{
				p1 = (*itt);
				p2 = (*itt);
				p3 = (*itt);
				continue;
			}
			else if (p1 == p3)
			{
				p2 = (*itt);
				p3 = (*itt);
				continue;
			}
			else
			{
				p3 = (*itt);
			}
			//向量法 向量p1->p2(x1,y1) p3->p2(x2,y2) 偏移向量=(x1+x2,y1+y2)
			double px = (p1.x - p2.x) + (p3.x - p2.x);
			double py = (p1.y - p2.y) + (p3.y - p2.y);
			//归一化
			double fmod = px * px + py * py;
			if (fmod != 0)
			{
				xf = px * sqrt(fmod) / fmod;
				yf = py * sqrt(fmod) / fmod;
				p4.x = p2.x + xf * zoom;
				p4.y = p2.y + yf * zoom;
				endResult.push_back(p4);
			}
			else
			{
				//法向法，依次将每个点在相邻两点的法向上平移
			//Ax+By+C=0; 法向为(B,-A)
				A = (p3.y - p1.y);
				B = (p1.x - p3.x);
				C = p3.x*p1.y - p1.x*p3.y;
				//法向归一化（xf,yf）
				double fmod = A * A + B * B;
				xf = A * sqrt(fmod) / fmod;
				yf = B * sqrt(fmod) / fmod;
				p4.x = p2.x + xf * zoom;
				p4.y = p2.y + yf * zoom;
				endResult.push_back(p4);
			}
			//内缩结束,p1,p2点后移
			p1 = p2;
			p2 = p3;
			
		}
		(*it) = endResult;
	}
}
//轮廓平滑
void SRVision::SRFindContour::FilterContour()
{
	cv::Point p1, p2, p3, p4;
	//-------------------------------//
	// ↑  ↑  ↑////p1,p2,p3在数据中的位置///
	// p1  p2  p3/////////////////
	vector<cv::Point> endResult;
	for (vector<vector<cv::Point>>::iterator it = result.begin(); it != result.end(); it++)
	{
		endResult.resize(0);
		for (vector<cv::Point>::iterator itt = (*it).begin(); itt != (*it).end(); itt++)
		{
			if (itt == (*it).begin())
			{
				p1 = (*itt);
				p2 = (*itt);
				p3 = (*itt);
				endResult.push_back(p1);
				continue;
			}
			else if (p1 == p3)
			{
				p2 = (*itt);
				p3 = (*itt);
				continue;
			}
			else
			{
				p3 = (*itt);
			}
			p4.x = (p1.x + p2.x + p3.x) / 3;
			p4.y = (p1.y + p2.y + p3.y) / 3;
			endResult.push_back(p4);
			if (itt == (*it).end())
			{
				endResult.push_back(p3);
			}
			p1 = p2;
			p2 = p3;
		}
		(*it) = endResult;
	}
}

void SRVision::SRFindContour::OptimizeContour()
{

}
//检查结果是否存在并插入
void SRVision::SRMatch::checkValkue(MatchValue v0)
{
	if (this->result.empty())
	{
		this->result.push_back(v0);
		return;
	}
	for (auto &v : this->result)
	{
		if (abs(v.Loc.x - v0.Loc.x) < 10)
			if (abs(v.Loc.y - v0.Loc.y) < 10)
				if (v.Value < v0.Value)
				{
					v = v0;
					return;
				}
				else
				{
					return;
				}
	}
	this->result.push_back(v0);
}
//生成匹配的结果图像
void SRVision::SRMatch::buildResultImg()
{
	if (this->result.empty())
		return;
	for (auto &v : this->result)
	{
		cv::Point pLeftUp = cv::Point(v.Loc.x + this->diffCols / 2, v.Loc.y + this->diffRows / 2);
		cv::Point pRightDowm = cv::Point(pLeftUp.x + this->templateImg.cols, pLeftUp.y + this->templateImg.rows);
		cv::Point pLeftDowm = cv::Point(pLeftUp.x, pRightDowm.y);
		cv::Point pRightUp = cv::Point(pRightDowm.x, pLeftUp.y);
		cv::Point pCenter = cv::Point(pLeftUp.x + round(this->templateImg.cols / 2), pLeftUp.y + round(this->templateImg.rows / 2));
		Point point[4] = { pLeftUp,pRightDowm,pLeftDowm,pRightUp };
		Point rpoint[4] = { cv::Point(0,0) };
		double angle = v.Angle * CV_PI / 180;
		for (int i = 0; i < 4; i++)
		{
			int offsetX = point[i].x - pCenter.x;
			int offsetY = point[i].y - pCenter.y;
			rpoint[i].x = round(offsetX * cos(angle) + offsetY * sin(angle) + pCenter.x);
			rpoint[i].y = round(-offsetX * sin(angle)+ offsetY * cos(angle) + pCenter.y);
		}
		cv::Point rLU = rpoint[0], rRD = rpoint[1], rLD = rpoint[2], rRU = rpoint[3];
		line(this->resultImg, rLU, rRU, Scalar(0, 255, 0), 1, 4);
		line(this->resultImg, rRU, rRD, Scalar(0, 255, 0), 1, 4);
		line(this->resultImg, rRD, rLD, Scalar(0, 255, 0), 1, 4);
		line(this->resultImg, rLD, rLU, Scalar(0, 255, 0), 1, 4);
 		circle(this->resultImg, pCenter, 2, Scalar(0, 255, 0), -1);//画点
	}
}
/*
		* 模板匹配（旋转模板依次匹配）
		* @param[in] image 输入图像
		* @param[in] templat 模板图像
		* @param[in] level 金字塔层级
		* @param[in] startAngle 开始角度
		* @param[in] endAngle 结束角度
		* @param[in] angelStep 角度步长
		* @param[in] grade 最小匹配分数
		*/
void SRVision::SRMatch::templateMatch(cv::Mat& image, cv::Mat& temp, int level, int startAngle, int endAngle, int angleStep, int grade, int count)
{
	this->result.clear();
	this->resultImg = Mat::zeros(image.rows, image.cols, CV_8UC3);
	if (temp.rows > image.rows || temp.cols > image.cols)
		return;
	vector<Mat> channels;
	for (int i = 0; i < 3; i++)
	{
		channels.push_back(image);
	}
	merge(channels, this->resultImg);
	for (int i = startAngle; i < endAngle; i += angleStep)
	{
		/*建立掩膜 掩膜灰度为0的部分匹配不起作用
		方法有两种
		1.先旋转、将旋转后的黑边剔除做mask。会损失一部分模板信息
		2.放大图像尺寸再旋转。匹配时使用mask，不丢失模板信息
		本次采用第二种
		*/
		//扩大模板，边界以0填充，大小为根号下模板尺寸的一半
		Mat mask, rotaTemp;
		int top = (sqrt(temp.rows*temp.rows + temp.cols *temp.cols) - temp.rows)/ 2 ;
		int bottom = (sqrt(temp.rows * temp.rows + temp.cols * temp.cols) - temp.rows) / 2;
		int left = (sqrt(temp.rows * temp.rows + temp.cols * temp.cols) - temp.cols) / 2;
		int right = (sqrt(temp.rows * temp.rows + temp.cols * temp.cols) - temp.cols) / 2;
		cv::copyMakeBorder(temp, rotaTemp, top, bottom, left, right, cv::BORDER_ISOLATED);
		//旋转
		rotaTemp = getRotationImage(rotaTemp, i);
		//获取旋转后图片放大了多少
		this->diffRows = rotaTemp.rows - temp.rows;
		this->diffCols = rotaTemp.cols - temp.cols;
		//获得mask
		//cvtColor(rotaTemp, mask, COLOR_BGR2GRAY);
		threshold(rotaTemp, mask, 2, 255, THRESH_BINARY);
		//cv::imshow("temp", rotaTemp);
		//cv::imshow("mask", mask);
		//匹配	
		double minValue, maxValue;
		cv::Point minLoc, maxLoc;
		int width = image.cols - rotaTemp.cols + 1;  //result 宽度
		int height = image.rows - rotaTemp.rows + 1; //result 高度
		Mat res(height, width, CV_32FC1); //创建结果映射图像
		cv::matchTemplate(image, rotaTemp, res, cv::TM_CCOEFF_NORMED, mask);
	
		//获取匹配结果
		cv::minMaxLoc(res, &minValue, &maxValue, &minLoc, &maxLoc);
		maxValue = round(maxValue * 100);//取前两位
		MatchValue value(maxLoc, maxValue, i);
		//检查并插入结果 
		if(maxValue > grade)
			checkValkue(value);
	}
	//生成结果图像
	buildResultImg();
}
void SRVision::SRMatch::setTemplate(cv::Mat & image, SRroiRect roi)
{
	this->templateImg = image;
}
