﻿// AVfilterPlayer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>



#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
#define snprintf _snprintf
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"  
#include "SDL/SDL.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <SDL/SDL.h>
#ifdef __cplusplus
};
#endif
#endif

#include "includeLib/ImportLib.h"

//Enable SDL?
#define ENABLE_SDL 0
//Output YUV data?
#define ENABLE_YUVFILE 0

#define  ENABLE_YUVFILE_VESION 1


const char *filter_descr = "movie=my_logo.png[wm];[in][wm]overlay=5:5[out]";

static AVFormatContext *pFormatCtx;
static AVCodecContext *pCodecCtx;
AVFilterContext *buffersink_ctx;
AVFilterContext *buffersrc_ctx;
AVFilterGraph *filter_graph;
static int video_stream_index = -1;

static int open_input_file(const char *filename)
{
	int ret;
	AVCodec *dec;

	if ((ret = avformat_open_input(&pFormatCtx, filename, NULL, NULL)) < 0) {
		printf("Cannot open input file\n");
		return ret;
	}

	if ((ret = avformat_find_stream_info(pFormatCtx, NULL)) < 0) {
		printf("Cannot find stream information\n");
		return ret;
	}

	/* select the video stream */
	ret = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
	if (ret < 0) {
		printf("Cannot find a video stream in the input file\n");
		return ret;
	}
	video_stream_index = ret;
	pCodecCtx = pFormatCtx->streams[video_stream_index]->codec;

	/* init the video decoder */
	if ((ret = avcodec_open2(pCodecCtx, dec, NULL)) < 0) {
		printf("Cannot open video decoder\n");
		return ret;
	}

	return 0;
}

static int init_filters(const char *filters_descr)
{
	char args[512];
	int ret;
	const  AVFilter *buffersrc = avfilter_get_by_name("buffer");
	const  AVFilter *buffersink = avfilter_get_by_name("buffersink");
	AVFilterInOut *outputs = avfilter_inout_alloc();
	AVFilterInOut *inputs = avfilter_inout_alloc();
	enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
	AVBufferSinkParams *buffersink_params;

	filter_graph = avfilter_graph_alloc();

	/* buffer video source: the decoded frames from the decoder will be inserted here. */
	snprintf(args, sizeof(args),
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->time_base.num, pCodecCtx->time_base.den,
		pCodecCtx->sample_aspect_ratio.num, pCodecCtx->sample_aspect_ratio.den);

	ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
		args, NULL, filter_graph);
	if (ret < 0) {
		printf("Cannot create buffer source\n");
		return ret;
	}

	/* buffer video sink: to terminate the filter chain. */
	buffersink_params = av_buffersink_params_alloc();
	buffersink_params->pixel_fmts = pix_fmts;
	ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
		NULL, buffersink_params, filter_graph);
	av_free(buffersink_params);
	if (ret < 0) {
		printf("Cannot create buffer sink\n");
		return ret;
	}

	/* Endpoints for the filter graph. */
	outputs->name = av_strdup("in");
	outputs->filter_ctx = buffersrc_ctx;
	outputs->pad_idx = 0;
	outputs->next = NULL;

	inputs->name = av_strdup("out");
	inputs->filter_ctx = buffersink_ctx;
	inputs->pad_idx = 0;
	inputs->next = NULL;

	if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr,
		&inputs, &outputs, NULL)) < 0)
		return ret;

	if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
		return ret;
	return 0;
}



int main()
{
	int ret;
	AVPacket packet;
	AVFrame *pFrame;
	AVFrame *pFrame_out;

	int got_frame;

	av_register_all();
	avfilter_register_all();

	if ((ret = open_input_file("cuc_ieschool.flv")) < 0)
		goto end;
	if ((ret = init_filters(filter_descr)) < 0)
		goto end;

#if ENABLE_YUVFILE
	FILE *fp_yuv = fopen("test.yuv", "wb+");
#endif

#if ENABLE_SDL
	SDL_Surface *screen;
	SDL_Overlay *bmp;
	SDL_Rect rect;
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}
	screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 0, 0);
	if (!screen) {
		printf("SDL: could not set video mode - exiting\n");
		return -1;
	}
	bmp = SDL_CreateYUVOverlay(pCodecCtx->width, pCodecCtx->height, SDL_YV12_OVERLAY, screen);

	SDL_WM_SetCaption("Simplest FFmpeg Video Filter", NULL);
