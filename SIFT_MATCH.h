#pragma once
#include "common.h"
using namespace cv;
using namespace std;


const int sift_mode = 1;

//to extract and pairs the SIFT features
void sift_match(Mat src1, Mat src2, vector<KeyPoint>& keypoints1, vector<KeyPoint>& keypoints2, vector<DMatch>& res_matches);

//get strategy for cut
void getStrategy(vector<POINT> points, vector<POINT> bombs, vector<vector<POINT>>& strategy, int size_x, int size_y, int grid_x, int grid_y, int threshold);
