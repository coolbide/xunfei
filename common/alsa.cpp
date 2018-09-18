#include "alsa.h"
#include <stdio.h>
#include <iostream>
#include <QDebug>
#include <sys/time.h>
#include "utility.h"


#define DEVICE_MIX  0
#if DEVICE_MIX
#define PLAYER_DEVICE "plug:dmix"
#define RECORD_DEVICE "plug:dsnoop"
#else
#define PLAYER_DEVICE "default"
#define RECORD_DEVICE "default"
//#define RECORD_DEVICE "hw:1,0"
#endif

QString CAlsa::soundPath = "";
int BUFFER_LENGTH   = AUDIO_PLAY_BUFFER_LEN;

struct WAV_HEADER
{
   char rld[4]; //riff 标志符号
   int rLen;
   char wld[4]; //格式类型（wave）
   char fld[4]; //"fmt"

   int fLen; //sizeof(wave format matex)

   short wFormatTag; //编码格式
   short wChannels; //声道数
   int nSamplesPersec ; //采样频率
   int nAvgBitsPerSample;//WAVE文件采样大小
   short wBlockAlign; //块对齐
   short wBitsPerSample; //WAVE文件采样大小

   char dld[4]; //”data“
   int wSampleLength; //音频数据的大小

} wav_header;

CAlsa::CAlsa()
{
    playHandle = NULL;

    bufLen = 0;
    isBuffer = true;

    sndFormat = SND_PCM_FORMAT_S16_LE;
    sampleLen = 2;

    channels = 1;
    rate = 16000;
}

