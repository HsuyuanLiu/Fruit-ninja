#include "SIFT_EXTRACTION.h"

bool step2(group_DoG* pyramid, KeyPoint& resPt, int& group, int& layer, int& row, int& col)
{
	const double img_scale = 1.f / (255 * SIFT_FIXPT_SCALE);
	const double der_scale = img_scale * 0.5f;
	const double sec_der_scale = img_scale;
	const double cross_sec_der_scale = img_scale * 0.25f;
	/*const double der_scale =  0.5f;
	const double sec_der_scale = 0.25f;
	const double cross_sec_der_scale =  0.25f;*/

	double xs = 0, xr = 0, xc = 0;
	int iter_id = 0;
	for (iter_id = 0;iter_id < MAX_ITER;iter_id++)
	{
		const Mat& cur_layer = pyramid[group].DoG[layer];
		const Mat& front_layer = pyramid[group].DoG[layer - 1];
		const Mat& next_layer = pyramid[group].DoG[layer + 1];

		double  dx = (cur_layer.at<double>(row, col + 1) - cur_layer.at<double>(row, col - 1)) * der_scale;
		double  dy = (cur_layer.at<double>(row + 1, col) - cur_layer.at<double>(row - 1, col)) * der_scale;
		double  dz = (next_layer.at<double>(row, col) - front_layer.at<double>(row, col)) * der_scale;

		double dxx = (cur_layer.at<double>(row, col + 1) + cur_layer.at<double>(row, col - 1) - cur_layer.at<double>(row, col) * 2.f) * sec_der_scale;
		double dyy = (cur_layer.at<double>(row + 1, col) + cur_layer.at<double>(row - 1, col) - cur_layer.at<double>(row, col) * 2.f) * sec_der_scale;
		double dzz = (next_layer.at<double>(row, col) + front_layer.at<double>(row, col) - cur_layer.at<double>(row, col) * 2.f) * sec_der_scale;

		double dxy = (cur_layer.at<double>(row + 1, col + 1) + cur_layer.at<double>(row - 1, col - 1) - cur_layer.at<double>(row + 1, col - 1) - cur_layer.at<double>(row - 1, col + 1)) * cross_sec_der_scale;
		double dxz = (next_layer.at<double>(row, col + 1) + front_layer.at<double>(row, col - 1) - next_layer.at<double>(row, col - 1) - front_layer.at<double>(row, col + 1)) * cross_sec_der_scale;
		double dyz = (next_layer.at<double>(row + 1, col) + front_layer.at<double>(row - 1, col) - next_layer.at<double>(row - 1, col) - front_layer.at<double>(row + 1, col)) * cross_sec_der_scale;

		Matx33f H(dxx, dxy, dxz, dxy, dyy, dyz, dxz, dyz, dzz);
		Vec3f dD(dx, dy, dz);
		Vec3f X = H.solve(dD, DECOMP_SVD);

		xc = -X[0];
		xr = -X[1];
		xs = -X[2];

		if (abs(xc) < 0.5f && abs(xr) < 0.5f && abs(xs) < 0.5f)
			break;

		double max_c_shift = cur_layer.cols / 3.f;
		double max_r_shift = cur_layer.rows / 3.f;
		if (abs(xc) > max_c_shift || abs(xr) > max_r_shift)
		{
			return false;
		}
		else
		{
			if (layer + abs(xs) > num_DoG - 1 || layer - abs(xs) < 0)
			{
				return false;
			}
		}

		col = col + cvRound(xc);
		row = row + cvRound(xr);
		layer = layer + cvRound(xs);

		if (layer < 1
			|| layer >= num_DoG - 1
			|| row < num_not_check
			|| row >= cur_layer.rows - num_not_check
			|| col < num_not_check
			|| col >= cur_layer.cols - num_not_check)
		{
			return false;
		}

	}
	if (iter_id >= MAX_ITER)
		return false;

	const Mat& cur_layer = pyramid[group].DoG[layer];
	const Mat& front_layer = pyramid[group].DoG[layer - 1];
	const Mat& next_layer = pyramid[group].DoG[layer + 1];

	double  dx = (cur_layer.at<double>(row, col + 1) - cur_layer.at<double>(row, col - 1)) * der_scale;
	double  dy = (cur_layer.at<double>(row + 1, col) - cur_layer.at<double>(row - 1, col)) * der_scale;
	double  dz = (next_layer.at<double>(row, col) - front_layer.at<double>(row, col)) * der_scale;

	Matx31f dD(dx, dy, dz);
	double t = dD.dot(Matx31f(xc, xr, xs));
	double _D_ = cur_layer.at<double>(row, col) + t * 0.5f;

	if (abs(_D_) < exp_thresh)
		return false;

	double dxx = (cur_layer.at<double>(row, col + 1) + cur_layer.at<double>(row, col - 1) - cur_layer.at<double>(row, col) * 2.f) * sec_der_scale;
	double dyy = (cur_layer.at<double>(row + 1, col) + cur_layer.at<double>(row - 1, col) - cur_layer.at<double>(row, col) * 2.f) * sec_der_scale;
	double dxy = (cur_layer.at<double>(row + 1, col + 1) + cur_layer.at<double>(row - 1, col - 1) - cur_layer.at<double>(row + 1, col - 1) - cur_layer.at<double>(row - 1, col + 1)) * cross_sec_der_scale;

	double det = dxx * dyy - dxy * dxy;
	double Tr = dxx + dyy;

	if (det <= 0 || Tr * Tr * R_ >= det * (R_ + 1) * (R_ + 1))
	{
		return false;
	}

	resPt.pt.x = ((double)col + xc) * (1 << group);
	resPt.pt.y = ((double)row + xr) * (1 << group);
	//resPt.pt.x = ((double)col + xc);
	//resPt.pt.y = ((double)row + xr);

	//resPt.octave = group + (layer << 8) + (cvRound((xs+0.5)*255)<<16);
	resPt.octave = group + (int)(layer << 8);
	resPt.size = sigma * pow(2, (double)(group + (double)(layer - 1) / (double)S));
	resPt.response = abs(_D_);

	return true;
    //return false;
}

