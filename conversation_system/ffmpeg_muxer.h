/***********************************************************************
File：FFmpeg_Muxer.h
Description：FFmpeg音视频封装时所需的参数
***********************************************************************/
#include "header.h"
#include "queue.h"

using namespace std;

extern int stop_flag;
extern int Full_flag;
extern int CaptureStore;
extern int input_width;
extern int input_height;

AVFormatContext *ifmt_ctx = NULL;
AVFormatContext *ifmt_ctx_a = NULL;
AVFormatContext *ofmt_ctx;
AVFormatContext *ofmt_ctx2;
AVInputFormat *ifmt;
AVStream *video_st;
AVStream *video_st2;
AVStream *audio_st;
AVStream *audio_st2;
AVCodecContext *pCodecCtx;
AVCodecContext *pCodecCtx_a;
AVCodec *pCodec;
AVCodec *pCodec_a;
AVPacket enc_pkt;
AVPacket enc_pkt2;
AVPacket *dec_pkt_a, enc_pkt_a;
AVPacket packet2;
AVFrame *picture;
AVFrame *pframe, *pFrameYUV;
struct SwrContext *aud_convert_ctx;
AVAudioFifo *fifo = NULL;
AVCodecParserContext *pCodecParserCtx = NULL;

AVRational time_base_q = { 1, AV_TIME_BASE };
uint8_t *picture_buf = NULL;
uint8_t **converted_input_samples = NULL;

int audioindex;
int videoindex;
int size;
int fps_den, fps_num;
int b_rate;
int gop;
int audio_rate;
int ret;

int exit_thread = 0;
int exit_all = 0;

LPTSTR Path1 = new char[100];
LPTSTR Path2 = new char[100];
char device_name_a[1024];

DWORD WINAPI MP4_record(LPVOID lparam);
