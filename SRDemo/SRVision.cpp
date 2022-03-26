#include "SRVision.h"

bool calibration(vector<Point2f> world, vector<Point2f> pix, calibResult &calib)
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
Mat cvtColor_RGB2HSI(Mat &input)
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
Mat ImageRotation(cv::Mat &image, double angle)
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
cv::Mat ImageMirror(cv::Mat &image, int type)
{
	cv::Mat dst;
	cv::Mat src = image.clone();
	cv::flip(src, dst, type);//0 x轴镜像  <0 y轴镜像 >0同时翻转
	return dst;
}
/*
函数作用：
		获取直方图
	变量：
		input:输入图像
		output：输出直方图图像
*/
void GetCalcHistImage(Mat input, Mat &output)
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
	int width = 256;
	int height = 256;
	Mat histImage(height, width, CV_8UC3, Scalar(255, 255, 255));
	//设置直方图有256个区间,因为图像的灰度值变化为0~255
	int numbers = 256;
	//定义变量范围，并定义三个矩阵来存储每个直方图
	float range[] = { 0, 256 };
	const float *histRange = { range };
	int binStep = cvRound((float)width / (float)numbers);
	if (input.channels() == 1)
	{
		Mat hist;
		calcHist(&input, 1, 0, Mat(), hist, 1, &numbers, &histRange);
		normalize(hist, hist, 0, height, NORM_MINMAX);
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
	}
	else
	{
		vector<Mat> bgr;
		//将输入图像划分为三个通道R、G、B
		split(input, bgr);
		Mat b_hist, g_hist, r_hist;

		calcHist(&bgr[0], 1, 0, Mat(), b_hist, 1, &numbers, &histRange);
		calcHist(&bgr[1], 1, 0, Mat(), g_hist, 1, &numbers, &histRange);
		calcHist(&bgr[2], 1, 0, Mat(), r_hist, 1, &numbers, &histRange);


		//归一化
		normalize(b_hist, b_hist, 0, height, NORM_MINMAX);
		normalize(g_hist, g_hist, 0, height, NORM_MINMAX);
		normalize(r_hist, r_hist, 0, height, NORM_MINMAX);

		for (int i = 0; i < numbers; i++)
		{
			try
			{
				line(
					histImage,
					Point(binStep*(i - 1), height - cvRound(b_hist.at<float>(i - 1))),
					Point(binStep*(i), height - cvRound(b_hist.at<float>(i))),
					Scalar(255, 0, 0)
				);
				line(
					histImage,
					Point(binStep*(i - 1), height - cvRound(g_hist.at<float>(i - 1))),
					Point(binStep*(i), height - cvRound(g_hist.at<float>(i))),
					Scalar(0, 255, 0)
				);
				line(
					histImage,
					Point(binStep*(i - 1), height - cvRound(r_hist.at<float>(i - 1))),
					Point(binStep*(i), height - cvRound(r_hist.at<float>(i))),
					Scalar(0, 0, 255)
				);
			}
			catch (Exception exception)
			{
				printf(exception.err.c_str());
			}
		}
	}
	output = histImage;
}

