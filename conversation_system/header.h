/***********************************************************************
File：Header.h
Description：头文件，声明用到的头文件
***********************************************************************/
#include <conio.h>
#include <time.h>
#include <stdio.h>
#include <windows.h>
#include <initguid.h>
#include <vector>
#include <string>
#include <iostream>
#include "strmif.h"
#include "omp.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

extern "C"
{
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>
#include "libavutil/mathematics.h"
};

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "ddraw")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "postproc.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "avutil")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "OleAut32.lib")
#pragma comment(lib, "winmm.lib")