void CAlsa::PlaySound(QString name)
{
    long       loops;
    int        rc;
    int        size;
    snd_pcm_t* handle;
    snd_pcm_hw_params_t* params;//硬件信息和PCM流配置
    unsigned int val;
    int dir=0;
    snd_pcm_uframes_t frames;
    char* buffer;
    int nread;
    unsigned int exact_rate;
    int ret;
    int channels;
    int frequency;
    int bit;
    int datablock;
    char*  cPath;
    unsigned char ch[100]; //用来存储wav文件的头信息
    QString path;  // = "/media/sda/MMIWav/";
    FILE *fp;

    path = soundPath + name;
    QByteArray ba = path.toLatin1();
    cPath=ba.data();
//    cPath = "/media/sda/67.wav";

    //printf("to play %s\n", cPath);

    fp=fopen(cPath,"rb");
    if(fp==NULL)
    {
        printf("open file failed:%s\n", cPath);
        return;
    }

    nread=fread(&wav_header,1,sizeof(wav_header),fp);

    /*
    printf("nread=%d\n",nread);
    //printf("RIFF 标志%s\n",wav_header.rld);
    printf("文件大小rLen：%d\n",wav_header.rLen);
    //printf("wld=%s\n",wav_header.wld);
    //printf("fld=%s\n",wav_header.fld);
    // printf("fLen=%d\n",wav_header.fLen);
    //printf("wFormatTag=%d\n",wav_header.wFormatTag);
    printf("声道数：%d\n",wav_header.wChannels);
    printf("采样频率：%d\n",wav_header.nSamplesPersec);
    //printf("nAvgBitsPerSample=%d\n",wav_header.nAvgBitsPerSample);
    printf("wBlockAlign=%d\n",wav_header.wBlockAlign);
    printf("采样的位数：%d\n",wav_header.wBitsPerSample);
    // printf("data=%s\n",wav_header.dld);
    printf("wSampleLength=%d\n",wav_header.wSampleLength);
    */

    channels=wav_header.wChannels;
    val=wav_header.nSamplesPersec;
    bit=wav_header.wBitsPerSample;
    datablock=wav_header.wBlockAlign;
    rc=snd_pcm_open(&handle, PLAYER_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
    if(rc<0)
    {
           perror("\nopen PCM device failed:");
           return;
    }


    snd_pcm_hw_params_alloca(&params); //分配params结构体
    if(rc<0)
    {
           perror("\nsnd_pcm_hw_params_alloca:");
           return;
    }
    rc=snd_pcm_hw_params_any(handle, params);//初始化params
    if(rc<0)
    {
           perror("\nsnd_pcm_hw_params_any:");
           return;
    }
    rc=snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED); //初始化访问权限
    if(rc<0)
    {
           perror("\nsed_pcm_hw_set_access:");
           return;

    }

    //采样位数
    snd_pcm_format_t sndFormat;

    switch(bit/8)
    {
        case 1:
            sndFormat = SND_PCM_FORMAT_U8;
                break ;
        case 2:
            sndFormat = SND_PCM_FORMAT_S16_LE;
            break ;
        case 3:
            sndFormat = SND_PCM_FORMAT_S24_LE;
                break ;

    }
    snd_pcm_hw_params_set_format(handle, params, sndFormat);

    rc=snd_pcm_hw_params_set_channels(handle, params, channels); //设置声道,1表示单声>道，2表示立体声
    if(rc<0)
    {
            perror("\nsnd_pcm_hw_params_set_channels:");
            return;
    }
//    val = 14000;//<=9800  >=13520

    rc=snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir); //设置>频率
    if(rc<0)
    {
            perror("\nsnd_pcm_hw_params_set_rate_near:");
            return;
    }

    rc = snd_pcm_hw_params(handle, params);
    if(rc<0)
    {
        perror("\nsnd_pcm_hw_params: ");
        return;
    }

    rc=snd_pcm_hw_params_get_period_size(params, &frames, &dir); /*获取周期长度*/
    if(rc<0)
    {
            perror("\nsnd_pcm_hw_params_get_period_size:");
            return;
    }

    size = frames * datablock; /*4 代表数据快长度*/

    /*
    printf("\nframes = %d", frames);
    printf("\nsize = %d\n", size);
    printf("\nbolck size = %d\n", datablock);
    */

    buffer =(char*)malloc(size);
    fseek(fp,58,SEEK_SET); //定位歌曲到数据区

    bool over = false;
    int playFrames;
    int index;
    int times;
    int times1;
    while (1)
    {
        memset(buffer,0,sizeof(buffer));
        ret = fread(buffer, 1, size, fp);
        if(ret <= 0)
        {
            //printf("歌曲写入结束\n");
            break;
        }
        else if (ret != size)
        {
            // last block:
            over = true;
            frames = ret / datablock;
            //if (!frames)
             break;
        }

        // 写音频数据到PCM设备
        playFrames = frames;
        index = 0;
        times = 0;
        times1 = 0;
        while ((ret = snd_pcm_writei(handle, buffer+index, playFrames)) != 0)
        {

            if (ret == -EPIPE)
            {
                /* EPIPE means underrun */
                snd_pcm_prepare(handle);
                times1++;
                if(times1 > 5)
                {
                    break;
                }
                fprintf(stderr, "underrun occurred ret = -EPIPE\n");
                continue;
            }
            else if (ret < 0)
            {
                //完成硬件参数设置，使设备准备好
                snd_pcm_prepare(handle);
                fprintf(stderr,"error from writei: %s\n",snd_strerror(ret));
                break;
            }
            else // ret > 0
            {
                //Log("snd_pcm_writei() left: %d - %d times", ret, times);
                times++;
                if (times > 5)
                {
                    //Log("snd_pcm_writei() left: %d - to Max %d times", ret, times);
                    break;
                }

                index = datablock * ret;
                playFrames -= ret;
            }
        }

        /*
        if (times > 0)
        {
            Log("snd_pcm_writei() out. left: %d - %d times", ret, times);
        }*/

        if (over)
        {
            break;
        }

    }

    snd_pcm_drain(handle);
    //snd_pcm_close(handle);
    free(buffer);
}


