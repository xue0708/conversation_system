/***********************************************************************
File：Output_Play.cpp
Description：利用opencv完成对本地摄像头的采集，并接收到IPC的画面，组合在一起播放
***********************************************************************/
#include "output_play.h"

/***********************************************************************
Function：write_parameter
Description：将可以更改的参数写入到配置文件中
Input：null
Output：null
***********************************************************************/
void write_parameter()
{
	//摄像头数量
	WritePrivateProfileString(TEXT("CAMERA_NUM"), TEXT("number"), TEXT("1"), TEXT("..\\video_parameter.ini"));

	//IPC参数
	WritePrivateProfileString(TEXT("INPUT_VIDEO"), TEXT("video1_ip"), TEXT("10.11.0.5"), TEXT("..\\video_parameter.ini"));
	WritePrivateProfileString(TEXT("INPUT_VIDEO"), TEXT("video2_ip"), TEXT("10.11.0.6"), TEXT("..\\video_parameter.ini"));
	WritePrivateProfileString(TEXT("INPUT_VIDEO"), TEXT("video1_pwd"), TEXT("hk888888"), TEXT("..\\video_parameter.ini"));
	WritePrivateProfileString(TEXT("INPUT_VIDEO"), TEXT("video2_pwd"), TEXT("hk888888"), TEXT("..\\video_parameter.ini"));
	WritePrivateProfileString(TEXT("INPUT_VIDEO"), TEXT("username"), TEXT("admin"), TEXT("..\\video_parameter.ini"));
	WritePrivateProfileString(TEXT("INPUT_VIDEO"), TEXT("bit_type"), TEXT("sub"), TEXT("..\\video_parameter.ini"));

	//保存路径
	WritePrivateProfileString(TEXT("OUTPUT"), TEXT("Output_path1"), TEXT("result1.mp4"), TEXT("..\\video_parameter.ini"));
	WritePrivateProfileString(TEXT("OUTPUT"), TEXT("Output_path2"), TEXT("result2.mp4"), TEXT("..\\video_parameter.ini"));

	//音视频参数
	WritePrivateProfileString(TEXT("VIDEO"), TEXT("video_width"), TEXT("640"), TEXT("..\\video_parameter.ini"));
	WritePrivateProfileString(TEXT("VIDEO"), TEXT("video_height"), TEXT("480"), TEXT("..\\video_parameter.ini"));
	WritePrivateProfileString(TEXT("VIDEO"), TEXT("FPS_den"), TEXT("25"), TEXT("..\\video_parameter.ini"));
	WritePrivateProfileString(TEXT("VIDEO"), TEXT("FPS_num"), TEXT("1"), TEXT("..\\video_parameter.ini"));
	WritePrivateProfileString(TEXT("VIDEO"), TEXT("gop_size"), TEXT("100"), TEXT("..\\video_parameter.ini"));
	WritePrivateProfileString(TEXT("VIDEO"), TEXT("bit_rate"), TEXT("3000000"), TEXT("..\\video_parameter.ini"));
	WritePrivateProfileString(TEXT("AUDIO"), TEXT("bit_rate"), TEXT("30000"), TEXT("..\\video_parameter.ini"));
}

/***********************************************************************
Function：dup_wchar_to_utf8
Description：字符转换函数，转为UTF-8编码
Input：w：输入的字符串
Output：null
***********************************************************************/
char *dup_wchar_to_utf8(const wchar_t *w)
{
	char *s = NULL;
	int l = WideCharToMultiByte(CP_UTF8, 0, w, -1, 0, 0, 0, 0);
	s = (char *)av_malloc(l);
	if (s)
	{
		WideCharToMultiByte(CP_UTF8, 0, w, -1, s, l, 0, 0);
	}
	return s;
}

