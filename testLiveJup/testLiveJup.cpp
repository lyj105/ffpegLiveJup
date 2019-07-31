// testLiveJup.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

// ffpegLiveJup.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>

#include <stdio.h>
#include <string.h>

extern "C"
{

#include "libavformat/avformat.h"
#include "libavutil/dict.h"
};

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")


int main()
{
	AVFormatContext *pFormatCtx = NULL;
	AVCodecContext *pCodecCtx = NULL;
	AVCodec *pCodec;
	AVDictionaryEntry *dict = NULL;

	int iHour, iMinute, iSecond, iTotalSeconds;//HH:MM:SS
	int videoIndex, audioIndex;

	const char *fileName = "nwn.mp4";
	//char *fileName = "Titanic.ts";

	av_register_all();//注册所有组件

	if (avformat_open_input(&pFormatCtx, fileName, NULL, NULL) != 0)//打开输入视频文件
	{
		printf("Couldn't open input stream.\n");
		return -1;
	}

	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		printf("Couldn't find stream information.\n");
		return -1;
	}

	videoIndex = -1;
	for (int i = 0; i < pFormatCtx->nb_streams/*视音频流的个数*/; i++)
	{
		if (pFormatCtx->streams[i]/*视音频流*/->codec->codec_type == AVMEDIA_TYPE_VIDEO)//查找音频
		{
			videoIndex = i;
			break;
		}
	}
	if (videoIndex == -1)
	{
		printf("Couldn't find a video stream.\n");
		return -1;
	}

	pCodecCtx = pFormatCtx->streams[videoIndex]->codec;	//指向AVCodecContext的指针
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);	//指向AVCodec的指针.查找解码器
	if (pCodec == NULL)
	{
		printf("Codec not found.\n");
		return -1;
	}
	//打开解码器
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		printf("Could not open codec.\n");
		return -1;
	}

	audioIndex = -1;
	for (int i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioIndex = i;
			break;
		}
	}
	if (audioIndex == -1)
	{
		printf("Couldn't find a audio stream.\n");
		return -1;
	}



	//打印结构体信息

	puts("AVFormatContext信息：");
	puts("---------------------------------------------");
	printf("文件名：%s\n", pFormatCtx->filename);
	iTotalSeconds = (int)pFormatCtx->duration/*微秒*/ / 1000000;
	iHour = iTotalSeconds / 3600;//小时
	iMinute = iTotalSeconds % 3600 / 60;//分钟
	iSecond = iTotalSeconds % 60;//秒
	printf("持续时间：%02d:%02d:%02d\n", iHour, iMinute, iSecond);
	printf("平均混合码率：%d kb/s\n", pFormatCtx->bit_rate / 1000);
	printf("视音频个数：%d\n", pFormatCtx->nb_streams);
	puts("---------------------------------------------");

	puts("AVInputFormat信息:");
	puts("---------------------------------------------");
	printf("封装格式名称：%s\n", pFormatCtx->iformat->name);
	printf("封装格式长名称：%s\n", pFormatCtx->iformat->long_name);
	printf("封装格式扩展名：%s\n", pFormatCtx->iformat->extensions);
	printf("封装格式ID：%d\n", pFormatCtx->iformat->raw_codec_id);
	puts("---------------------------------------------");

	puts("AVStream信息:");
	puts("---------------------------------------------");
	printf("视频流标识符：%d\n", pFormatCtx->streams[videoIndex]->index);
	printf("音频流标识符：%d\n", pFormatCtx->streams[audioIndex]->index);
	printf("视频流长度：%d微秒\n", pFormatCtx->streams[videoIndex]->duration);
	printf("音频流长度：%d微秒\n", pFormatCtx->streams[audioIndex]->duration);
	puts("---------------------------------------------");

	puts("AVCodecContext信息:");
	puts("---------------------------------------------");
	printf("视频码率：%d kb/s\n", pCodecCtx->bit_rate / 1000);
	printf("视频大小：%d * %d\n", pCodecCtx->width, pCodecCtx->height);
	puts("---------------------------------------------");

	puts("AVCodec信息:");
	puts("---------------------------------------------");
	printf("视频编码格式：%s\n", pCodec->name);
	printf("视频编码详细格式：%s\n", pCodec->long_name);
	puts("---------------------------------------------");

	printf("视频时长：%d微秒\n", pFormatCtx->streams[videoIndex]->duration);
	printf("音频时长：%d微秒\n", pFormatCtx->streams[audioIndex]->duration);
	printf("音频采样率：%d\n", pFormatCtx->streams[audioIndex]->codec->sample_rate);
	printf("音频信道数目：%d\n", pFormatCtx->streams[audioIndex]->codec->channels);

	puts("AVFormatContext元数据：");
	puts("---------------------------------------------");
	while (dict = av_dict_get(pFormatCtx->metadata, "", dict, AV_DICT_IGNORE_SUFFIX))
	{
		printf("[%s] = %s\n", dict->key, dict->value);
	}
	puts("---------------------------------------------");

	puts("AVStream视频元数据：");
	puts("---------------------------------------------");
	dict = NULL;
	while (dict = av_dict_get(pFormatCtx->streams[videoIndex]->metadata, "", dict, AV_DICT_IGNORE_SUFFIX))
	{
		printf("[%s] = %s\n", dict->key, dict->value);
	}
	puts("---------------------------------------------");

	puts("AVStream音频元数据：");
	puts("---------------------------------------------");
	dict = NULL;
	while (dict = av_dict_get(pFormatCtx->streams[audioIndex]->metadata, "", dict, AV_DICT_IGNORE_SUFFIX))
	{
		printf("[%s] = %s\n", dict->key, dict->value);
	}
	puts("---------------------------------------------");


	av_dump_format(pFormatCtx, -1, fileName, 0);
	printf("\n\n编译信息：\n%s\n\n", avcodec_configuration());


	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
	return 0;
}
