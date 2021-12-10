/***********************************************************************
File：Output_Play.h
Description：Output_Play.cpp里需要用到的变量定义
***********************************************************************/
#include "ffmpeg_capture.h"
#include "header.h"
#include "queue.h"

using namespace cv;
using namespace std;

#define CAM_NUM 5
#define FRAME_NUM 5

result_link_type *result_link;
result_link_type *NetRLink1;
result_link_type *NetRLink2;

DEFINE_GUID(CLSID_SystemDeviceEnum, 0x62be5d10, 0x60eb, 0x11d0, 0xbd, 0x3b, 0x00, 0xa0, 0xc9, 0x11, 0xce, 0x86);
DEFINE_GUID(CLSID_VideoInputDeviceCategory, 0x860bb310, 0x5d01, 0x11d0, 0xbd, 0x3b, 0x00, 0xa0, 0xc9, 0x11, 0xce, 0x86);
DEFINE_GUID(IID_ICreateDevEnum, 0x29840822, 0x5b84, 0x11d0, 0xbd, 0x3b, 0x00, 0xa0, 0xc9, 0x11, 0xce, 0x86);
extern DWORD WINAPI MP4_Save(LPVOID lparam);

DWORD64 qwFreeByters, qwFreeBytesToCaller, qwTotalByters;
VideoCapture capture[CAM_NUM];
Mat frame[FRAME_NUM];
Mat pCvMat1;
Mat pCvMat2;

int IPC_Width;
int IPC_Height;
int device_num;
int stop_flag = 0;
int mouse_flag = 0;
int thread_flag = 0;
int CaptureStore = 0;
int Full_flag = 1;
int FristRecive = 1;
int input_width, input_height;
int yuv_bufLen;
int camera_num = 0;

LPTSTR video1_ip = new char[100];
LPTSTR video2_ip = new char[100];
LPTSTR video1_pwd = new char[100];
LPTSTR video2_pwd = new char[100];
LPTSTR username = new char[100];
extern char device_name_a[1024];