/***********************************************************************
Function：listDevices
Description：列出电脑上自带的视频设备
Input：list：string类型的数组，保存查询到的视频设备名称
Output：deviceCounter：视频设备的数目
***********************************************************************/
int listDevices(vector<string>& list)
{
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;
	int deviceCounter = 0;
	CoInitialize(NULL);

	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void**>(&pDevEnum));
	if (SUCCEEDED(hr))
	{
		hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
		if (hr == S_OK)
		{
			IMoniker *pMoniker = NULL;
			while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
			{
				IPropertyBag *pPropBag;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)(&pPropBag));
				if (FAILED(hr))
				{
					pMoniker->Release();
					continue;
				}

				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"Description", &varName, 0);
				if (FAILED(hr))
				{
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				}
				if (SUCCEEDED(hr))
				{
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);
					int count = 0;
					char tmp[255] = { 0 };
					while (varName.bstrVal[count] != 0x00 && count < 255)
					{
						tmp[count] = (char)varName.bstrVal[count];
						count++;
					}
					list.push_back(tmp);
				}

				pPropBag->Release();
				pPropBag = NULL;
				pMoniker->Release();
				pMoniker = NULL;
				deviceCounter++;
			}
			pDevEnum->Release();
			pDevEnum = NULL;
			pEnum->Release();
			pEnum = NULL;
		}
	}
	return deviceCounter;
}

/***********************************************************************
Function：EnumAudioDevice
Description：列出电脑上自带的音频设备
Input：null
Output：null
***********************************************************************/
char *EnumAudioDevice()
{
	UINT i = 0;
	UINT uDevNum = waveInGetNumDevs();

	if (uDevNum > 0)
	{
		WAVEINCAPSW	wic = { 0 };
		waveInGetDevCapsW(i, &wic, sizeof(wic));

		char *tmpStr = dup_wchar_to_utf8(wic.szPname);
		memset(device_name_a, '\0', 1024);
		sprintf(device_name_a, "audio=%s", tmpStr);
		printf("%s\n", device_name_a);
	}
	else
	{
		printf("Audio Device not Found\n");
	}
	return NULL;
}

/***********************************************************************
Function：Copy_Mode2
Description：输入两张图片组合在一张图片显示
Input：frame1、frame2：输入的图片；frame5：组合后的图片
Output：frame5
***********************************************************************/
void Copy_Mode2(Mat frame1, Mat frame2, Mat &frame5)
{
	Rect rect(input_width / 4 * 3, input_height / 4 * 3, input_width / 4, input_height / 4);
	frame1.copyTo(frame5);
	resize(frame2, frame5(rect), Size(input_width / 4, input_height / 4), (0, 0), (0, 0), INTER_LINEAR);
}

/***********************************************************************
Function：Copy_Mode3
Description：输入三张图片组合在一张图片显示
Input：frame1、frame2、frame3：输入的图片；frame5：组合后的图片
Output：frame5
***********************************************************************/
void Copy_Mode3(Mat frame1, Mat frame2, Mat frame3, Mat &frame5)
{
	frame1.copyTo(frame5);
	Rect rect1(input_width / 2, 0, input_width / 2, input_height / 2);
	Rect rect2(input_width / 2, input_height / 2, input_width / 2, input_height / 2);
	resize(frame2, frame5(rect1), Size(input_width / 2, input_height / 2), (0, 0), (0, 0), INTER_LINEAR);
	resize(frame3, frame5(rect2), Size(input_width / 2, input_height / 2), (0, 0), (0, 0), INTER_LINEAR);
}

/***********************************************************************
Function：Copy_Mode3
Description：输入四张图片组合在一张图片显示
Input：frame1、frame2、frame3、frame4：输入的图片；frame5：组合后的图片
Output：frame5
***********************************************************************/
void Copy_Mode4(Mat frame1, Mat frame2, Mat frame3, Mat frame4, Mat &frame5)
{
	frame1.copyTo(frame5);
	Rect rect1(0, 0, input_width / 2, input_height / 2);
	Rect rect2(input_width / 2, 0, input_width / 2, input_height / 2);
	Rect rect3(0, input_height / 2, input_width / 2, input_height / 2);
	Rect rect4(input_width / 2, input_height / 2, input_width / 2, input_height / 2);
	resize(frame1, frame5(rect1), Size(input_width / 2, input_height / 2), (0, 0), (0, 0), INTER_LINEAR);
	resize(frame2, frame5(rect2), Size(input_width / 2, input_height / 2), (0, 0), (0, 0), INTER_LINEAR);
	resize(frame3, frame5(rect3), Size(input_width / 2, input_height / 2), (0, 0), (0, 0), INTER_LINEAR);
	resize(frame4, frame5(rect4), Size(input_width / 2, input_height / 2), (0, 0), (0, 0), INTER_LINEAR);
}

