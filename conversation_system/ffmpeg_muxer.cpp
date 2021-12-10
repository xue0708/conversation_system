/***********************************************************************
File：FFmpeg_Muxer.cpp
Description：利用FFmpeg完成对音频的采集，与FFmpeg_Capture中合并的图片进行封装，输出mp4文件
***********************************************************************/
#include "ffmpeg_muxer.h"

/***********************************************************************
Function：MyThreadFunction
Description：开启线程，实时监听是否有回车键按下，按下停止mp4文件的录制
Input：null
Output：null
***********************************************************************/
DWORD WINAPI MyThreadFunction(LPVOID lpParam)
{
	while ((getchar()) != '\n')
	{
	
	}
	printf("audio end\n");
	exit_thread = 1;
	return 0;
}

/***********************************************************************
Function：FFmpeg_Init
Description：FFmpeg初始化
Input：null
Output：null
***********************************************************************/
void FFmpeg_Init()
{
	av_register_all();
	avcodec_register_all();
	avformat_network_init();
	avdevice_register_all();
}

/***********************************************************************
Function：Open_Audio
Description：利用FFmpeg打开音频设备
Input：inputfile：输入的音频设备的名称
Output：null
***********************************************************************/
int Open_Audio(const char* inputfile)
{
	ifmt = av_find_input_format("dshow");
	AVDictionary *device_param = nullptr;
	av_dict_set_int(&device_param, "rtbufsize", 18432000, 0);

	ret = avformat_open_input(&ifmt_ctx_a, inputfile, ifmt, &device_param);
	if (ret != 0)
	{
		printf("Couldn't open input audio stream.\n");
		return -1;
	}
	ret = avformat_find_stream_info(ifmt_ctx_a, NULL);
	if (ret < 0)
	{
		printf("Couldn't find audio stream information.\n");
		return -1;
	}
	audioindex = -1;
	for (int i = 0; i < ifmt_ctx_a->nb_streams; i++)
	{
		if (ifmt_ctx_a->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioindex = i;
			break;
		}
	}
	if (audioindex == -1)
	{
		printf("Couldn't find a audio stream.\n");
		return -1;
	}
	ret = avcodec_open2(ifmt_ctx_a->streams[audioindex]->codec, avcodec_find_decoder(ifmt_ctx_a->streams[audioindex]->codec->codec_id), NULL);
	if (ret < 0)
	{
		printf("Could not open audio codec.\n");
		return -1;
	}

	return 0;
}

