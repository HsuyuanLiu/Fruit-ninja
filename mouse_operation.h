#pragma once

#include<Windows.h>
#include<iostream>
#include<atlimage.h>
#include"common.h"
using namespace std;
using namespace cv;
#pragma comment (lib, "User32.lib")

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)
#define KEY_ON(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 0 : 1)


float getDPI();
RECT getGameRegion();


static RECT fenbianlv;
static void getFenBianLv()//get DPI
{
	HDC hdc = GetDC(NULL);

	int hdcWidth, hdcHeight;

	hdcWidth = GetDeviceCaps(hdc, DESKTOPHORZRES);

	hdcHeight = GetDeviceCaps(hdc, DESKTOPVERTRES);

	fenbianlv.left = 0;
	fenbianlv.top = 0;
	fenbianlv.bottom = hdcHeight;
	fenbianlv.right = hdcWidth;

	ReleaseDC(NULL, hdc);
}

void make_lbutton_down(POINT pt);//press down left button of mouse

void make_lbutton_on(POINT pt);//loose left button of mouse

void mov_mouse(POINT& pt1, POINT pt2);

void mov_mouse(POINT pt);//mouse movement

Mat screen_cut(RECT rect);//screen cut

void one_cut(int x_offset, int y_offset, POINT pt1, POINT pt2);
void multi_cut(int x_offset, int y_offset, POINT* points, int num);//get the points, generate multiple lines
