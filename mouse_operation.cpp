#include "mouse_operation.h"
static int numofpic = 0;
float getDPI()
{
    HWND hwnd = GetDesktopWindow();//to get the window
    HDC hdc = GetWindowDC(hwnd);//handles of get the window
    int zoom = GetDpiForWindow(hwnd);//to get DOI
    //cout << zoom << endl;
    float dpi = 0.0;
    //SET THE DPIhttps://blog.csdn.net/wzxiaodu/article/details/122338649*/
    switch (zoom)
    {
    case 96:
        dpi = 1;
        break;
    case 120:
        dpi = 1.25;
        break;
    case 144:
        dpi = 1.5;
        break;
    case 192:
        dpi = 2;
        break;
    default:
        dpi = -1;
        break;
    }
    ReleaseDC(hwnd, hdc);
    return dpi;
}

RECT getGameRegion()
{
    RECT testRect;
    testRect.left = 0;
    testRect.right = 0;
    testRect.top = 0;
    testRect.bottom = 0;

    /*ON WIN10*/
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    SetConsoleMode(hStdin, mode);
    /*https://blog.csdn.net/wangxun20081008/article/details/113915805*/

    HWND hwnd = GetDesktopWindow();
    HDC hdc = GetWindowDC(hwnd);
    int zoom = GetDpiForWindow(hwnd);
    
    float dpi = 0.0;

    switch (zoom)
    {
    case 96:
        dpi = 1;
        break;
    case 120:
        dpi = 1.25;
        break;
    case 144:
        dpi = 1.5;
        break;
    case 192:
        dpi = 2;
        break;
    default:
        dpi = -1;
        break;
    }

    POINT originPos;
    POINT curPos;

    while (KEY_ON(VK_LBUTTON)) {
        //cout << "test";
        Sleep(10);
    }
    if (KEY_DOWN(VK_LBUTTON)) {
        GetCursorPos(&originPos);
        ScreenToClient(hwnd, &originPos);
        while (KEY_DOWN(VK_LBUTTON))
        {
            DrawFocusRect(hdc, &testRect);
            GetCursorPos(&curPos);
            ScreenToClient(hwnd, &curPos);
            testRect.left = originPos.x * dpi;
            testRect.top = originPos.y * dpi;
            testRect.right = curPos.x * dpi;
            testRect.bottom = curPos.y * dpi;
            DrawFocusRect(hdc, &testRect);
            Sleep(10);
        }
        //Sleep(10000);
    }
    
    ReleaseDC(hwnd, hdc);
    cout << testRect.left << "," << testRect.top << "," << testRect.right << "," << testRect.bottom << endl;
    return testRect;
}




/* MOUSE OPERATION*/

void make_lbutton_down(POINT pt)
{
    float dpi = getDPI();
    //SetCursorPos(pt.x / dpi, pt.y / dpi);
    ::mouse_event(MOUSEEVENTF_LEFTDOWN, pt.x / dpi, pt.y / dpi, 0, 0);
}

void make_lbutton_on(POINT pt)
{
    float dpi = getDPI();
    //SetCursorPos(pt.x / dpi, pt.y / dpi);
    ::mouse_event(MOUSEEVENTF_LEFTUP, pt.x / dpi, pt.y / dpi, 0, 0);
}

void mov_mouse(POINT& pt1, POINT pt2)
{
    float dpi = getDPI();
    GetCursorPos(&pt1);
    SetCursorPos(pt2.x / dpi, pt2.y / dpi);
}

void mov_mouse(POINT pt)
{
    float dpi = getDPI();
    SetCursorPos(pt.x / dpi, pt.y / dpi);
}

Mat screen_cut(RECT rect)
{
    HWND hwnd = GetDesktopWindow();
    HDC hdc = GetWindowDC(hwnd);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    float dpi = getDPI();
    int bitOfPix = GetDeviceCaps(hdc, BITSPIXEL);
    CImage frame;
    frame.Create(width, height, bitOfPix);
    BitBlt(frame.GetDC(), 0, 0, width, height, hdc, rect.left, rect.top, SRCCOPY);
    //char path[100];
    //CString path;
    //path.Format(_T("gameresource / % d.png"), numofpic++);
    //frame.Save(path, Gdiplus::ImageFormatPNG);
    frame.Save(_T("1.png"), Gdiplus::ImageFormatPNG);

    
    Mat ret = imread(samples::findFile("1.png"), IMREAD_COLOR);

    frame.ReleaseDC();
    ReleaseDC(hwnd, hdc);

    return ret;
}

void one_cut(int x_offset, int y_offset, POINT pt1, POINT pt2)
{

    pt1.x += x_offset;
    pt1.y += y_offset;

    pt2.x += x_offset;
    pt2.y += y_offset;

    getFenBianLv();

    float dpi = getDPI();
    mov_mouse(pt1);
    make_lbutton_down(pt1);
    //Sleep(500);
    //mov_mouse(pt1);
    //mov_mouse(pt2);
    //cout << fenbianlv.right << " " << fenbianlv.bottom << endl;
    mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, pt2.x * (65535 / fenbianlv.right), pt2.y * (65535 / fenbianlv.bottom), 0, 0);
    Sleep(10);
    make_lbutton_on(pt2);
}

void multi_cut(int x_offset, int y_offset, POINT* points, int num)
{
    for (int i = 0;i < num;i++)
    {
        points[i].x += x_offset;
        points[i].y += y_offset;
    }

    mov_mouse(points[0]);
    make_lbutton_down(points[0]);
    //Sleep(500);
    //mov_mouse(pt1);
    for (int i = 1;i < num;i++)
    {
        mov_mouse(points[i]);
        Sleep(10);
    }
    //mov_mouse(pt2);
    //Sleep(10);
    make_lbutton_on(points[num - 1]);


}


