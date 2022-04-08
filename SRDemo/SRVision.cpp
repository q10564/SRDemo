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