/***********************************************************************
Function：on_mouse2
Description：在Copy_Mode2下，根据鼠标点击的范围和类型切换画面的播放
Input：event：事件类型；x、y：坐标位置
Output：null
***********************************************************************/
void on_mouse2(int event, int x, int y, int flags, void* ustc)
{
	if (event == CV_EVENT_LBUTTONDOWN && x > (input_width / 4 * 3) && x < input_width && y >(input_height / 4 * 3) && y < input_height)
	{
		mouse_flag ^= 1;
	}
}

/***********************************************************************
Function：on_mouse3
Description：在Copy_Mode3下，根据鼠标点击的范围和类型切换画面的播放
Input：event：事件类型；x、y：坐标位置
Output：null
***********************************************************************/
void on_mouse3(int event, int x, int y, int flags, void* ustc)
{
	if (event == CV_EVENT_RBUTTONDOWN)
	{
		mouse_flag = 0;
	}
	if (event == CV_EVENT_LBUTTONDOWN && x > 0 && x < (input_width / 2) && y > 0 && y < input_height && mouse_flag == 0)
	{
		mouse_flag = 1;
	}
	if (event == CV_EVENT_LBUTTONDOWN && x >(input_width / 2) && x < input_width && y > 0 && y < (input_height / 2) && mouse_flag == 0)
	{
		mouse_flag = 2;
	}
	if (event == CV_EVENT_LBUTTONDOWN && x >(input_width / 2) && x < input_width && y >(input_height / 2) && y < input_height && mouse_flag == 0)
	{
		mouse_flag = 3;
	}
}

/***********************************************************************
Function：on_mouse4
Description：在Copy_Mode4下，根据鼠标点击的范围和类型切换画面的播放
Input：event：事件类型；x、y：坐标位置
Output：null
***********************************************************************/
void on_mouse4(int event, int x, int y, int flags, void* ustc)
{
	if (event == CV_EVENT_RBUTTONDOWN)
	{
		mouse_flag = 0;
	}
	if (event == CV_EVENT_LBUTTONDOWN && x > 0 && x < (input_width / 2) && y > 0 && y < (input_height / 2) && mouse_flag == 0)
	{
		mouse_flag = 1;
	}
	if (event == CV_EVENT_LBUTTONDOWN && x >(input_width / 2) && x < input_width && y > 0 && y < (input_height / 2) && mouse_flag == 0)
	{
		mouse_flag = 2;
	}
	if (event == CV_EVENT_LBUTTONDOWN && x > 0 && x < (input_width / 2) && y >(input_height / 2) && y < input_height && mouse_flag == 0)
	{
		mouse_flag = 3;
	}
	if (event == CV_EVENT_LBUTTONDOWN && x >(input_width / 2) && x < input_width && y >(input_height / 2) && y < input_height && mouse_flag == 0)
	{
		mouse_flag = 4;
	}
}

/***********************************************************************
Function：Put_Time
Description：在图片上添加时间水印
Input：In_Frame：输入的需要添加时间水印的图片
Output：null
***********************************************************************/
void Put_Time(Mat In_Frame)
{
	Point point = Point(30, 50);
	struct tm t;
	time_t now_time;
	char ch[64];
	time(&now_time);
	localtime_s(&t, &now_time);

	int len = strftime(ch, sizeof(ch), "%Y-%m-%d %H:%M:%S", &t);
	ch[len] = '\0';
	putText(In_Frame, ch, point, FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 200, 200), 1, CV_AA);
}

/***********************************************************************
Function：MemoryCount
Description：指定磁盘，实时监测磁盘的剩余空间大小
Input：null
Output：null
***********************************************************************/
void MemoryCount()
{
	BOOL bResult = GetDiskFreeSpaceEx(TEXT("E:"), (PULARGE_INTEGER)&qwFreeBytesToCaller, (PULARGE_INTEGER)&qwTotalByters, (PULARGE_INTEGER)&qwFreeByters);
	if (bResult)
	{
		//cout << "Currently available disk free capacity：" << qwFreeByters << endl;
	}
}

/***********************************************************************
Function：run_get_data_thread
Description：线程函数，打开IPC1
Input：null
Output：null
***********************************************************************/
DWORD WINAPI run_get_data_thread(LPVOID lparam)
{
	ffmpeg_data data(*(ffmpeg_data*)lparam);
	data.ffmpeg_data_init();
	data.ffmpeg_data_collection(0);

	return NULL;
}

