#include"mouse_operation.h"
#include <iostream>
#include "SIFT_MATCH.h"
RECT gameRegion;

vector<Mat> srcMat;
Mat bomb;
Mat fruits;
Mat src1, src2,base;
const int feature_thresh = 25;
static int num = 0;

int loadFruitPic()
{
    string path = "gameresource\\*";
    string prePath = "gameresource/";
    long long handle;
    struct _finddata_t picFile;
    handle = _findfirst(path.c_str(), &picFile);
    if (handle == -1)
        return -1;
    do
    {
        //printf("%s\n", picFile.name);
        if (strcmp(picFile.name, ".") == 0 || strcmp(picFile.name, "..") == 0)
            continue;
        else
        {
            string filename(picFile.name);
            string nowPath = prePath + filename;
            Mat nowPic = imread(nowPath, IMREAD_COLOR);
            srcMat.push_back(nowPic);
        }

    } while (!_findnext(handle, &picFile));

    _findclose(handle);
    cout << "load successfully" << "!\n Num of resources" << srcMat.size()  << endl;
    return 0;
}

vector<KeyPoint> fruits_key1;//feature points of fruit
vector<KeyPoint> fruits_key2;//matched fruit points
vector<DMatch> fruits_match;

vector<KeyPoint> bomb_key1;//feature points of bomb
vector<KeyPoint> bomb_key2;//matched bomb points
vector<DMatch> bomb_match;

vector<KeyPoint> bomb_pts;//bomb key point after match
vector<KeyPoint> fruits_pts;//fruit key point after match
vector<POINT> points;//convert to cv.point
vector<POINT> bombs;//
vector<vector<POINT>> strategy;//Strategys

void filter(Mat base, Mat& cur, uchar val) {
    if (base.rows != cur.rows || base.cols != cur.cols) {
        exit(0);
    }
    Vec3b temp;
    temp[0] = val;
    temp[1] = val;
    temp[2] = val;
    //xor every single pixel
    for (int i = 0; i < base.rows; ++i) {
        for (int j = 0; j < base.cols; ++j) {
            if (base.at<Vec3b>(i, j) == cur.at<Vec3b>(i, j)) {
                cur.at<Vec3b>(i, j) = temp;
            }
        }
    }
    return;
}


int main()
{
    //get DPI
    getFenBianLv();
    cout << fenbianlv.right << " " << fenbianlv.bottom << endl;
    
    fruits = imread("gameresource/fruits.png", IMREAD_COLOR);
    bomb = imread("gameresource/bomb.png", IMREAD_COLOR);
    base = imread("gameresource/base.png", IMREAD_COLOR);
    
    Sleep(3000);
    cout << "Begining Frame" << endl;
    //cut the game region by mouse
    gameRegion = getGameRegion();



    /*gameRegion.left = 591;
    gameRegion.top = 452;
    gameRegion.right = 1390;
    gameRegion.bottom = 860;*/
    cout << gameRegion.left << "," << gameRegion.top << endl;

    Sleep(3000);

    //step into loop
    while (1)
    {
        //clear last round
        fruits_key1.clear();
        fruits_key2.clear();
        fruits_match.clear();
        fruits_pts.clear();

        bomb_key1.clear();
        bomb_key2.clear();
        bomb_match.clear();
        bomb_pts.clear();

        //screen_cut to get current image 
        Mat now_img = screen_cut(gameRegion);
        
        
        //filter(nowimg,nowimg,0);

        //match the feature points
        sift_match(fruits, now_img, fruits_key1, fruits_key2, fruits_match);
        sift_match(bomb, now_img, bomb_key1, bomb_key2, bomb_match);
        
        //store points
        for (int i = 0; i < fruits_match.size(); i++)
        {
            fruits_pts.push_back(fruits_key2[fruits_match[i].trainIdx]);
        }
        for (int i = 0; i < bomb_match.size(); i++)
        {
            bomb_pts.push_back(bomb_key2[bomb_match[i].trainIdx]);
        }

        
        //to cv.point
        for (int i = 0; i < fruits_pts.size(); i++)
        {
            POINT pt1, pt2;
            pt1.x = 50;
            pt1.y = 50;
            pt2.x = fruits_pts[i].pt.x;
            pt2.y = fruits_pts[i].pt.y;
            //one_cut(gameRegion.left, gameRegion.top, pt1, pt2);
            points.push_back(pt2);
        }
        for (int i = 0; i < bomb_pts.size(); ++i) {
            POINT pt1;
            pt1.x = bomb_pts[i].pt.x;
            pt1.y = bomb_pts[i].pt.y;
            bombs.push_back(pt1);
        }

        //size of the windows
        int size_x = 0, size_y = 0;
        //width,height of grid
        int grid_x = 30, grid_y = 30;
        //threshold for matching
        int threshold = 10;
 
        size_x = gameRegion.bottom - gameRegion.top;
        size_y = gameRegion.right - gameRegion.left;

        //strategy
        if (fruits_key2.size() >= feature_thresh)
        {
            //get the start/end
            getStrategy(points, bombs, strategy, size_x, size_y, grid_x, grid_y, threshold);
            //one cut function to cut
            for (int i = 0; i < strategy.size(); ++i) {
                //cout << strategy[i].size()<<endl;
                one_cut(gameRegion.left, gameRegion.top, strategy[i][0], strategy[i][1]);
            }

        }
        //set sleep time according to sampleing rates
        cout << "finished single round" << endl;
        waitKey(2000);
    }

   
    return 0;
}