double cal_direction(const Mat& value_gdt, const Mat& dir_gdt, Point2f& pt, double scale, double* hist, int n)
{
	for (int i = 0; i < n; i++)
	{
		hist[i] = 0.0;
	}
	int num_row = value_gdt.rows;
	int num_col = value_gdt.cols;

	Point point(cvRound(pt.x), cvRound(pt.y));
	int radius = cvRound(3 * 1.5 * scale);

	radius = min(radius, min(num_row / 2, num_col / 2));

	int radius_x_left = point.x - radius;
	int radius_x_right = point.x + radius;
	int radius_y_up = point.y - radius;
	int radius_y_down = point.y + radius;


	if (radius_x_left < 0)
		radius_x_left = 0;
	if (radius_x_right > num_col - 1)
		radius_x_right = num_col - 1;
	if (radius_y_up < 0)
		radius_y_up = 0;
	if (radius_y_down > num_row - 1)
		radius_y_down = num_row - 1;

	for (int r = radius_y_up; r <= radius_y_down; r++)
	{
		for (int c = radius_x_left; c <= radius_x_right; c++)
		{
			double value = value_gdt.at<double>(r, c);
			double dir = dir_gdt.at<double>(r, c);
			if (dir < 0)
				dir = dir + 2 * PAI;
			dir = dir * 360 / (2 * PAI);
			int idx = (int)(cvRound(dir) / 10);
			hist[idx] += value;
		}
	}
	double max_value = hist[0];
	for (int i = 1; i < n; i++)
	{
		if (hist[i] > max_value)
			max_value = hist[i];
	}
	return max_value;
    //return 0.0;
}

