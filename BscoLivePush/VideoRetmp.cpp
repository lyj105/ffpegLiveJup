
#include "VideoRetmp.h"

using namespace std;


BossLiveRept::BossLiveRept() {
	
	input_ctx = NULL;
    output_fmt = NULL;
	output_ctx = NULL;
	inUrl = "test.mp4";
	outUrl = "rtmp://push-play.bosscome.com/appp/inster?auth_key=1564492592-0-0-30fb8531f6fcb60b3a85c51ff61fb66c__";
};

BossLiveRept::~BossLiveRept() {


};

int BossLiveRept::GetBossLivePushRetp() {

	int videoindex = -1;

	av_register_all();
	avformat_network_init();



	int ret = avformat_open_input(&input_ctx, inUrl, 0, NULL);
	if (ret < 0)
	{
		return print_avError(ret);
	}

	printf("%c", "avformat_open_input success!");

	ret = avformat_find_stream_info(input_ctx, 0);
	if (ret != 0)
	{
		return print_avError(ret);
	}

	av_dump_format(input_ctx, 0, inUrl, 0);

	ret = avformat_alloc_output_context2(&output_ctx, NULL, "flv", outUrl);
	if (ret < 0) {
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

		if (!out_stream) {

			printf("δ�ܳɹ��������Ƶ��\n");
			ret = AVERROR_UNKNOWN;
		}

		//��������������������Ϣ copy ������������������
		ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);

		if (ret < 0) {
			printf("copy �������������ʧ��\n");
		}

		out_stream->codecpar->codec_tag = 0;

		out_stream->codec->codec_tag = 0;
		if (output_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
			out_stream->codec->flags = out_stream->codec->flags | AV_CODEC_FLAG_GLOBAL_HEADER;
		}

	}

	//���ҵ���ǰ�������е���Ƶ��������¼��Ƶ��������
	for (i = 0; i < input_ctx->nb_streams; i++)
	{
		if (input_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
			break;
		}
	}

	cout << "videoindex: " << videoindex << endl;
	av_dump_format(output_ctx, 0, outUrl, 1);

	ret = avio_open(&output_ctx->pb, outUrl, AVIO_FLAG_WRITE);
	if (ret < 0) {
		print_avError(ret);
	}

	//д��ͷ����Ϣ
	ret = avformat_write_header(output_ctx, 0);
	if (ret < 0) {
		print_avError(ret);
	}

	cout << "avformat_write_header Success!" << endl;

	//����ÿһ֡����
	//int64_t pts  [ pts*(num/den)  �ڼ�����ʾ]
	//int64_t dts  ����ʱ�� 
	//uint8_t *data
	//int size
	//int stream_index
	//int flag
	AVPacket pkt;
	//��ȡ��ǰʱ���
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
		PTS��Presentation Time Stamp����ʾ����ʱ��
		DTS��Decoding Time Stamp������ʱ��
		*/
		//û����ʾʱ�䣨����δ����� H.264 ��
		if (pkt.pts == AV_NOPTS_VALUE)
		{
			//AVRational time_base��ʱ����ͨ����ֵ���԰�PTS��DTSת��Ϊ������ʱ�䡣
			AVRational time_base1 = input_ctx->streams[videoindex]->time_base;

			//������֮֡���ʱ��
			//r_frame_rate ֡��
			//av_q2d ת��Ϊdouble����
			int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(input_ctx->streams[videoindex]->r_frame_rate);

			//���ò���
			pkt.pts = (double)(frame_index*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
			pkt.dts = pkt.pts;
			pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
		}

		//��ʱ
		if (pkt.stream_index == videoindex)
		{
			AVRational time_base = input_ctx->streams[videoindex]->time_base;
			AVRational time_base_q = { 1,AV_TIME_BASE };
			//������Ƶ����ʱ��
			int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
			//����ʵ����Ƶ�Ĳ���ʱ��
			int64_t now_time = av_gettime() - start_time;

			AVRational avr = input_ctx->streams[videoindex]->time_base;
			cout << avr.num << " " << avr.den << "  " << pkt.dts << "  " << pkt.pts << "   " << pts_time << endl;
			if (pts_time > now_time)
			{
				//˯��һ��ʱ�䣨Ŀ�����õ�ǰ��Ƶ��¼�Ĳ���ʱ����ʵ��ʱ��ͬ����
				cout << "pts_time: " << pts_time << "now_time: " << now_time << endl;
				av_usleep((unsigned int)(pts_time - now_time));
			}
		}

		in_stream = input_ctx->streams[pkt.stream_index];
		out_stream = output_ctx->streams[pkt.stream_index];

		//������ʱ������ָ��ʱ���
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = (int)av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		//�ֽ�����λ�ã�-1 ��ʾ��֪���ֽ���λ��
		pkt.pos = -1;

		if (pkt.stream_index == videoindex)
		{
			printf("Send %8lld video frames to output URL\n", frame_index);
			frame_index++;
		}

		//����������ķ��ͣ����ַ���ͣ�
		ret = av_interleaved_write_frame(output_ctx, &pkt);

		if (ret < 0)
		{
			printf("�������ݰ�����\n");
			break;
		}
		av_free_packet(&pkt);
	}

	return EXIT_SUCCESS;
};

int BossLiveRept::print_avError(int errNum)
{
	char buf[1024];
	//��ȡ������Ϣ
	//av_strerror(errNum, buf, sizeof(buf));
	cout << " failed! " << buf << endl;
	return -1;
};