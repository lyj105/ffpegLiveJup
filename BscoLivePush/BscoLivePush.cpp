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


//..\ffpegLIB\include\
//..\ffpegLIB\lib

#include "VideoRetmp.h"

int main()
{
	//这里简单的输出一个版本号
	cout << "Hello FFmpeg!" << endl;
	av_register_all();
	unsigned version = avcodec_version();
	cout << "version is:" << version;

	printf("\n");

	BossLiveRept *rept = new BossLiveRept;

	rept->GetBossLivePushRetp();

	delete rept;



}
