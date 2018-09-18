#ifndef PLAYSOUND_H
#define PLAYSOUND_H

#include <qstring.h>
#include <alsa/asoundlib.h>
#include <QMutex>

#define G723_PACKET_LEN    20
#define AUDIO_PACKET_LEN    480
#define AUDIO_PLAY_BUFFER_LEN   AUDIO_PACKET_LEN*10

#define AUDIO_CHANNEL_MONO   1
#define AUDIO_SAMPLE_RATE_8K  8000
#define AUDIO_SAMPLE_RATE_16K  16000
#define AUDIO_SAMPLE_RATE_44K  44100
#define AUDIO_SAMPLE_BIT_16    16

enum {
    ALSA_OPER_PLAY,
    ALSA_OPER_RECORD
};

class CAlsa
{
public:
    CAlsa();
    static void PlaySound(QString name);
    static QString soundPath;

private:
    int devMode; //ALSA_OPER_PLAY , ALSA_OPER_RECORD
    int channels;
    unsigned int rate;
    snd_pcm_format_t sndFormat; //采样位数
    int sampleLen;
    snd_pcm_t *playHandle;

    bool isBuffer;
    unsigned char audioBuffer[AUDIO_PLAY_BUFFER_LEN+16];
    int bufLen;
    QMutex playHandleMutex;
public:
    int open(int oper);
    int setChannel(int ch);
    int setRate(int rate);
    int setSamplebits(int bits);
    int play(unsigned char* data, int len);
    int playDrain();
    int record(unsigned char *data, int len);
    int stop();
    int writePcmData(snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size);
};

#endif // PLAYSOUND_H
