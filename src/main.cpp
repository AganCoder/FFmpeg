#include <iostream>
#include <string>

using namespace std;

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <inttypes.h>
#include <stdio.h>
}

// YUV 4:2:0采样，每四个Y共用一组UV分量
// 视频宽高以及帧率
int simplest_yuv420_split(const char *url, int w, int h, int frame)
{
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("/Users/noah-normal/Desktop/output_420_y.yuv", "wb+");
    FILE *fp2 = fopen("/Users/noah-normal/Desktop/output_420_u.yuv", "wb+");
    FILE *fp3 = fopen("/Users/noah-normal/Desktop/output_420_v.yuv", "wb+");

    unsigned char *pic = (unsigned char *)malloc( w * h * 3 / 2  );
    for (int i = 0; i < frame; ++i) 
    {
        fread(pic, 1, w * h * 3 / 2, fp );
        fwrite(pic, 1, w * h , fp1);
        fwrite(pic + w * h, 1, w * h / 4 , fp2);
        fwrite(pic + w * h * 5 / 4, 1, w * h /4 , fp3);
    }

    free(pic);
    fclose(fp);
    fclose(fp1);
    fclose(fp2);
    fclose(fp3);

    return 0;
}

static int decodePacket(AVCodecContext *avctx, AVFrame *frame, AVPacket *packet, FILE *fp)
{
    int ret = 0;
    
    ret = avcodec_send_packet(avctx, packet);
    
    if( ret < 0 ) {
        cerr << "Error to send packet" << av_err2str(ret);
        return ret;
    }
    
    while ( ret >= 0) {
        
        ret = avcodec_receive_frame(avctx, frame);
        if( ret < 0 ) {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return 0;
            
            cerr <<"Error on receive frame " << av_err2str(ret);
            
            return ret;
        }
                        
        fwrite(frame->data[0], 1, frame->width * frame->height, fp);
        fwrite(frame->data[1], 1, frame->width * frame->height / 4, fp);
        fwrite(frame->data[2], 1, frame->width * frame->height / 4, fp);
            
        av_frame_unref(frame);
    }
    
    return 0;
}

int main(int argc, char const *argv[])
{
    char filePath[] = "/Users/noah-normal/Documents/Videos/sintel_640_360.yuv";

    simplest_yuv420_split(filePath, 640, 360, 99);
    
    return 0;
}
