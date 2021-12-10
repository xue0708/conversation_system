/***********************************************************************
File：FFmpeg_Capture.cpp
Description：利用opencv完成对本地摄像头的采集，并接收到IPC的画面，组合在一起播放
***********************************************************************/
#include "ffmpeg_capture.h"
#include <assert.h>

extern result_link_type *NetRLink1;
extern result_link_type *NetRLink2;
extern int IPC_Width;
extern int IPC_Height;

/***********************************************************************
Function：ffmpeg_data_set_cam_info
Description：获得IPC的用户名、密码、IP地址
Input：username：用户名；password：密码；ip：IP地址
Output：null
***********************************************************************/
int ffmpeg_data::ffmpeg_data_set_cam_info(char *username, char *password, char *ip)
{
	this->camUserName = new char[strlen(username) + 1];
	memcpy(this->camUserName, username, strlen(username));
	this->camPassword = new char[strlen(password) + 1];
	memcpy(this->camPassword, password, strlen(password));
	this->ipAddr = new char[strlen(ip) + 1];
	memcpy(this->ipAddr, ip, strlen(ip));

	return 0;
}

/***********************************************************************
Function：ffmpeg_data_set_cam_active_flag
Description：设置两个IPC的活跃状态，避免冲突
Input：active：状态
Output：null
***********************************************************************/
int ffmpeg_data::ffmpeg_data_set_cam_active_flag(bool active)
{
	this->netCamActiveFlag = active;

	return 0;
}

/***********************************************************************
Function：ffmpeg_data_init
Description：FFmpeg打开IP流前的初始化
Input：null
Output：null
***********************************************************************/
int ffmpeg_data::ffmpeg_data_init()
{
	if (!this->camUserName || !this->camPassword || !this->ipAddr)
	{
		cout << "ffmpeg info error!" << endl;
		return EAGAIN;
	}

	AVCodec *pCodec;
	AVDictionary *options = NULL;
	pFormatCtx = NULL;
	int i;

	char *filepath = new char[100];
	sprintf_s(filepath, 100, "rtsp://%s:%s@%s/h264/ch1/sub/av_stream", camUserName, camPassword, ipAddr);

	av_register_all();
	avformat_network_init();
	av_dict_set(&options, "rtsp_transport", "tcp", 0);
	if (avformat_open_input(&pFormatCtx, filepath, NULL, &options) != 0)
	{
		return -1;
	}		
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		return -1;
	}
	av_dump_format(pFormatCtx, 0, NULL, false);

	videoStream = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStream = i;
			break;
		}
	}	
	if (videoStream == -1)
	{
		return -1;
	}	
	pCodecCtx = pFormatCtx->streams[videoStream]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, 0) < 0)
	{
		return -1;
	}
	if (pCodecCtx->time_base.num > 1000 && pCodecCtx->time_base.den == 1)
	{
		pCodecCtx->time_base.den = 1000;
	}
		
	return 0;
}

/***********************************************************************
Function：ffmpeg_data_collection
Description：FFmpeg采集RTSP流，采集后的数据流解码为图片送到队列中
Input：flag：避免两个IPC同时采集时发生冲突，设置的标志
Output：null
***********************************************************************/
int ffmpeg_data::ffmpeg_data_collection(int flag)
{
	int             i = 0;
	uint8_t         *buffer = NULL;
	AVPacket        packet;
	bool            first_flag = true;

	AVFrame *pFrame = av_frame_alloc();
	AVFrame *pFrameRGB = av_frame_alloc();
	
	int ret, got_picture;
	static struct SwsContext *img_convert_ctx;
	double start, end;
	
	while (netCamActiveFlag)
	{	
		if (av_read_frame(pFormatCtx, &packet) >= 0)
		{
			if (packet.stream_index == videoStream)
			{
				ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
				IPC_Width = pCodecCtx->width;
				IPC_Height = pCodecCtx->height;
				if (ret < 0) 
				{
					printf("Decode Error.(解码错误)\n");
					return ret;
				}
				
				if (got_picture)
				{	
					if (first_flag)
					{
						img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
						if (img_convert_ctx == NULL)
						{
							fprintf(stderr, "Cannot initialize the conversion context!\n");
							exit(1);
						}
						numBytes = avpicture_get_size(AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height);
						buffer = (uint8_t *)av_malloc(numBytes);
						avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height);
					
						pCvMat.create(cv::Size(pCodecCtx->width, pCodecCtx->height), CV_8UC3);
						first_flag = false;
					}
	
					int ret = sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
					if (flag == 0)
					{
						if (NetRLink1->result_num < 10)
						{
							struct result_node_datatype *NetRLink1Node = new struct result_node_datatype;
							NetRLink1Node->result = new unsigned char[numBytes];
							memset(NetRLink1Node->result, '\0', numBytes);
							memcpy(NetRLink1Node->result, buffer, numBytes);
							NetRLink1Node->size = numBytes;
							result_push(NetRLink1, NetRLink1Node);							
						}						
					}
					else
					{
						if (NetRLink2->result_num < 10)
						{
							struct result_node_datatype *NetRLink2Node = new struct result_node_datatype;
							NetRLink2Node->result = new unsigned char[numBytes];
							memset(NetRLink2Node->result, '\0', numBytes);
							memcpy(NetRLink2Node->result, buffer, numBytes*sizeof(unsigned char));
							NetRLink2Node->size = numBytes;
							result_push(NetRLink2, NetRLink2Node);						
						}					
					}					
					waitKey(1);
				}				
			}			
			av_free_packet(&packet);
		}
	}
	av_free(pFrameRGB);	
	av_free(pFrame);
	return 0;
}

/***********************************************************************
Function：ffmpeg_data_release
Description：FFmpeg关闭时，释放申请的内存
Input：null
Output：null
***********************************************************************/
int ffmpeg_data::ffmpeg_data_release()
{
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
	return 0;
}

int ffmpeg_data::ffmpeg_data_recollection()
{
	return 0;
}

int ffmpeg_data::ffmpeg_data_discollection()
{
	return 0;
}

bool ffmpeg_data::ffmpeg_data_start_thread()
{
	return true;
}

void *ffmpeg_data::callback(void*)
{
	return NULL;
}