/***********************************************************************
Function：run_get_data_thread1
Description：线程函数，打开IPC2
Input：null
Output：null
***********************************************************************/
DWORD WINAPI run_get_data_thread1(LPVOID lparam)
{
	ffmpeg_data data(*(ffmpeg_data*)lparam);
	data.ffmpeg_data_init();
	data.ffmpeg_data_collection(1);

	return NULL;
}

/***********************************************************************
Function：PreviewPlayinit
Description：播放前初始化参数，确定播放画面的分辨率以及采集多少路画面
Input：null
Output：null
***********************************************************************/
int PreviewPlayinit()
{
	camera_num = GetPrivateProfileInt(TEXT("CAMERA_NUM"), TEXT("number"), -1, TEXT("..\\video_parameter.ini"));
	input_width = GetPrivateProfileInt(TEXT("VIDEO"), TEXT("Video_width"), -1, TEXT("..\\video_parameter.ini"));
	input_height = GetPrivateProfileInt(TEXT("VIDEO"), TEXT("Video_height"), -1, TEXT("..\\video_parameter.ini"));
	yuv_bufLen = input_width * input_height * 3 / 2;

	for (int i = 0; i < device_num; i++)
	{
		capture[i].open(i);
	}	

	for (int i = 0; i < device_num; i++)
	{
		capture[i].set(CV_CAP_PROP_FRAME_WIDTH, input_width);
		capture[i].set(CV_CAP_PROP_FRAME_HEIGHT, input_height);
	}

	result_link = new result_link_type;
	result_link->head = result_link->end = NULL;
	result_link->result_num = 0;
	printf("result_link  init \n");

	NetRLink1 = new result_link_type;
	NetRLink1->head = NetRLink1->end = NULL;
	NetRLink1->result_num = 0;
	printf("NetRLink1  init \n");

	NetRLink2 = new result_link_type;
	NetRLink2->head = NetRLink2->end = NULL;
	NetRLink2->result_num = 0;
	printf("NetRLink2  init \n");

	cvNamedWindow("result", 0);
	return 0;
}

