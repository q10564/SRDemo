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