void cal_descriptor(const Mat& gauss_image, const Mat& value_gdt, const Mat& dir_gdt, float main_ori, Point2f pt, int d, float scale, int n, double* descriptor)
{
	Point point(cvRound(pt.x), cvRound(pt.y));

	double cosine = cosf(-main_ori * (float)(CV_PI / 180));
	double sine = sinf(-main_ori * (float)(CV_PI / 180));

	float bins_per_rad = n / 360.f;

	int num_rows = gauss_image.rows;
	int num_cols = gauss_image.cols;

	int radius = 8;

	vector<vector<double>> bins;
	bins.resize(16);
	for (int i = 0;i < 16;i++)
	{
		bins[i].resize(8);
	}
	for (int i = 0;i < 16;i++)
	{
		for (int j = 0;j < 8;j++)
		{
			bins[i][j] = 0;
		}
	}

	int radius_x_left = point.x - radius;
	int radius_x_right = point.x + radius;
	int radius_y_up = point.y - radius;
	int radius_y_down = point.y + radius;

	
	

	for (int r = radius_y_up; r < radius_y_down; r++)
	{
		for (int c = radius_x_left; c < radius_x_right; c++)
		{
			/*if (isdebug)
				cout << r << "   " << c << endl;
			if (r == 46 && c == 443)
				debug();*/

			int r_d = whichBin(r - point.y);
			int c_d = whichBin(c - point.x);
			int bin_id = r_d * 4 + c_d;
			int r_idx = cvRound(c * cosine - r * sine);
			int c_idx = cvRound(c * cosine + r * sine);

			if (r_idx < 0 || r_idx >= num_rows || c_idx < 0 || c_idx >= num_cols)
			{
				continue;
			}

			double value = value_gdt.at<double>(r_idx, c_idx);
			double dir = dir_gdt.at<double>(r_idx, c_idx);
			if (dir < 0)
				dir = dir + 2 * PAI;
			dir = dir * 360 / (2 * PAI);
			int idx = cvRound(dir) / 45;
			if (idx == 8)
				idx--;
			bins[bin_id][idx] += value;
		}
	}
	for (int i = 0;i < n;i++)
	{
		int bin_id = i / 8;
		int dir_id = i % 8;
		descriptor[i] = bins[bin_id][dir_id];
	}


	double norm = 0;
	for (int i = 0; i < 128; i++)
	{
		norm = norm + descriptor[i] * descriptor[i];
	}
	norm = sqrt(norm);
	for (int i = 0; i < 128; i++)
	{
		descriptor[i] = descriptor[i] / norm;
	}
}

