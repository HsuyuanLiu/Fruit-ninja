#include "SIFT_MATCH.h"
#include "SIFT_EXTRACTION.h"

void sift_match(Mat src1, Mat src2, vector<KeyPoint>& keypoints1, vector<KeyPoint>& keypoints2, vector<DMatch>& res_matches)
{
	double thresh = 3;
	if (sift_mode == 1) {

		Ptr<SIFT> sift = SIFT::create();

		Mat desc1, desc2;
		sift->detectAndCompute(src1, Mat(), keypoints1, desc1);

		sift->detectAndCompute(src2, Mat(), keypoints2, desc2);

	    //need enough points, otherwise not good effect
		if (keypoints1.size() == 0 || keypoints2.size() == 0)
			return;
		FlannBasedMatcher matcher;
		vector<vector<DMatch>> matches;
		matcher.knnMatch(desc1, desc2, matches, 2);


		//use RATIO to filter, abandoned method
		//vector< DMatch > res_matches;
		for (int i = 0; i < desc1.rows; i++)
		{
			//if (matches[i][0].distance / matches[i][1].distance < thresh)
				res_matches.push_back(matches[i][0]);
		}
	}
	else {
		Mat desc1, desc2;
		keypoints1 = mySift(src1, desc1);
		keypoints2 = mySift(src2, desc2);
		res_matches = mySiftMatch(desc1, desc2, thresh);
	}
}

void getStrategy(vector<POINT> points, vector<POINT> bombs, vector<vector<POINT>>& strategy, int size_x, int size_y, int grid_x, int grid_y, int threshold) {
	//size: size of window
	// grid :size of grid

	strategy.clear();

	int vec_x = size_x / grid_x;//number of grid
	int vec_y = size_y / grid_y;
	vector<vector<int>> grid(vec_x + 1, vector<int>(vec_y + 1, 0));//index of grid
	int x = 0, y = 0;
	//if point matched with bomb, minus 1
	for (int i = 0; i < bombs.size(); ++i) {
		x = int((float)bombs[i].y / size_y);
		y = int((float)bombs[i].x / size_x);
		grid[x][y]--;
	}
	//below the threshold, set as -1
	for (int i = 0; i < grid.size(); ++i) {
		for (int j = 0; j < grid[0].size(); ++j) {
			if (grid[i][j] <= -threshold / 2) {
				grid[i][j] = -1;
			}
			else {
				grid[i][j] = 0;
			}
		}
	}
	//if point matched with fruit, +1
	for (int i = 0; i < points.size(); ++i) {
		x = int((float)points[i].y / grid_y);
		y = int((float)points[i].x / grid_x);
		if (grid[x][y] != -1) {
			grid[x][y]++;
		}
	}


	//set strategy for move
	POINT p1, p2;
	vector<POINT> temp;
	bool flag1 = false;//flag of start
	bool flag2 = false;//flag of end
	
	for (int i = 0; i < vec_x; ++i) {
		flag1 = false;
		flag2 = false;
		temp.clear();
		for (int j = 0; j < vec_y; ++j) {
			//set the first fruit grid as start 
			if (grid[i][j] >= threshold) {
				//median of the grid as start
				if (flag1 == false) {
					flag1 = true;
					p1.x = (j + 0.5) * grid_y;
					p1.y = (i + 0.5) * grid_x;
				}
				//no end, step into next grid
				else {
					flag2 = true;
					p2.x = (j + 0.5) * grid_y;
					p2.y = (i + 0.5) * grid_x;
				}
			}
			//bomb grid, set the end 
			if (grid[i][j] == -1) {
				j += 2;
				if (flag1 && flag2) {


					temp.push_back(p1);
					temp.push_back(p2);
					strategy.push_back(temp);
					/*cout << p1.x << "," << p1.y << endl;
					cout << p2.x << "," << p2.y << endl << endl;*/
				}
				else if (flag1 && !flag2) {

					temp.push_back(p1);
					p2.x = p1.x + grid_x / 2;
					p2.y = p1.y;
					temp.push_back(p2);
					strategy.push_back(temp);
					/*cout << p1.x << "," << p1.y << endl;
					cout << p2.x << "," << p2.y << endl << endl;*/
				}
				flag1 = false;
				flag2 = false;
				temp.clear();
			}
		}
		//no bomber, fruit grid in row, long line
		if (flag1 && flag2) {

			temp.push_back(p1);
			temp.push_back(p2);
			strategy.push_back(temp);
			/*cout << p1.x << "," << p1.y << endl;
			cout << p2.x << "," << p2.y << endl << endl;*/
		}
		//no bomb, single fruit grid ,short line
		else if (flag1 && !flag2) {
			temp.push_back(p1);
			p2.x = p1.x + grid_x / 2;
			p2.y = p1.y;
			temp.push_back(p2);
			strategy.push_back(temp);
			/*cout << p1.x << "," << p1.y << endl;
			cout << p2.x << "," << p2.y << endl << endl;*/
		}
	}
	return;
}
