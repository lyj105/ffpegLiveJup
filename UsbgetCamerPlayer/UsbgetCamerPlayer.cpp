// UsbgetCamerPlayer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//



/**
 * 最简单的基于FFmpeg的AVDevice例子（读取摄像头）
 * Simplest FFmpeg Device (Read Camera)
 *
 * 本程序实现了本地摄像头数据的获取解码和显示。是基于FFmpeg
 * 的libavdevice类库最简单的例子。通过该例子，可以学习FFmpeg中
 * libavdevice类库的使用方法。
 * 本程序在Windows下可以使用2种方式读取摄像头数据：
 *  1.VFW: Video for Windows 屏幕捕捉设备。注意输入URL是设备的序号，
 *          从0至9。
 *  2.dshow: 使用Directshow。注意作者机器上的摄像头设备名称是
 *         “Integrated Camera”，使用的时候需要改成自己电脑上摄像头设
 *          备的名称。
 * 在Linux下可以使用video4linux2读取摄像头设备。
 * 在MacOS下可以使用avfoundation读取摄像头设备。
 *
 * This software read data from Computer's Camera and play it.
 * It's the simplest example about usage of FFmpeg's libavdevice Library.
 * It's suiltable for the beginner of FFmpeg.
 * This software support 2 methods to read camera in Microsoft Windows:
 *  1.gdigrab: VfW (Video for Windows) capture input device.
 *             The filename passed as input is the capture driver number,
 *             ranging from 0 to 9.
 *  2.dshow: Use Directshow. Camera's name in author's computer is
 *             "Integrated Camera".
 * It use video4linux2 to read Camera in Linux.
 * It use avfoundation to read Camera in MacOS.
 *
 */
#include "pch.h"
#include <iostream>

#include <stdio.h>  
#define __STDC_CONSTANT_MACROS  
 //Windows  

//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "SDL/SDL.h"
};

#include "includeLib/ImportLib.h"

void show_dshow_device();
void show_dshow_device_option();


int main()
{
	AVFormatContext	*pFormatCtx;
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;

	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	//Register Device
	avdevice_register_all();

	//Show Dshow Device
	show_dshow_device();
	//Show Device Options
	show_dshow_device_option();

    std::cout << "Hello World!\n"; 
}


void show_dshow_device() {
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVDictionary* options = NULL;
	av_dict_set(&options, "list_devices", "true", 0);
	AVInputFormat *iformat = av_find_input_format("dshow");
	printf("========Device Info=============\n");
	avformat_open_input(&pFormatCtx, "video=dummy", iformat, &options);
	printf("================================\n");
}


//Show Dshow Device Option
void show_dshow_device_option() {
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVDictionary* options = NULL;
	av_dict_set(&options, "list_options", "true", 0);
	AVInputFormat *iformat = av_find_input_format("dshow");
	printf("========Device Option Info======\n");
	//avformat_open_input(&pFormatCtx,"video=Integrated Camera",iformat,&options);
	//avformat_open_input(&pFormatCtx, "video=USB2.0 PC CAMERA", iformat, &options);
	avformat_open_input(&pFormatCtx, "video=BudeBuai-6Plus", iformat, &options);
	//avformat_open_input(&pFormatCtx, "video=USB2.0 HD UVC WebCam", iformat, &options);
	printf("================================\n");
}