vector<KeyPoint> mySift(Mat testImg, Mat& descriptor)
{
	Mat testImgGray, img1;
	vector<KeyPoint> toDraw;//���Ե���draw�����Ĺؼ��㼯��
	cvtColor(testImg, testImgGray, COLOR_BGR2GRAY);
	char grayname[100];
	char resname[100];
	
	//to  flost
	testImgGray.convertTo(testImgGray, CV_64FC1);

	//calculate numbers of pyramid groups
	num_groups = ((int)log2(min(testImgGray.rows, testImgGray.cols))) - 2;
	//num_groups = 2;
	group_DoG* pyramid = new group_DoG[num_groups];//DoG pyramid
	group_Gau* gaus = new group_Gau[num_groups];//gaussian pyramid
	group_Gau* x_gdt = new group_Gau[num_groups];//x gradient
	group_Gau* y_gdt = new group_Gau[num_groups];//y gradient
	group_Gau* value_gdt = new group_Gau[num_groups];
	group_Gau* dir_gdt = new group_Gau[num_groups];
	toDraw.clear();
	//initialize the pyramid groups, row=col
	for (int i = 0; i < num_groups;i++)
	{
		int rows = 0, cols = 0;
		int gau_rows = 0, gau_cols = 0;
		if (i == 0)
		{
			rows = testImgGray.rows;
			cols = testImgGray.cols;
			gau_rows = testImgGray.rows;
			gau_cols = testImgGray.cols;
		}
		else
		{
			rows = pyramid[i - 1].DoG[num_DoG - 1].rows / 2;
			cols = pyramid[i - 1].DoG[num_DoG - 1].cols / 2;
			gau_rows = gaus[i - 1].Gau[num_Gau - 1].rows / 2;
			gau_cols = gaus[i - 1].Gau[num_Gau - 1].cols / 2;
		}
		for (int j = 0;j < num_DoG;j++)
		{
			pyramid[i].DoG[j].create(rows, cols, CV_64FC1);
		}
		for (int j = 0; j < num_Gau;j++)
		{
			gaus[i].Gau[j].create(gau_rows, gau_cols, CV_64FC1);
			x_gdt[i].Gau[j].create(gau_rows, gau_cols, CV_64FC1);
			y_gdt[i].Gau[j].create(gau_rows, gau_cols, CV_64FC1);
			value_gdt[i].Gau[j].create(gau_rows, gau_cols, CV_64FC1);
			dir_gdt[i].Gau[j].create(gau_rows, gau_cols, CV_64FC1);
		}
	}
	

	//Calculate gaussian paramid and DoG pyramid
	for (int i = 0; i < num_groups; i++)
	{
		for (int j = 0; j < num_Gau;j++)
		{
			if (i == 0 && j == 0)
			{
				//first layer
				gaus[i].Gau[j] = testImgGray;

			}
			else if (i != 0 && j == 0)
			{
				
				pyrDown(gaus[i - 1].Gau[num_Gau - 1], gaus[i].Gau[j], Size(gaus[i].Gau[j].cols, gaus[i].Gau[j].rows));
			}
			else
			{
				double tmp_sig = sigma * pow(2, (double)(i + (double)(j - 1) / (double)S));
				GaussianBlur(gaus[i].Gau[j - 1], gaus[i].Gau[j], Size(k_size, k_size), tmp_sig);
			}
			Sobel(gaus[i].Gau[j], x_gdt[i].Gau[j], -1, 1, 0);
			Sobel(gaus[i].Gau[j], y_gdt[i].Gau[j], -1, 0, 1);
			for (int r = 0;r < value_gdt[i].Gau[j].rows; r++)
			{
				for (int c = 0;c < value_gdt[i].Gau[j].cols;c++)
				{
					value_gdt[i].Gau[j].at<double>(r, c) = fun1(x_gdt[i].Gau[j].at<double>(r, c), y_gdt[i].Gau[j].at<double>(r, c));
					dir_gdt[i].Gau[j].at<double>(r, c) = atan2(y_gdt[i].Gau[j].at<double>(r, c), x_gdt[i].Gau[j].at<double>(r, c));

				}
			}
		}
	}

	for (int i = 0; i < num_groups; i++)
	{
		for (int j = 1; j < num_Gau;j++)
		{
			pyramid[i].DoG[j - 1] = gaus[i].Gau[j] - gaus[i].Gau[j - 1];
		}
	}

	int num_before = 0, num_after = 0;
	for (int i = 0; i < num_groups;i++)
	{
		for (int j = 0; j < num_DoG;j++)
		{
			if (j > 0 && j < num_DoG - 1)
			{
				for (int r = num_not_check;r < pyramid[i].DoG[j].rows - num_not_check; r++)
				{
					for (int c = num_not_check;c < pyramid[i].DoG[j].cols - num_not_check;c++)
					{

						if (check_min(pyramid, i, j, r, c) || check_max(pyramid, i, j, r, c))
						{
							num_before++;

							int tmp_group = i, tmp_layer = j, tmp_row = r, tmp_col = c;
							KeyPoint resPt;
							//cout << i <<" "<<" "<< j <<" "<< r <<" "<< c<<endl;
							/*if (r == 159 && c == 462)
							{
								debug();
							}*/
							if (!step2(pyramid, resPt, tmp_group, tmp_layer, tmp_row, tmp_col))
							{
								continue;
							}
							int layer = resPt.octave >> 8;
							//int now_group = resPt.octave & 0xff;
							num_after++;
							Mat& cur_value_gdt = value_gdt[i].Gau[layer - 1];
							Mat& cur_dir_gdt = dir_gdt[i].Gau[layer - 1];
							double scale = sigma * pow(2, (double)(i + (double)(layer - 1) / (double)S));
							int n = 36;
							double* hist = new double[n];
							double max_dir = cal_direction(cur_value_gdt, cur_dir_gdt, resPt.pt, scale, hist, n);
							double thresh_dir = max_dir * sec_dir_ratio;
							for (int k = 0; k < n;k++)
							{
								int left = k > 0 ? k - 1 : n - 1;
								int right = k < n - 1 ? k + 1 : 0;
								if (hist[k] >= thresh_dir && hist[k] > hist[left] && hist[k] > hist[right])
								{
									KeyPoint newPt(resPt);
									float bin = k + 0.5f * (hist[left] - hist[right]) / (hist[left] + hist[right] - 2 * hist[k]);
									bin = bin < 0 ? n + bin : bin >= n ? bin - n : bin;

									newPt.angle = (360.f / n) * bin;
									toDraw.push_back(newPt);

								}
							}
						}
					}
				}
			}
		}
	}
	//cout << num_before << " " << toDraw.size() << endl << endl << endl;

	//drawKeypoints(testImg, toDraw, img1);
	//imshow(resname, img1);

	vector<double*> desc;
	for (int i = 0;i < toDraw.size();i++)
	{
		int tmp_group = toDraw[i].octave & 0xff;
		int tmp_layer = toDraw[i].octave >> 8;
		Mat& cur_gau = gaus[tmp_group].Gau[tmp_layer];
		Mat& cur_value_gdt = value_gdt[tmp_group].Gau[tmp_layer];
		Mat& cur_dir_gdt = dir_gdt[tmp_group].Gau[tmp_layer];
		Point2f point;
		point.x = toDraw[i].pt.x / (1 << tmp_group);
		point.y = toDraw[i].pt.y * (1 << tmp_group);
		double* des = new double[128];
		//cout << i << endl;
		/*if (i == 143)
			debug();*/
		cal_descriptor(cur_gau, cur_value_gdt, cur_dir_gdt, toDraw[i].angle, point, 4, toDraw[i].size, 128, des);
		desc.push_back(des);
		//isdebug = false;
	}


	descriptor.create(toDraw.size(), 128, CV_64FC1);
	for (int i = 0;i < toDraw.size();i++)
	{
		for (int j = 0;j < 128;j++)
		{
			descriptor.at<double>(i, j) = desc[i][j];
		}
	}
	return toDraw;

}

