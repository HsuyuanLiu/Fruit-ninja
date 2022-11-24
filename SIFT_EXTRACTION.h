#pragma once
#include"common.h"
using namespace std;
using namespace cv;

/*global variables*/
const int num_Gau = 6;//gaussian pyramid layer size
const int num_DoG = 5;//DoG layer size
const int num_not_check = 5;//margin pixel not to check
const double S = 3.0;//DoG median
const double exp_thresh = 0.03;//
const double SIFT_FIXPT_SCALE = 48;//
const double MAX_ITER = 5;//max iteration
const double R_ = 10.f;
const double PAI = CV_PI;
const double sec_dir_ratio = 0.8;



struct group_Gau
{
	Mat Gau[num_Gau];
};

struct group_DoG
{
	Mat DoG[num_DoG];
};

struct KPoint
{
	Point2f pt;
	int octave;
	float size;
	int layer;
};

struct myMatch
{
	int srcIdx = -1;
	int nearestIdx = -1;
	int secondNearestIdx = -1;
	float nearestDistance = 10000;
	float secondNearestDistance = 10000;
	float ratio = 0.0;
};

/*global variant */
Mat testImg, testImg1;
double sigma = 0.8;//initial sigma value
double k = sqrt(2);
int k_size = 49;//kernel size
int num_groups = 0;//
int num_sift = 0;
//vector<KPoint> keys;//key pairs
bool ifMatch = false;
bool isdebug = false;






//check in the minium 
bool check_min(group_DoG* pyramid, int group, int layer, int row, int col)
{
	double base = pyramid[group].DoG[layer].at<double>(row, col);
	for (int l = -1; l <= 1; l++)
	{
		for (int r = -1; r <= 1;r++)
		{
			for (int c = -1;c <= 1;c++)
			{
				if (pyramid[group].DoG[layer + l].at<double>(row + r, col + c) < base)
					return false;
			}
		}
	}

	return true;
}


//check in the maxium 
bool check_max(group_DoG* pyramid, int group, int layer, int row, int col)
{
	double base = pyramid[group].DoG[layer].at<double>(row, col);
	for (int l = -1; l <= 1; l++)
	{
		for (int r = -1; r <= 1;r++)
		{
			for (int c = -1;c <= 1;c++)
			{
				if (pyramid[group].DoG[layer + l].at<double>(row + r, col + c) > base)
					return false;
			}
		}
	}
	return true;
}

double fun1(double x, double y)
{
	return sqrt(pow(x, 2) + pow(y, 2));
}
void debug()
{
	isdebug = true;
	return;
}
int whichBin(int x)
{
	if (x >= -8 && x <= -5)
		return 0;
	if (x >= -4 && x <= -1)
		return 1;
	if (x >= 0 && x <= 3)
		return 2;
	if (x >= 4 && x <= 7)
		return 3;
}



bool step2(group_DoG* pyramid, KeyPoint& resPt, int& group, int& layer, int& row, int& col);


//calculate directions
double cal_direction(const Mat& value_gdt, const Mat& dir_gdt, Point2f& pt, double scale, double* hist, int n);


//calculate descriptor
void cal_descriptor(const Mat& gauss_image, const Mat& value_gdt, const Mat& dir_gdt, float main_ori, Point2f pt, int d, float scale, int n, double* descriptor);


vector<KeyPoint> mySift(Mat testImg, Mat& descriptor);

//
vector<DMatch> mySiftMatch(Mat& desc1, Mat& desc2, double thresh);