/***********************************************************************
Function：Open_Output
Description：对输出的mp4做一些参数上的配置，输出的视频保存两路
Input：out_file1、out_file2：视频的两个保存路径
Output：null
***********************************************************************/
void Open_Output(const char* out_file1, const char* out_file2)
{
	fps_den = GetPrivateProfileInt(TEXT("video"), TEXT("FPS_den"), -1, TEXT("..\\video_parameter.ini"));
	fps_num = GetPrivateProfileInt(TEXT("video"), TEXT("FPS_num"), -1, TEXT("..\\video_parameter.ini"));
	b_rate = GetPrivateProfileInt(TEXT("video"), TEXT("bit_rate"), -1, TEXT("..\\video_parameter.ini"));
	gop = GetPrivateProfileInt(TEXT("video"), TEXT("gop_size"), -1, TEXT("..\\video_parameter.ini"));
	audio_rate = GetPrivateProfileInt(TEXT("audio"), TEXT("bit_rate"), -1, TEXT("..\\video_parameter.ini"));

	avformat_alloc_output_context2(&ofmt_ctx, NULL, "mp4", out_file1);
	avformat_alloc_output_context2(&ofmt_ctx2, NULL, "mp4", out_file2);

	//视频参数
	pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!pCodec)
	{
		printf("Can not find output video encoder!\n");
	}
	pCodecCtx = avcodec_alloc_context3(pCodec);
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	pCodecCtx->width = input_width;
	pCodecCtx->height = input_height;
	pCodecCtx->time_base.num = fps_num;
	pCodecCtx->time_base.den = fps_den;
	pCodecCtx->bit_rate = b_rate;
	pCodecCtx->gop_size = gop;
	if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
	{
		pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}
	if (ofmt_ctx2->oformat->flags & AVFMT_GLOBALHEADER)
	{
		pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}
	pCodecCtx->qmin = 10;
	pCodecCtx->qmax = 51;
	pCodecCtx->max_b_frames = 0;
	AVDictionary *param = 0;
	av_dict_set(&param, "preset", "medium", 0);
	av_dict_set(&param, "tune", "zerolatency", 0);
	av_dict_set(&param, "profile", "main", 0);
	avcodec_open2(pCodecCtx, pCodec, &param);

	//音频参数
	pCodec_a = avcodec_find_encoder(AV_CODEC_ID_AAC);
	if (!pCodec_a)
	{
		printf("Can not find output audio encoder!\n");
	}
	pCodecCtx_a = avcodec_alloc_context3(pCodec_a);
	pCodecCtx_a->channels = 2;
	pCodecCtx_a->channel_layout = av_get_default_channel_layout(2);
	pCodecCtx_a->sample_rate = ifmt_ctx_a->streams[audioindex]->codec->sample_rate;
	pCodecCtx_a->sample_fmt = pCodec_a->sample_fmts[0];
	pCodecCtx_a->bit_rate = audio_rate;
	pCodecCtx_a->time_base.num = 1;
	pCodecCtx_a->time_base.den = pCodecCtx_a->sample_rate;
	pCodecCtx_a->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
	if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
	{
		pCodecCtx_a->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}
	if (ofmt_ctx2->oformat->flags & AVFMT_GLOBALHEADER)
	{
		pCodecCtx_a->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}
	ret = avcodec_open2(pCodecCtx_a, pCodec_a, NULL);
	if (ret < 0)
	{
		printf("Failed to open ouput audio encoder!\n");
	}

	pCodecParserCtx = av_parser_init(AV_CODEC_ID_H264);
	if (!pCodecParserCtx)
	{
		printf("Could not allocate video parser context\n");
	}

	//添加视频流1
	video_st = avformat_new_stream(ofmt_ctx, pCodec);
	video_st->time_base.num = fps_num;
	video_st->time_base.den = fps_den;
	video_st->codec = pCodecCtx;
	video_st->codec->codec_id = AV_CODEC_ID_H264;
	//添加视频流2
	video_st2 = avformat_new_stream(ofmt_ctx2, pCodec);
	video_st2->time_base.num = fps_num;
	video_st2->time_base.den = fps_den;
	video_st2->codec = pCodecCtx;
	video_st2->codec->codec_id = AV_CODEC_ID_H264;

	//添加音频流1
	audio_st = avformat_new_stream(ofmt_ctx, pCodec_a);
	audio_st->time_base.num = 1;
	audio_st->time_base.den = pCodecCtx_a->sample_rate;
	audio_st->codec = pCodecCtx_a;
	//添加音频流2
	audio_st2 = avformat_new_stream(ofmt_ctx2, pCodec_a);
	audio_st2->time_base.num = 1;
	audio_st2->time_base.den = pCodecCtx_a->sample_rate;
	audio_st2->codec = pCodecCtx_a;

	ret = avio_open(&ofmt_ctx->pb, out_file1, AVIO_FLAG_READ_WRITE);
	if (ret < 0)
	{
		printf("Failed to open output_file1!\n");
	}
	ret = avio_open(&ofmt_ctx2->pb, out_file2, AVIO_FLAG_READ_WRITE);
	if (ret < 0)
	{
		printf("Failed to open output_file2!\n");
	}
	av_dump_format(ofmt_ctx, 0, out_file1, 1);
	av_dump_format(ofmt_ctx2, 0, out_file2, 1);

	//写入输出文件的头信息
	avformat_write_header(ofmt_ctx, NULL);
	avformat_write_header(ofmt_ctx2, NULL);

	//音频重采样
	aud_convert_ctx = swr_alloc_set_opts(NULL, av_get_default_channel_layout(pCodecCtx_a->channels), pCodecCtx_a->sample_fmt, pCodecCtx_a->sample_rate, av_get_default_channel_layout(ifmt_ctx_a->streams[audioindex]->codec->channels), ifmt_ctx_a->streams[audioindex]->codec->sample_fmt, ifmt_ctx_a->streams[audioindex]->codec->sample_rate, 0, NULL);
	swr_init(aud_convert_ctx);

	//音频缓冲设置
	fifo = av_audio_fifo_alloc(pCodecCtx_a->sample_fmt, pCodecCtx_a->channels, 1);
	if (!(converted_input_samples = (uint8_t**)calloc(pCodecCtx_a->channels, sizeof(**converted_input_samples))))
	{
		printf("Could not allocate converted input sample pointers\n");
	}

	//输入数据配置
	picture = av_frame_alloc();
	picture->width = pCodecCtx->width;
	picture->height = pCodecCtx->height;
	picture->format = pCodecCtx->pix_fmt;
	size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
	picture_buf = (uint8_t*)av_malloc(size);
	avpicture_fill((AVPicture*)picture, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

	int y_size = input_width * input_height;
	picture->data[0] = picture_buf;
	picture->data[1] = picture_buf + y_size;
	picture->data[2] = picture_buf + y_size * 5 / 4;
}