int CAlsa::open(int oper)
{
    int rc;
    snd_pcm_hw_params_t* params = NULL;//硬件信息和PCM流配置
    snd_pcm_stream_t stream;
    int mode = 0;
    char* device = NULL;

    int dir = 0;
    int ret = -1;
    if (playHandle != NULL)
    {
        return 0;
    }

    LogDebug("channel %d, bits %d, rate %d",  channels, sampleLen*8, rate);
    devMode = oper;

    if (oper == ALSA_OPER_PLAY)
    {
        stream = SND_PCM_STREAM_PLAYBACK;
        mode = 0;   // SND_PCM_NONBLOCK
        //LogDebug("sound device: %s", PLAYER_DEVICE);
        device = PLAYER_DEVICE;
    }
    else
    {
        stream = SND_PCM_STREAM_CAPTURE;
        mode = 0;
        //LogDebug("sound device: %s", RECORD_DEVICE);
        device = RECORD_DEVICE;
    }
    
    rc = snd_pcm_open(&playHandle, device, stream, mode);
    if (rc < 0)
    {
        LogError("unable to open pcm device: %s\n",snd_strerror(rc));
        return ret;
    }

    //snd_pcm_hw_params_alloca(&params); //分配params结构体
    rc = snd_pcm_hw_params_malloc(&params); //分配params结构体
    if((rc < 0) || (!params))
    {
           LogError("\nsnd_pcm_hw_params_alloca:");
           snd_pcm_close(playHandle);
           return ret;
    }
    //LogDebug("%s %d:snd_pcm_hw_params_any()", __func__, __LINE__);
    rc=snd_pcm_hw_params_any(playHandle, params);//初始化params
    if(rc<0)
    {
           LogError("\nsnd_pcm_hw_params_any:");
           snd_pcm_close(playHandle);
           return ret;
    }

    rc = snd_pcm_hw_params_set_access(playHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED); //初始化访问权限
    if(rc < 0)
    {
           LogError("\nsed_pcm_hw_set_access:");
           snd_pcm_close(playHandle);
           return ret;
    }

    rc = snd_pcm_hw_params_set_format(playHandle, params, sndFormat);
    if(rc < 0)
    {
            LogError("snd_pcm_hw_params_set_format:");
            snd_pcm_close(playHandle);
            return ret;
    }
    if (oper != ALSA_OPER_PLAY)
    {
        qDebug() << "ALSA_OPER_RECORD channel" << channels << endl;
    }

    rc=snd_pcm_hw_params_set_channels(playHandle, params, channels); //设置声道,1表示单声>道，2表示立体声
    if(rc < 0)
    {
            LogError("\nsnd_pcm_hw_params_set_channels:");
            snd_pcm_close(playHandle);
            return ret;
    }

    //rc=snd_pcm_hw_params_set_rate_near(playHandle, params, &rate, &dir); //设置>频率
    rc=snd_pcm_hw_params_set_rate(playHandle, params, rate, dir); //设置>频率
    if(rc < 0)
    {
            LogError("\nsnd_pcm_hw_params_set_rate_near:%d, rate %d", rc, rate);
            snd_pcm_close(playHandle);
            return ret;
    }
    else
    {
        LogDebug("rate:%d", rate);
    }

    snd_pcm_uframes_t frames = 300;//2400;

    rc = snd_pcm_hw_params_set_period_size_near(playHandle, params, &frames, &dir);
    if(rc < 0)
    {
        LogError("\snd_pcm_hw_params_set_period_size:%d, frames %d", rc, frames);

        snd_pcm_close(playHandle);
        return ret;
    }
    else
    {
        LogDebug("frames near:%d", frames);
    }

    rc = snd_pcm_hw_params_get_period_size(params, &frames, &dir); /*获取周期长度*/
    if(rc < 0)
    {
        LogError("\snd_pcm_hw_params_get_period_size:%d, frames %d", rc, frames);

        snd_pcm_close(playHandle);
        return ret;
    }


    rc = snd_pcm_hw_params(playHandle, params);
    if(rc<0)
    {
        LogError("\nsnd_pcm_hw_params: ");
        snd_pcm_close(playHandle);
        return ret;
    }

    snd_pcm_hw_params_free(params);

    rc = snd_pcm_prepare(playHandle);
    if(rc<0)
    {
        LogError("\nsnd_pcm_hw_params: ");
        snd_pcm_close(playHandle);
        return ret;
    }

    //Log("channel:%d, sampleLen:%d, rate:%d", channels, sampleLen,rate);
    isBuffer = true;
    return 0;
}

int CAlsa::setChannel(int ch)
{
    channels = ch;
}

int CAlsa::setRate(int rate)
{
    this->rate = rate;
}

int CAlsa::setSamplebits(int bits)
{
    switch(bits/8)
    {
        case 1:
            sndFormat = SND_PCM_FORMAT_U8;
            sampleLen = 1;
            break ;
        case 2:
            sndFormat = SND_PCM_FORMAT_S16_LE; //SND_PCM_FORMAT_U16_LE
            sampleLen = 2;
            LogDebug("sndFormat SND_PCM_FORMAT_S16_LE, 2")
            break ;
        case 3:
            sndFormat = SND_PCM_FORMAT_S24_LE;
            sampleLen = 3;
                break ;
        default:
            sndFormat = SND_PCM_FORMAT_S16_LE; //SND_PCM_FORMAT_U16_LE
            sampleLen = 2;
            break ;
    }
}