#endif

	pFrame = av_frame_alloc();
	pFrame_out = av_frame_alloc();

	/* read all packets */
	while (1) {

		ret = av_read_frame(pFormatCtx, &packet);
		if (ret < 0)
			break;

		if (packet.stream_index == video_stream_index) {
			//got_frame = 0;
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_frame, &packet);
			if (ret < 0) {
				printf("Error decoding video\n");
				break;
			}

			if (got_frame) {
				pFrame->pts = av_frame_get_best_effort_timestamp(pFrame);

				/* push the decoded frame into the filtergraph */
				if (av_buffersrc_add_frame(buffersrc_ctx, pFrame) < 0) {
					printf("Error while feeding the filtergraph\n");
					break;
				}

				/* pull filtered pictures from the filtergraph */
				while (1) {

					ret = av_buffersink_get_frame(buffersink_ctx, pFrame_out);
					if (ret < 0)
						break;

					printf("Process 1 frame!\n");

#if ENABLE_YUVFILE_VESION
					int screen_w = 0, screen_h = 0;
					SDL_Window *screen;
					SDL_Renderer* sdlRenderer;
					SDL_Texture* sdlTexture;
					SDL_Rect sdlRect;

					int videoindex = -1;
					for (int i = 0; i < buffersink_ctx->nb_outputs; i++)
						if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
							videoindex = i;
							break;
						}
					if (videoindex == -1) {
						printf("Didn't find a video stream.\n");
						return -1;
					}

					pCodecCtx = pFormatCtx->streams[videoindex]->codec;
					AVCodec         *pCodec;
					pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

					unsigned char *out_buffer;
					out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
					av_image_fill_arrays(pFrame_out->data, pFrame_out->linesize, out_buffer,
						AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);



					if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
						printf("Could not initialize SDL - %s\n", SDL_GetError());
						return -1;
					}

					screen_w = pCodecCtx->width;
					screen_h = pCodecCtx->height;
					//SDL 2.0 Support for multiple windows  
					screen = SDL_CreateWindow("demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
						screen_w, screen_h,
						SDL_WINDOW_OPENGL);

					if (!screen) {
						printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
						return -1;
					}

					sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
					//IYUV: Y + U + V  (3 planes)  
					//YV12: Y + V + U  (3 planes)  
					sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);

					sdlRect.x = 0;
					sdlRect.y = 0;
					sdlRect.w = screen_w;
					sdlRect.h = screen_h;

					//packet = (AVPacket *)av_malloc(sizeof(AVPacket));
					//Output Info-----------------------------  
					struct SwsContext *img_convert_ctx;
					img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
						pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);


					sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
						pFrame_out->data, pFrame_out->linesize);

#if OUTPUT_YUV420P  
					y_size = pCodecCtx->width*pCodecCtx->height;
					fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y   
					fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U  
					fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V  
#endif  
				//SDL---------------------------  
#if 0  
					SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
#else  
					SDL_UpdateYUVTexture(sdlTexture, &sdlRect,
						pFrame_out->data[0], pFrame_out->linesize[0],
						pFrame_out->data[1], pFrame_out->linesize[1],
						pFrame_out->data[2], pFrame_out->linesize[2]);
#endif    

					SDL_RenderClear(sdlRenderer);
					SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
					SDL_RenderPresent(sdlRenderer);
					//SDL End-----------------------  
					//Delay 40ms  
					SDL_Delay(40);

#endif






					if (pFrame_out->format == AV_PIX_FMT_YUV420P) {
#if ENABLE_YUVFILE
						//Y, U, V
						for (int i = 0; i < pFrame_out->height; i++) {
							fwrite(pFrame_out->data[0] + pFrame_out->linesize[0] * i, 1, pFrame_out->width, fp_yuv);
						}
						for (int i = 0; i < pFrame_out->height / 2; i++) {
							fwrite(pFrame_out->data[1] + pFrame_out->linesize[1] * i, 1, pFrame_out->width / 2, fp_yuv);
						}
						for (int i = 0; i < pFrame_out->height / 2; i++) {
							fwrite(pFrame_out->data[2] + pFrame_out->linesize[2] * i, 1, pFrame_out->width / 2, fp_yuv);
						}
#endif

#if ENABLE_SDL
						SDL_LockYUVOverlay(bmp);
						int y_size = pFrame_out->width*pFrame_out->height;
						memcpy(bmp->pixels[0], pFrame_out->data[0], y_size);   //Y
						memcpy(bmp->pixels[2], pFrame_out->data[1], y_size / 4); //U
						memcpy(bmp->pixels[1], pFrame_out->data[2], y_size / 4); //V 
						bmp->pitches[0] = pFrame_out->linesize[0];
						bmp->pitches[2] = pFrame_out->linesize[1];
						bmp->pitches[1] = pFrame_out->linesize[2];
						SDL_UnlockYUVOverlay(bmp);
						rect.x = 0;
						rect.y = 0;
						rect.w = pFrame_out->width;
						rect.h = pFrame_out->height;
						SDL_DisplayYUVOverlay(bmp, &rect);
						//Delay 40ms
						SDL_Delay(40);
#endif
					}
					av_frame_unref(pFrame_out);
				}
			}
			av_frame_unref(pFrame);
		}
		av_free_packet(&packet);
	}
#if ENABLE_YUVFILE
	fclose(fp_yuv);
#endif

end:
	avfilter_graph_free(&filter_graph);
	if (pCodecCtx)
		avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);


	if (ret < 0 && ret != AVERROR_EOF) {
		char buf[1024];
		av_strerror(ret, buf, sizeof(buf));
		printf("Error occurred: %s\n", buf);
		return -1;
	}

	return 0;

}