/***********************************************************************
Function：MP4_Save
Description：音视频数据的时间戳设置，封装后的数据送入队列
Input：null
Output：null
***********************************************************************/
DWORD WINAPI MP4_Save(LPVOID lparam)
{
	CaptureStore = 1;
	result_link_type* result_link = (result_link_type*)lparam;
	struct result_node_datatype *result_node2 = NULL;

	result_link_type *result_link2 = new result_link_type;
	result_link2->head = result_link2->end = NULL;
	result_link2->result_num = 0;
	struct result_node_datatype *result_node3 = NULL;

	HANDLE hThread = CreateThread(NULL, 0, MyThreadFunction, NULL, 0, NULL);
	HANDLE hThread2 = CreateThread(NULL, 0, MP4_record, (LPVOID)result_link2, 0, NULL);

	int ret;
	int framecnt = 0;
	int nb_samples = 0;
	int enc_got_frame;
	int dec_got_frame_a, enc_got_frame_a;
	int encode_video = 1, encode_audio = 1;
	int64_t aud_next_pts = 0;
	int64_t vid_next_pts = 0;

	GetPrivateProfileString(TEXT("OUTPUT"), TEXT("Output_path1"), TEXT(""), Path1, 100, TEXT("..\\video_parameter.ini"));
	GetPrivateProfileString(TEXT("OUTPUT"), TEXT("Output_path2"), TEXT(""), Path2, 100, TEXT("..\\video_parameter.ini"));

	FFmpeg_Init();

	Open_Audio(device_name_a);
	Open_Output(Path1, Path2);

	int64_t start_time = av_gettime();
	while (encode_video || encode_audio)
	{
		if (exit_all == 1)
		{
			break;
		}
		if (Full_flag == 0)
		{
			break;
		}

		if (encode_video && (!encode_audio || av_compare_ts(vid_next_pts, time_base_q, aud_next_pts, time_base_q) <= 0))
		{			
			result_node2 = result_pop(result_link);
			if (result_node2 != NULL)
			{
				memcpy(picture_buf, result_node2->result, result_node2->size);
				delete[] result_node2->result;
			}

			enc_pkt.data = NULL;
			enc_pkt.size = 0;
			av_init_packet(&enc_pkt);

			avcodec_encode_video2(pCodecCtx, &enc_pkt, picture, &enc_got_frame);
			if (enc_got_frame == 1)
			{
				framecnt++;
				enc_pkt.stream_index = video_st->index;

				AVRational time_base = ofmt_ctx->streams[0]->time_base;
				AVRational r_framerate1;
				r_framerate1.num = 10000000;
				r_framerate1.den = 10000000 / fps_den;
				int64_t calc_duration = (double)(AV_TIME_BASE)*(1 / av_q2d(r_framerate1));

				enc_pkt.pts = av_rescale_q(framecnt*calc_duration, time_base_q, time_base);
				enc_pkt.dts = enc_pkt.pts;
				enc_pkt.duration = av_rescale_q(calc_duration, time_base_q, time_base);
				enc_pkt.pos = -1;
				vid_next_pts = framecnt * calc_duration;

				int64_t pts_time = av_rescale_q(enc_pkt.pts, time_base, time_base_q);
				int64_t now_time = av_gettime() - start_time;
				if ((pts_time > now_time) && ((vid_next_pts + pts_time - now_time) < aud_next_pts))
				{
					av_usleep(pts_time - now_time);
				}

				result_node3 = new struct result_node_datatype;
				av_copy_packet(&result_node3->packet, &enc_pkt);
				result_push(result_link2, result_node3);

				av_free_packet(&enc_pkt);	
			}
		}
		else
		{
			const int output_frame_size = pCodecCtx_a->frame_size;
			AVFrame *input_frame;
			while (av_audio_fifo_size(fifo) < output_frame_size)
			{
				AVPacket input_packet;
				av_init_packet(&input_packet);
				input_packet.data = NULL;
				input_packet.size = 0;
				
				av_read_frame(ifmt_ctx_a, &input_packet);

				input_frame = av_frame_alloc();
				if (!input_frame)
				{
					ret = AVERROR(ENOMEM);
					return ret;
				}

				avcodec_decode_audio4(ifmt_ctx_a->streams[audioindex]->codec, input_frame, &dec_got_frame_a, &input_packet);
				av_packet_unref(&input_packet);

				if (dec_got_frame_a)
				{
					if ((ret = av_samples_alloc(converted_input_samples, NULL, pCodecCtx_a->channels, input_frame->nb_samples, pCodecCtx_a->sample_fmt, 0)) < 0)
					{
						printf("Could not allocate converted input samples\n");
						av_freep(&(*converted_input_samples)[0]);
						free(*converted_input_samples);
						return ret;
					}
					if ((ret = swr_convert(aud_convert_ctx, converted_input_samples, input_frame->nb_samples, (const uint8_t**)input_frame->extended_data, input_frame->nb_samples)) < 0)
					{
						printf("Could not convert input samples\n");
						return ret;
					}
					if ((ret = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + input_frame->nb_samples)) < 0) 
					{
						printf("Could not reallocate FIFO\n");
						return ret;
					}
					if (av_audio_fifo_write(fifo, (void **)converted_input_samples, input_frame->nb_samples) < input_frame->nb_samples)
					{
						printf("Could not write data to FIFO\n");
						return AVERROR_EXIT;
					}
				}
				if (converted_input_samples)
				{
					av_freep(&converted_input_samples[0]);
				}
				av_frame_free(&input_frame);
			}
			if (av_audio_fifo_size(fifo) >= output_frame_size)
			{
				AVFrame *output_frame = av_frame_alloc();
				if (!output_frame)
				{
					ret = AVERROR(ENOMEM);
					return ret;
				}

				const int frame_size = FFMIN(av_audio_fifo_size(fifo), pCodecCtx_a->frame_size);
				output_frame->nb_samples = frame_size;
				output_frame->channel_layout = pCodecCtx_a->channel_layout;
				output_frame->format = pCodecCtx_a->sample_fmt;
				output_frame->sample_rate = pCodecCtx_a->sample_rate;

				if ((ret = av_frame_get_buffer(output_frame, 0)) < 0)
				{
					printf("Could not allocate output frame samples\n");
					av_frame_free(&output_frame);
					return ret;
				}
				if (av_audio_fifo_read(fifo, (void **)output_frame->data, frame_size) < frame_size)
				{
					printf("Could not read data from FIFO\n");
					return AVERROR_EXIT;
				}

				AVPacket output_packet;
				AVPacket output_packet2;
				av_init_packet(&output_packet);
				output_packet.data = NULL;
				output_packet.size = 0;

				if (output_frame)
				{
					nb_samples += output_frame->nb_samples;
				}

				if ((ret = avcodec_encode_audio2(pCodecCtx_a, &output_packet, output_frame, &enc_got_frame_a)) < 0)
				{
					printf("Could not encode frame\n");
					av_packet_unref(&output_packet);
					return ret;
				}
				if (enc_got_frame_a)
				{
					output_packet.stream_index = 1;

					AVRational time_base = ofmt_ctx->streams[1]->time_base;
					AVRational r_framerate1 = { ifmt_ctx_a->streams[audioindex]->codec->sample_rate, 1 };
					int64_t calc_duration = (double)(AV_TIME_BASE)*(1 / av_q2d(r_framerate1));

					output_packet.pts = av_rescale_q(nb_samples*calc_duration, time_base_q, time_base);
					output_packet.dts = output_packet.pts;
					output_packet.duration = output_frame->nb_samples;

					aud_next_pts = nb_samples*calc_duration;

					int64_t pts_time = av_rescale_q(output_packet.pts, time_base, time_base_q);
					int64_t now_time = av_gettime() - start_time;
					if ((pts_time > now_time) && ((aud_next_pts + pts_time - now_time) < vid_next_pts))
					{
						av_usleep(pts_time - now_time);
					}

					result_node3 = new struct result_node_datatype;
					av_copy_packet(&result_node3->packet, &output_packet);				
					result_push(result_link2, result_node3);

					av_free_packet(&output_packet);					
				}
				av_frame_free(&output_frame);
			}
		}
	}

	CloseHandle(hThread);
	CloseHandle(hThread2);

	stop_flag = 1;
	return 0;
}

/***********************************************************************
Function：MP4_record
Description：从队列中取出封装好的音视频数据，以mp4格式存储在本地
Input：null
Output：null
***********************************************************************/
DWORD WINAPI MP4_record(LPVOID lparam)
{
	result_link_type* result_link2 = (result_link_type*)lparam;
	struct result_node_datatype *result_node3 = NULL;

	while (1)
	{
		if (exit_thread)
		{
			exit_all = 1;
			break;
		}
		result_node3 = result_pop(result_link2);
		if (result_node3 != NULL)
		{
			av_copy_packet(&packet2, &result_node3->packet);
			av_interleaved_write_frame(ofmt_ctx, &result_node3->packet);
			av_interleaved_write_frame(ofmt_ctx2, &packet2);

			av_free_packet(&result_node3->packet);
			av_free_packet(&packet2);
			delete[] result_node3;
		}
	}
	//写入输出文件的尾信息	
	av_write_trailer(ofmt_ctx);
	av_write_trailer(ofmt_ctx2);

	return 0;
}