// buffered + half buffer
int CAlsa::play(unsigned char *data, int len)
{
    int ret = -1;
    if(data == NULL || len <= 0)
    {
        return ret;
    }

    if (!playHandle)
    {
        return -1;
    }

    snd_pcm_uframes_t frames;
    unsigned char *buffer = data;

    int datablock = sampleLen * channels;
    int playFrames, index;
    isBuffer = true;
    if (isBuffer)
    {
        //if ((len + bufLen) >= BUFFER_LENGTH)
        {
            /*
            buffer = audioBuffer;
            frames = bufLen/sampleLen;
            frames /= channels;
            */
            frames = len / datablock;

            playFrames = frames;

            //qDebug() << "playframs" << playFrames << endl;
            index = 0;
            int times1 = 0;

            //while ((ret = snd_pcm_writei(playHandle, buffer+index, playFrames)) != 0)
            while((ret = writePcmData(playHandle, data + index, playFrames)) != 0)
            {
                /* EPIPE means underrun */
                if (ret == -EPIPE)
                {
                    snd_pcm_prepare(playHandle);
                    times1++;
                    if(times1 > 5)
                    {
                        break;
                    }
                    fprintf(stderr, "underrun occurred ret = -EPIPE\n");
                    continue;
                }
                else if (ret < 0)
                {
                    //完成硬件参数设置，使设备准备好
                    playHandleMutex.lock();
                    if(playHandle == NULL)
                    {
                        playHandleMutex.unlock();
                        break;
                    }
                    snd_pcm_prepare(playHandle);
                    playHandleMutex.unlock();
                    fprintf(stderr,"error from writei: %s\n",snd_strerror(ret));
                    break;
                }
                else // ret > 0
                {
                    index += datablock * ret;
                    playFrames -= ret;
                    //qDebug() << "playframes" << playFrames << endl;
                }
            }
            /*
            bufLen = 0;
            memcpy(audioBuffer+bufLen, data, len);
            bufLen += len;
            */
        }
        /*
        else
        {
            memcpy(audioBuffer+bufLen, data, len);
            bufLen += len;

            return 0;
        }
        */
    }
    else
    {
        // half buffer:
        qDebug() << "else" << endl;
        if ((len + bufLen) >= (BUFFER_LENGTH/2))
        {
            buffer = audioBuffer;
            frames = bufLen/sampleLen;
            frames /= channels;

            //LogDebug("\nplay buffer, frames: %d", frames);
            //ret = snd_pcm_writei(playHandle, buffer, frames);
            ret = writePcmData(playHandle,buffer,frames);
            if (ret < 0)
            {
                bufLen = 0;
                isBuffer = true;

                LogError("歌曲写入 error\n");
                //
                if (ret == -EPIPE)
                {
                    /* EPIPE means underrun */
                    LogError("underrun occurred\n");
                    //完成硬件参数设置，使设备准备好
                    usleep(1000*10);
                    if(playHandle != NULL)
                    {
                        snd_pcm_prepare(playHandle);
                    }
                }
                else
                {
                    LogError("error from writei: (%d) %s\n",ret, snd_strerror(ret));
                }
            }

            bufLen = 0;
            memcpy(audioBuffer+bufLen, data, len);
            bufLen += len;
        }
        else
        {
            //LogDebug("audio to buffer");
            memcpy(audioBuffer+bufLen, data, len);
            bufLen += len;

            return 0;
        }
    }
    return 0;
}

int CAlsa::playDrain()
{
    return snd_pcm_drain(playHandle);
}

int CAlsa::record(unsigned char *data, int len)
{
    int ret;
    snd_pcm_uframes_t frames;
    //unsigned char *buffer = data;

    if (!playHandle)
    {
        return -1;
    }

    frames = len/channels/sampleLen;

    //LogDebug("%s(len %d), frames:%d", __func__, len, frames);

    ret = snd_pcm_readi(playHandle, data, frames);

    if (ret > 0)
    {
        return 0;
    }
    else if (ret == -EAGAIN)// || (ret >= 0 && (size_t)ret < pcmdata->frames))
    {
        LogError("EAGAIN \n");
        snd_pcm_wait(playHandle, 1000);
    } else if (ret == -EPIPE)
    {
        LogError("underrun occurred\n");
        snd_pcm_prepare(playHandle);
    } else if (ret == -ESTRPIPE)
    {
        LogError("ESTRPIPE, Need suspend");
    } else
    {
        LogError("snd_pcm_readi error %s", snd_strerror(ret));
    }


    return -1;
}

int CAlsa::stop()
{
    playHandleMutex.lock();
    if (playHandle == NULL)
    {
        playHandleMutex.unlock();
        return 0;
    }

    // to do: confirm snd_pcm_drain
    snd_pcm_drain(playHandle);
    int ret = snd_pcm_close(playHandle);

    if (ret == 0)
    {
        playHandle = NULL;
    }
    else
    {
        playHandleMutex.unlock();
        return -1;
    }
    playHandleMutex.unlock();
    return 0;
}

int CAlsa::writePcmData(snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size)
{
    int ret = 0;

    if(pcm == NULL || buffer == NULL)
    {
        return ret;
    }
    playHandleMutex.lock();
    ret = snd_pcm_writei(pcm, buffer, size);
    playHandleMutex.unlock();
    return ret;
}
