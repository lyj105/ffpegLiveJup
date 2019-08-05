// TestLivePush.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include <libavutil/time.h>
#include <libavutil/mathematics.h>
}
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "postproc.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "swscale.lib")


int  getPushRetmpJUP();
int print_avError(int errNum);

using namespace std;

int main()
{
	getPushRetmpJUP();

	return EXIT_SUCCESS;

}

int  getPushRetmpJUP() {


	int videoindex = -1;
	av_register_all();
	avformat_network_init();

	const char *inUrl = "demo.mp4";
	const char *outUrl = "rtmp://push-play.bosscome.com/app/insnexx?auth_key=1564913019-0-0-37af47acf3b38c55fc2059c2b1504e9d";

	AVFormatContext *input_ctx = NULL;
	AVOutputFormat *output_fmt = NULL;

	int ret = avformat_open_input(&input_ctx, inUrl, 0, NULL);
	if (ret < 0)
	{
		return print_avError(ret);
	}
	cout << "avformat_open_input success!" << endl;

	ret = avformat_find_stream_info(input_ctx, 0);
	if (ret != 0)
	{
		return print_avError(ret);
	}

	av_dump_format(input_ctx, 0, inUrl, 0);

	AVFormatContext * output_ctx = NULL;
	//如果是输入文件 flv可以不传，可以从文件中判断。如果是流则必须传
	ret = avformat_alloc_output_context2(&output_ctx, NULL, "flv", outUrl);
	if (ret < 0)
	{
		return print_avError(ret);
	}
	cout << "avformat_alloc_output_context2 success!" << endl;

	output_fmt = output_ctx->oformat;
	cout << "nb_streams: " << input_ctx->nb_streams << endl;

	unsigned int i;
	for (i = 0; i < input_ctx->nb_streams; i++)
	{
		AVStream *in_stream = input_ctx->streams[i];
		AVStream *out_stream = avformat_new_stream(output_ctx, in_stream->codec->codec);
		if (!out_stream)
		{
			printf("未能成功添加音视频流\n");
			ret = AVERROR_UNKNOWN;
		}

		//将输入编解码器上下文信息 copy 给输出编解码器上下文
		//ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
		ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
		//ret = avcodec_parameters_from_context(out_stream->codecpar, in_stream->codec);
		//ret = avcodec_parameters_to_context(out_stream->codec, in_stream->codecpar);
		if (ret < 0)
		{
			printf("copy 编解码器上下文失败\n");
		}
		out_stream->codecpar->codec_tag = 0;

		out_stream->codec->codec_tag = 0;
		if (output_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		{
			out_stream->codec->flags = out_stream->codec->flags | AV_CODEC_FLAG_GLOBAL_HEADER;
		}
	}

	for (i = 0; i < input_ctx->nb_streams; i++)
	{
		if (input_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	}
	cout << "videoindex: " << videoindex << endl;
	av_dump_format(output_ctx, 0, outUrl, 1);

	ret = avio_open(&output_ctx->pb, outUrl, AVIO_FLAG_WRITE);
	if (ret < 0)
	{
		return print_avError(ret);
	}

	//写入头部信息
	ret = avformat_write_header(output_ctx, 0);
	if (ret < 0)
	{
		return print_avError(ret);
	}

	cout << "avformat_write_header Success!" << endl;
	//推流每一帧数据
	//int64_t pts  [ pts*(num/den)  第几秒显示]
	//int64_t dts  解码时间 
	//uint8_t *data
	//int size
	//int stream_index
	//int flag
	AVPacket pkt;
	//获取当前的时间戳  微妙
	long long start_time = av_gettime();
	long long frame_index = 0;
	while (1)
	{
		AVStream *in_stream, *out_stream;
		ret = av_read_frame(input_ctx, &pkt);
		if (ret < 0)
		{
			break;
		}

		/*
		PTS（Presentation Time Stamp）显示播放时间
		DTS（Decoding Time Stamp）解码时间
		*/
		//没有显示时间（比如未解码的 H.264 ）
		if (pkt.pts == AV_NOPTS_VALUE)
		{
			//AVRational time_base：时基。通过该值可以把PTS，DTS转化为真正的时间。
			AVRational time_base1 = input_ctx->streams[videoindex]->time_base;

			//计算两帧之间的时间
			//r_frame_rate 帧率
			//av_q2d 转化为double类型
			int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(input_ctx->streams[videoindex]->r_frame_rate);

			//配置参数
			pkt.pts = (double)(frame_index*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
			pkt.dts = pkt.pts;
			pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
		}

		//延时
		if (pkt.stream_index == videoindex)
		{
			AVRational time_base = input_ctx->streams[videoindex]->time_base;
			AVRational time_base_q = { 1,AV_TIME_BASE };
			//计算视频播放时间
			int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
			//计算实际视频的播放时间
			int64_t now_time = av_gettime() - start_time;

			AVRational avr = input_ctx->streams[videoindex]->time_base;
			cout << avr.num << " " << avr.den << "  " << pkt.dts << "  " << pkt.pts << "   " << pts_time << endl;
			if (pts_time > now_time)
			{
				//睡眠一段时间（目的是让当前视频记录的播放时间与实际时间同步）
				cout << "pts_time: " << pts_time << "now_time: " << now_time << endl;
				av_usleep((unsigned int)(pts_time - now_time));
			}
		}

		in_stream = input_ctx->streams[pkt.stream_index];
		out_stream = output_ctx->streams[pkt.stream_index];

		//计算延时后，重新指定时间戳
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = (int)av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		//字节流的位置，-1 表示不知道字节流位置
		pkt.pos = -1;

		if (pkt.stream_index == videoindex)
		{
			printf("Send %8lld video frames to output URL\n", frame_index);
			frame_index++;
		}

		//向输出上下文发送（向地址推送）
		ret = av_interleaved_write_frame(output_ctx, &pkt);

		if (ret < 0)
		{
			printf("发送数据包出错\n");
			break;
		}
		av_free_packet(&pkt);
	}
	return EXIT_SUCCESS;


};


int print_avError(int errNum)
{
	char buf[1024];
	//获取错误信息
	av_strerror(errNum, buf, sizeof(buf));
	cout << " failed! " << buf << endl;
	return -1;
};