vector<DMatch> mySiftMatch(Mat &desc1,Mat &desc2,double thresh)
{
	vector<DMatch> matches;
	vector<myMatch> myMatches;

	//cout << desc1.type() << endl << endl;
	//BF every pair of descriptor, keep the nearset pair and the ratios
	for (int i = 0;i < desc1.rows;i++){
		DMatch tmpDMatch;
		myMatch tmpMatch;
		tmpDMatch.queryIdx = i;
		tmpMatch.srcIdx = i;
		tmpDMatch.distance = 10000;
		tmpMatch.nearestDistance = 10000;
		tmpMatch.secondNearestDistance = 10000;
		
		for (int j = 0; j < desc2.rows; j++){
		
			float tmpDistance = 0;
			float squareDistance = 0;
			//eudican distance
			for (int k = 0; k < desc1.cols; k++)
			{
				squareDistance += pow((desc1.at<double>(i, k) - desc2.at<double>(j, k)), 2);
			}
			tmpDistance = sqrt(squareDistance);
			//keep match with low distance
			if (tmpDistance < tmpMatch.nearestDistance)
			{
				tmpMatch.secondNearestDistance = tmpMatch.nearestDistance;
				tmpMatch.secondNearestIdx = tmpMatch.nearestIdx;
				tmpMatch.nearestDistance = tmpDistance;
				tmpMatch.nearestIdx = j;
				tmpDMatch.imgIdx = tmpDMatch.trainIdx;
				tmpDMatch.trainIdx = j;
				tmpDMatch.distance = tmpDistance;
			}
			else if (tmpDistance >= tmpMatch.nearestDistance && tmpDistance < tmpMatch.secondNearestDistance)
			{
				tmpMatch.secondNearestDistance = tmpDistance;
				tmpMatch.secondNearestIdx = j;
				tmpDMatch.imgIdx = j;
			}
		}
		tmpMatch.ratio = tmpMatch.nearestDistance / tmpMatch.secondNearestDistance;
		matches.push_back(tmpDMatch);
		myMatches.push_back(tmpMatch);
	}
	//keep those below threshold, shows
	vector<DMatch> res_matches;
	for (int i = 0;i < myMatches.size(); i++)
	{
		if (myMatches[i].ratio <= thresh)
		{
			DMatch tmpMatch(myMatches[i].srcIdx, myMatches[i].nearestIdx, myMatches[i].nearestDistance);
			res_matches.push_back(tmpMatch);
		}
	}
	return res_matches;
	//Mat matchesImg;
	//drawMatches(testImg, keypoints1, testImg1, keypoints2, res_matches, matchesImg, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	//imshow("match_result", matchesImg);
	//return vector<DMatch>();
}