/***********************************************************************
Function：PreviewPlay
Description：得到多路采集的画面，将其合并为一个画面显示，并开启线程，实现音频的采集，并录制
Input：null
Output：null
***********************************************************************/
int PreviewPlay()
{
	GetPrivateProfileString(TEXT("INPUT_VIDEO"), TEXT("username"), TEXT(""), username, 100, TEXT("..\\video_parameter.ini"));
	GetPrivateProfileString(TEXT("INPUT_VIDEO"), TEXT("video1_pwd"), TEXT(""), video1_pwd, 100, TEXT("..\\video_parameter.ini"));
	GetPrivateProfileString(TEXT("INPUT_VIDEO"), TEXT("video2_pwd"), TEXT(""), video2_pwd, 100, TEXT("..\\video_parameter.ini"));
	GetPrivateProfileString(TEXT("INPUT_VIDEO"), TEXT("video1_ip"), TEXT(""), video1_ip, 100, TEXT("..\\video_parameter.ini"));
	GetPrivateProfileString(TEXT("INPUT_VIDEO"), TEXT("video2_ip"), TEXT(""), video2_ip, 100, TEXT("..\\video_parameter.ini"));
	
	ffmpeg_data f2(true, username, video1_pwd, video1_ip);
	HANDLE collection_thread2;
	collection_thread2 = CreateThread(NULL, 0, run_get_data_thread, (LPVOID)&f2, 0, NULL);
	Sleep(500);
	ffmpeg_data f3(true, username, video2_pwd, video2_ip);
	HANDLE collection_thread3;
	collection_thread3 = CreateThread(NULL, 0, run_get_data_thread1, (LPVOID)&f3, 0, NULL);

	struct result_node_datatype *NetRLink1Node2 = NULL;
	struct result_node_datatype *NetRLink2Node2 = NULL;
	IPC_Width = input_width;
	IPC_Height = input_height;

	while (waitKey(1) != ' ')
	{
		MemoryCount();
		if (qwFreeByters < 50000)
		{
			Full_flag = 0;
		}

		for (int i = 0; i < device_num; i++)
		{
			capture[i] >> frame[i];
		}
		if (FristRecive)
		{
			pCvMat1.create(cv::Size(IPC_Width, IPC_Height), CV_8UC3);
			pCvMat2.create(cv::Size(IPC_Width, IPC_Height), CV_8UC3);
			FristRecive = 0;
		}
		NetRLink1Node2 = result_pop(NetRLink1);
		if (NetRLink1Node2 != NULL)
		{
			memcpy(pCvMat1.data, NetRLink1Node2->result, NetRLink1Node2->size);
			delete[] NetRLink1Node2->result;
		}
		NetRLink2Node2 = result_pop(NetRLink2);
		if (NetRLink2Node2 != NULL)
		{
			memcpy(pCvMat2.data, NetRLink2Node2->result, NetRLink2Node2->size);
			delete[] NetRLink2Node2->result;
		}
		if (camera_num == 1)
		{
			frame[0].copyTo(frame[4]);
		}
		else if (camera_num == 2)
		{
			setMouseCallback("result", on_mouse2, 0);
			switch (mouse_flag)
			{
			case 0:
				Copy_Mode2(frame[0], frame[1], frame[4]);
				break;
			case 1:
				Copy_Mode2(frame[1], frame[0], frame[4]);
				break;
			default:
				break;
			}
		}
		else if (camera_num == 3)
		{
			setMouseCallback("result", on_mouse3, 0);
			switch (mouse_flag)
			{
			case 0:
				Copy_Mode3(frame[0], frame[1], pCvMat1, frame[4]);
				break;
			case 1:
				frame[0].copyTo(frame[4]);
				break;
			case 2:
				frame[1].copyTo(frame[4]);
				break;
			case 3:
				pCvMat1.copyTo(frame[4]);
				break;
			default:
				break;
			}
		}
		else if (camera_num == 4)
		{
			setMouseCallback("result", on_mouse4, 0);
			switch (mouse_flag)
			{
			case 0:
				Copy_Mode4(frame[0], frame[1], pCvMat1, pCvMat2, frame[4]);
				break;
			case 1:
				frame[0].copyTo(frame[4]);
				break;
			case 2:
				frame[1].copyTo(frame[4]);
				break;
			case 3:
				pCvMat1.copyTo(frame[4]);
				break;
			case 4:
				pCvMat2.copyTo(frame[4]);
				break;
			default:
				break;
			}
		}
		else
		{
			cout << "Error!" << endl;
			break;
		}

		Put_Time(frame[4]);
		imshow("result", frame[4]);

		if (thread_flag == 0)
		{
			HANDLE thread1 = CreateThread(NULL, 0, MP4_Save, (LPVOID)result_link, 0, NULL);
			thread_flag++;
		}
		if (CaptureStore)
		{
			struct result_node_datatype *result_node = new struct result_node_datatype;
			result_node->result = new unsigned char[yuv_bufLen];
			memset(result_node->result, '\0', yuv_bufLen);

			Mat yuvImg;
			cvtColor(frame[4], yuvImg, CV_BGR2YUV_I420);
			memcpy(result_node->result, yuvImg.data, yuv_bufLen*sizeof(unsigned char));
			result_node->size = yuv_bufLen;
			result_push(result_link, result_node);

			cout << "result_link->result_num = " << result_link->result_num << endl;
		}
		if (stop_flag)
		{
			while (1)
			{
				struct result_node_datatype *result_node3 = result_pop(result_link);
				if (result_node3 != NULL)
				{
					delete[] result_node3->result;
					result_node3 = NULL;
				}
				if (result_link->head == result_link->end)
				{
					break;
				}
			}
			stop_flag = 0;
			CaptureStore = 0;
		}
	}
	for (int i = 0; i < device_num; i++)
	{
		capture[i].release();
	}
	cvDestroyWindow("result");
	return 0;
}

/***********************************************************************
Function：main
Description：主函数，调用接口，实现功能
Input：null
Output：null
***********************************************************************/
int main()
{	
	//write_parameter();

	EnumAudioDevice();

	vector<string> CameraName;
	device_num = listDevices(CameraName);
	if (device_num < 1)
	{
		cout << "Can't find USB Camera!" << endl;
		return 0;
	}
	cout << "device_num = " << device_num << endl;
	for (int i = 0; i < device_num; i++)
	{
		cout << "ID: " << i << " ; " << "Name: " << CameraName[i] << endl;
	}

	PreviewPlayinit();
	PreviewPlay();
	
	return 0;
}
