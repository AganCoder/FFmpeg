#include <iostream>
#include <string>

using namespace std;

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <SDL.h>
#include <inttypes.h>
#include <stdio.h>
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
    if(  SDL_Init(SDL_INIT_EVERYTHING) != 0 )
    {
        SDL_Log("SDL_Init fail: %s \n", SDL_GetError());
        exit(1);
    }

    AVFormatContext *avformatCtx = avformat_alloc_context();
    char filePath[] = "/Users/noah-normal/Documents/Videos/Titanic.ts";
     
    if( avformat_open_input(&avformatCtx, filePath, NULL, NULL) != 0 )
    {
        cerr << "Could not open video file";
        exit(1);
    }
    
    /* retrieve stream information */
    if( avformat_find_stream_info(avformatCtx, NULL) < 0 )
    {
        cerr << "Could not find stream infomation \n";
        exit(1);
    }
    
//    cout << "时长: " << avformatCtx->duration << "" << endl;
//    cout << "封装格式名称:" << avformatCtx->iformat->name << endl;
//    cout << "封装格式长名称: = " << avformatCtx->iformat->long_name << endl;
//    // cout << "封装格式的扩展名: = " << avformatCtx->iformat->extensions << endl; // 某些上面没有,会 crash
//    cout << "封装格式 id = " << avformatCtx->iformat->raw_codec_id << endl;
//    cout << "输入视频的 AVStream 个数 = " << avformatCtx->nb_streams  << endl;
//    cout << "输入视频的码率 = " << avformatCtx->bit_rate << endl;
//    cout << "---- AVFormatContext Finish ----\n " << endl;
    
    // 查找视频流的 index
//    int videoStreamIndex = -1;
//    for( int index = 0; index < avformatCtx->nb_streams; ++index )
//    {
//        AVStream *stream = avformatCtx->streams[index];
//        AVCodecParameters *codecpar = stream->codecpar;
//
//        if( codecpar->codec_type == AVMEDIA_TYPE_VIDEO )
//        {
//            videoStreamIndex = index;
//            break;
//        }
//    }
    int videoStreamIndex = av_find_best_stream(avformatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if( videoStreamIndex == -1 )
    {
        cerr << "NO Video Stream " << endl;
        exit(1);
    }
    
    AVStream *videoStream = avformatCtx->streams[videoStreamIndex];
    AVCodecParameters *videoCodecPar = videoStream->codecpar;
//    cout << "AVStream 序号: " << videoStream->id << endl;
//    cout << "当前 AVStream 流的帧率 num = " << videoStream->r_frame_rate.num << " den = " << videoStream->r_frame_rate.den << endl;
//    cout << "宽 * 高: " << videoCodecPar->width << " * " << videoCodecPar->height << endl;
//    cout << "解码器名称: " << avcodec_get_name(videoCodecPar->codec_id) << endl;
//    cout << "------- AVStream Finish ------- \n " << endl;

    const AVCodec *videoCodec = avcodec_find_decoder(videoCodecPar->codec_id);
    if( videoCodec == NULL )
    {
        cerr << "Not find decoder";
        exit(1);
    }
    cout << "解码器名称: " << videoCodec->name << endl;
    cout << "解码器长名称: " << videoCodec->long_name << endl;
    cout << "------- AVCodec Finish ------- \n " << endl;
        
    AVCodecContext *videoCodecCtx = avcodec_alloc_context3(videoCodec);
    if( videoCodecCtx == NULL )
    {
        cerr << "alloc avcodec context failed" << endl;
        exit(1);
    }
    
    if( avcodec_parameters_to_context(videoCodecCtx, videoCodecPar) < 0 )
    {
        cerr << "parameters to context failed" << av_get_media_type_string(AVMEDIA_TYPE_VIDEO) << endl;
        exit(1);
    }

    if( avcodec_open2(videoCodecCtx, videoCodec, NULL) < 0 )
    {
        cerr << " Could not open codec ";
        exit(1);
    }
    FILE *fp_264 = fopen("/Users/noah-normal/Desktop/test264.h264", "wb+");
    FILE *fp_yuv = fopen("/Users/noah-normal/Desktop/test264.yuv", "wb+");
    if( fp_264 == NULL || fp_yuv == NULL )
    {
        cerr << "Could not open h264 && YUV file" << endl;
        exit(1);
    }
    
    AVFrame *avFrame = av_frame_alloc();
    if( avFrame == NULL )
    {
        cerr << "Could not alloc frame";
        exit(1);
    }
    
    AVPacket *packet = av_packet_alloc();
    if( packet == NULL )
    {
        cerr << "Could not alloc packet";
        exit(1);
    }    
    while( av_read_frame(avformatCtx, packet) >= 0 )
    {
        if( packet->stream_index == videoStreamIndex )
        {
            // 写入 h264 裸数据
            fwrite(packet->data, 1, packet->size, fp_264);
            int ret = decodePacket(videoCodecCtx, avFrame, packet, fp_yuv);
            av_packet_unref(packet);
            if( ret < 0 )
                break;
        }
    }
    
    decodePacket(videoCodecCtx, avFrame, packet, fp_yuv);
    
end:
    av_packet_free(&packet);
    fclose(fp_yuv);
    fclose(fp_264);
    avcodec_free_context(&videoCodecCtx);
    avformat_free_context(avformatCtx);

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *surface;
    SDL_Surface *imageSurface;
    SDL_Texture *texture;
    FILE *file;
    unsigned char *yuv_data;
    
    SDL_Init(SDL_INIT_EVERYTHING);
    
    int width = 640;
    int height = 272;
    int yuvSize = width * height * 3 / 2;
    file = fopen("/Users/noah-normal/Desktop/test264.yuv", "rb");
    
    if( file == NULL )
    {
        SDL_Log("Error opening");
        return EXIT_FAILURE;
    }
    yuv_data = static_cast<unsigned char*>(malloc(yuvSize * sizeof(unsigned char)));

    window = SDL_CreateWindow("Hello SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_ALLOW_HIGHDPI);

    if( window == NULL )
    {
        cout << "Counld not create window" << SDL_GetError() << endl;
        return EXIT_FAILURE;
    }
    
    renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STATIC, width, height);
 
    if( texture != NULL )
    {
        SDL_Event windowEvent;
        for(;;)
        {
            if( SDL_PollEvent(&windowEvent) )
            {
                if( SDL_QUIT == windowEvent.type )
                {
                    break;
                }
            }

            fread(yuv_data, 1, yuvSize, file);
            
            SDL_UpdateTexture(texture, NULL, yuv_data, width);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
            
            SDL_Delay(40);
        }
    }
    
    fclose(file);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    
    return 0;
}
