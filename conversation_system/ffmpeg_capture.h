/***********************************************************************
File：FFmpeg_Capture.h
Description：FFmpeg采集IPC网络流时所需的参数
***********************************************************************/
#ifndef FFMPEG_DATA_H
#define FFMPEG_DATA_H

#include <stdio.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <windows.h>
#include "queue.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

using namespace std;
using namespace cv;

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")	
#pragma comment(lib ,"swscale.lib")

#define ERROR 0
#define TRUE 1

/***********************************************************************
Function：ffmpeg_data
Description：网络流采集定义的类
***********************************************************************/
class ffmpeg_data
{
private:
	char *camUserName;
	char *camPassword;
	char *ipAddr; 
	bool netCamActiveFlag; 
	AVFormatContext *pFormatCtx;
	AVCodecContext  *pCodecCtx;
	int             numBytes;
	int             videoStream;

public:
	static void *callback(void *);
	HANDLE threadID;
	Mat pCvMat;

	ffmpeg_data()
	{
		this->camUserName = NULL;
		this->camPassword = NULL;
		this->ipAddr = NULL;
		this->netCamActiveFlag = false;
	}

	ffmpeg_data(bool activeFlag)
	{
		this->camUserName = NULL;
		this->camPassword = NULL;
		this->ipAddr = NULL;
		this->netCamActiveFlag = activeFlag;
	}

	ffmpeg_data(bool activeFlag, char *username, char *password, char *ip)
	{
		this->camUserName = new char[strlen(username) + 1];
		strcpy(this->camUserName, username);
		this->camPassword = new char[strlen(password) + 1];
		strcpy(this->camPassword, password);
		this->ipAddr = new char[strlen(ip) + 1];
		strcpy(this->ipAddr, ip);
		printf("this->ipAddr = %s\n", this->ipAddr);

		this->netCamActiveFlag = activeFlag;
	}

	ffmpeg_data(ffmpeg_data &ex_ff)
	{
		cout << "ex_ff.camUserName = " << ex_ff.camUserName << "ex_ff.camPassword = " << ex_ff.camPassword << endl;
		this->camUserName = new char[strlen(ex_ff.camUserName) + 1];
		strcpy(this->camUserName, ex_ff.camUserName);
		this->camPassword = new char[strlen(ex_ff.camPassword) + 1];
		strcpy(this->camPassword, ex_ff.camPassword);
		this->ipAddr = new char[strlen(ex_ff.ipAddr) + 1];
		strcpy(this->ipAddr, ex_ff.ipAddr);
		this->netCamActiveFlag = ex_ff.netCamActiveFlag;

		cout << "this->camUserName = " << this->camUserName << "this->camPassword  = " << this->camPassword << "this->camPassword  = " << this->ipAddr << endl;
	}

	~ffmpeg_data()
	{
		if (this->camUserName)
		{
			delete this->camUserName;
		}
		if (this->camPassword)
		{
			delete this->camPassword;
		}
		if (this->ipAddr)
		{
			delete this->ipAddr;
		}
	}
	int show(int flag);
	int ffmpeg_data_set_cam_active_flag(bool active);
	int ffmpeg_data_set_cam_info(char *username, char *password, char *ip); 
	int ffmpeg_data_init(); 
	int ffmpeg_data_collection(int flag); 
	int ffmpeg_data_discollection(); 
	int ffmpeg_data_recollection(); 
	int ffmpeg_data_release(); 
	bool ffmpeg_data_start_thread();
};

#endif