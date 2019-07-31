// BscoLivePush.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>

#include <iostream>
using namespace std;

///由于我们建立的是C++的工程
///编译的时候使用的C++的编译器编译
///而FFMPEG是C的库
///因此这里需要加上extern "C"
///否则会提示各种未定义
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
}
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "postproc.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "swscale.lib")

int main()
{
	//这里简单的输出一个版本号
	cout << "Hello FFmpeg!" << endl;
	av_register_all();
	unsigned version = avcodec_version();
	cout << "version is:" << version;
}
