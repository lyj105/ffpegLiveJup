
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




class BossLiveRept

{
		public:
			BossLiveRept();
			~BossLiveRept();
        public:
			int GetBossLivePushRetp();
			int print_avError(int errNum);
		private:
			//const char *inUrl;
			//const char *outUrl;

			//AVFormatContext *input_ctx;
			//AVOutputFormat *output_fmt;
			//AVFormatContext * output_ctx;



	
};

