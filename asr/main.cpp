//#include <QCoreApplication>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <QFile>

#include "../include/qisr.h"
#include "../include/msp_cmn.h"
#include "../include/msp_errors.h"

#include "../common/alsa.h"
#include "../common/utility.h"
#include "i2coper.h"
#include "parsexml.h"

#define SAMPLE_RATE_16K     (16000)
#define SAMPLE_RATE_8K      (8000)
#define MAX_GRAMMARID_LEN   (32)
#define MAX_PARAMS_LEN      (1024)

void play();
int save(int len);

const char * ASR_RES_PATH        = "fo|res/asr/common.jet"; //离线语法识别资源路径
//const char * ASR_RES_PATH        = "res/asr/common.jet"; //离线语法识别资源路径
const char * GRM_BUILD_PATH      = "res/asr/GrmBuilld"; //构建离线语法识别网络生成数据保存路径
const char * GRM_FILE            = "move.bnf"; //构建离线识别语法网络所用的语法文件
const char * LEX_NAME            = "contact"; //更新离线识别语法的contact槽（语法文件为此示例中使用的call.bnf）

typedef struct _UserData {
    int     build_fini; //标识语法构建是否完成
    int     update_fini; //标识更新词典是否完成
    int     errcode; //记录语法构建或更新词典回调错误码
    char    grammar_id[MAX_GRAMMARID_LEN]; //保存语法构建返回的语法ID
}UserData;


const char *get_audio_file(void); //选择进行离线语法识别的语音文件
int build_grammar(UserData *udata); //构建离线识别语法网络
int update_lexicon(UserData *udata); //更新离线识别语法词典
int run_asr(UserData *udata); //进行离线语法识别

const char* get_audio_file(void)
{
    char key = 0;
    while(key != 27) //按Esc则退出
    {
        printf("请选择音频文件：\n");
        printf("1.左转\n");
        printf("2.前进\n");
        key = getchar();
        getchar();
        switch(key)
        {
        case '1':
            printf("\n1.左转\n");
            return "wav/zuozhuan.pcm";
        case '2':
            printf("\n2.前进\n");
            return "wav/qianjin.pcm";
        default:
            continue;
        }
    }
    exit(0);
    return NULL;
}

int build_grm_cb(int ecode, const char *info, void *udata)
{
    UserData *grm_data = (UserData *)udata;

    if (NULL != grm_data) {
        grm_data->build_fini = 1;
        grm_data->errcode = ecode;
    }

    if (MSP_SUCCESS == ecode && NULL != info) {
        printf("构建语法成功！ 语法ID:%s\n", info);
        if (NULL != grm_data)
            snprintf(grm_data->grammar_id, MAX_GRAMMARID_LEN - 1, info);
    }
    else
        printf("构建语法失败！%d\n", ecode);

    return 0;
}

int build_grammar(UserData *udata)
{
    FILE *grm_file                           = NULL;
    char *grm_content                        = NULL;
    unsigned int grm_cnt_len                 = 0;
    char grm_build_params[MAX_PARAMS_LEN]    = {NULL};
    int ret                                  = 0;

    grm_file = fopen(GRM_FILE, "rb");
    if(NULL == grm_file) {
        printf("打开\"%s\"文件失败！[%s]\n", GRM_FILE, strerror(errno));
        return -1;
    }

    printf("打开语法文件\"%s\".\n", GRM_FILE);

    fseek(grm_file, 0, SEEK_END);
    grm_cnt_len = ftell(grm_file);
    fseek(grm_file, 0, SEEK_SET);

    grm_content = (char *)malloc(grm_cnt_len + 1);
    if (NULL == grm_content)
    {
        printf("内存分配失败!\n");
        fclose(grm_file);
        grm_file = NULL;
        return -1;
    }
    fread((void*)grm_content, 1, grm_cnt_len, grm_file);
    grm_content[grm_cnt_len] = '\0';
    fclose(grm_file);
    grm_file = NULL;

    snprintf(grm_build_params, MAX_PARAMS_LEN - 1,
        "engine_type = local, \
        asr_res_path = %s, sample_rate = %d, \
        grm_build_path = %s, ",
        ASR_RES_PATH,
        SAMPLE_RATE_16K,
        GRM_BUILD_PATH
        );

    printf("grm_content:\n%s\n", grm_content);
    printf("grm_cnt_len:%d\n", grm_cnt_len);
    printf("grm_build_params:%s\n", grm_build_params);

    ret = QISRBuildGrammar("bnf", grm_content, grm_cnt_len, grm_build_params, build_grm_cb, udata);

    free(grm_content);
    grm_content = NULL;

    printf("QISRBuildGrammar: %d\n", ret);

    return ret;
}

int update_lex_cb(int ecode, const char *info, void *udata)
{
    UserData *lex_data = (UserData *)udata;

    if (NULL != lex_data) {
        lex_data->update_fini = 1;
        lex_data->errcode = ecode;
    }

    if (MSP_SUCCESS == ecode)
        printf("更新词典成功！\n");
    else
        printf("更新词典失败！%d\n", ecode);

    return 0;
}

int update_lexicon(UserData *udata)
{
    const char *lex_content                   = "丁伟\n黄辣椒";
    unsigned int lex_cnt_len                  = strlen(lex_content);
    char update_lex_params[MAX_PARAMS_LEN]    = {NULL};

    snprintf(update_lex_params, MAX_PARAMS_LEN - 1,
        "engine_type = local, text_encoding = UTF-8, \
        asr_res_path = %s, sample_rate = %d, \
        grm_build_path = %s, grammar_list = %s, ",
        ASR_RES_PATH,
        SAMPLE_RATE_16K,
        GRM_BUILD_PATH,
        udata->grammar_id);
    return QISRUpdateLexicon(LEX_NAME, lex_content, lex_cnt_len, update_lex_params, update_lex_cb, udata);
}

#if 1
//#define AUDIO_PCM_LEN   6400
//#define AUDIO_PCM_LEN   4800
#define AUDIO_PCM_LEN   480

int len = AUDIO_PCM_LEN;

#define COUNT   600
char buffer[AUDIO_PCM_LEN * COUNT]={0};
CAlsa recorder;

int run_asr(UserData *udata)
{
    char asr_params[MAX_PARAMS_LEN]    = {NULL};
    const char *rec_rslt               = NULL;
    const char *session_id             = NULL;
    //const char *asr_audiof             = NULL;
    //FILE *f_pcm                        = NULL;
    char pcm_data[AUDIO_PCM_LEN]       = {0};
    //long pcm_count                     = 0;
    //long pcm_size                      = 0;
    //int last_audio                     = 0;
    int aud_stat                       = MSP_AUDIO_SAMPLE_CONTINUE;
    int ep_status                      = MSP_EP_LOOKING_FOR_SPEECH;
    int rec_status                     = MSP_REC_STATUS_INCOMPLETE;
    int rss_status                     = MSP_REC_STATUS_INCOMPLETE;
    int errcode                        = -1;


    recorder.setChannel(AUDIO_CHANNEL_MONO);
    recorder.setSamplebits(AUDIO_SAMPLE_BIT_16);
    recorder.setRate(AUDIO_SAMPLE_RATE_16K);

    if (0 != recorder.open(ALSA_OPER_RECORD))
    {
        printf("open recorder error\n");
        return -1;
    }


    //离线语法识别参数设置
    snprintf(asr_params, MAX_PARAMS_LEN - 1,
        "engine_type = local, \
        asr_res_path = %s, sample_rate = %d, \
        grm_build_path = %s, local_grammar = %s, \
        result_type = xml, result_encoding = UTF-8, ",
        ASR_RES_PATH,
        SAMPLE_RATE_16K,
        GRM_BUILD_PATH,
        udata->grammar_id
        );

    //printf("asr_params:%s\n", asr_params);
    session_id = QISRSessionBegin(NULL, asr_params, &errcode);
    if (NULL == session_id)
    {
        printf("QISRSessionBegin error!\n");
        return -1;
    }

    printf("开始识别...\n");



    aud_stat = MSP_AUDIO_SAMPLE_FIRST;
    unsigned int len = AUDIO_PCM_LEN;

    //struct timeval tv, tv2;

    int i = 0;
    //实时采集音频方式
    while (1) {

        printf(">");
        fflush(stdout);

        //gettimeofday(&tv, NULL);

        if (0 != recorder.record((unsigned char*) pcm_data, AUDIO_PCM_LEN))
        {
            printf("record error\n");
            break;
        }

        //printf("record ok.");
//        gettimeofday(&tv2, NULL);
//        printf("[%d]", (tv2.tv_sec-tv.tv_sec)*1000*1000 + tv2.tv_usec-tv.tv_usec);


        if (i < COUNT)
        {
            memcpy(buffer+i*AUDIO_PCM_LEN, pcm_data, AUDIO_PCM_LEN);
        }
        i++;

        errcode = QISRAudioWrite(session_id, (const void *)pcm_data, len, aud_stat, &ep_status, &rec_status);
        //printf("ret:%d, ep_status %d, rec_status", errcode, ep_status, rec_status);
        //fflush(stdout);

        if (MSP_SUCCESS != errcode)
        {
            printf("QISRAudioWrite error\n");
            break;
        }

        //检测到音频结束
        if (MSP_EP_AFTER_SPEECH == ep_status)
        {
            printf("\nMSP_EP_AFTER_SPEECH\n");

            break;
        }

        aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;

        //usleep(150 * 1000); //模拟人说话时间间隙
    }

    recorder.stop();


    if (MSP_SUCCESS == errcode)
    {
        //主动点击音频结束
        QISRAudioWrite(session_id, (const void *)NULL, 0, MSP_AUDIO_SAMPLE_LAST, &ep_status, &rec_status);

        printf("ep_status %d, rec_status %d\n", ep_status, rec_status);

        //获取识别结果
        while (MSP_REC_STATUS_COMPLETE != rss_status && MSP_SUCCESS == errcode) {
            rec_rslt = QISRGetResult(session_id, &rss_status, 0, &errcode);
            printf("rss_status %d, errcode %d\n", rss_status, errcode);
            usleep(150 * 1000);
        }
        printf("\n识别结束：\n");
        printf("=============================================================\n");
        if (NULL != rec_rslt)
            printf("%s\n", rec_rslt);
        else
            printf("没有识别结果！\n");
        printf("=============================================================\n");

    }
    else
    {
        //printf("error\n");
    }

    QISRSessionEnd(session_id, NULL);

    save(i*AUDIO_PCM_LEN);

    return errcode;
}

#else

int run_asr(UserData *udata)
{
    char asr_params[MAX_PARAMS_LEN]    = {NULL};
    const char *rec_rslt               = NULL;
    const char *session_id             = NULL;
    const char *asr_audiof             = NULL;
    FILE *f_pcm                        = NULL;
    char *pcm_data                     = NULL;
    long pcm_count                     = 0;
    long pcm_size                      = 0;
    int last_audio                     = 0;
    int aud_stat                       = MSP_AUDIO_SAMPLE_CONTINUE;
    int ep_status                      = MSP_EP_LOOKING_FOR_SPEECH;
    int rec_status                     = MSP_REC_STATUS_INCOMPLETE;
    int rss_status                     = MSP_REC_STATUS_INCOMPLETE;
    int errcode                        = -1;

    asr_audiof = get_audio_file();
    f_pcm = fopen(asr_audiof, "rb");
    if (NULL == f_pcm) {
        printf("打开\"%s\"失败！[%s]\n", f_pcm, strerror(errno));
        goto run_error;
    }
    fseek(f_pcm, 0, SEEK_END);
    pcm_size = ftell(f_pcm);
    fseek(f_pcm, 0, SEEK_SET);
    pcm_data = (char *)malloc(pcm_size);
    if (NULL == pcm_data)
        goto run_error;
    fread((void *)pcm_data, pcm_size, 1, f_pcm);
    fclose(f_pcm);
    f_pcm = NULL;

    //离线语法识别参数设置
    snprintf(asr_params, MAX_PARAMS_LEN - 1,
        "engine_type = local, \
        asr_res_path = %s, sample_rate = %d, \
        grm_build_path = %s, local_grammar = %s, \
        result_type = xml, result_encoding = UTF-8, ",
        ASR_RES_PATH,
        SAMPLE_RATE_16K,
        GRM_BUILD_PATH,
        udata->grammar_id
        );
    session_id = QISRSessionBegin(NULL, asr_params, &errcode);
    if (NULL == session_id)
        goto run_error;
    printf("开始识别...\n");

    //文件方式
    while (1) {
        unsigned int len = 6400;

        if (pcm_size < 12800) {
            len = pcm_size;
            last_audio = 1;
        }

        aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;

        if (0 == pcm_count)
            aud_stat = MSP_AUDIO_SAMPLE_FIRST;

        if (len <= 0)
            break;

        printf(">");
        fflush(stdout);
        errcode = QISRAudioWrite(session_id, (const void *)&pcm_data[pcm_count], len, aud_stat, &ep_status, &rec_status);
        if (MSP_SUCCESS != errcode)
            goto run_error;

        pcm_count += (long)len;
        pcm_size -= (long)len;

        //检测到音频结束
        if (MSP_EP_AFTER_SPEECH == ep_status)
            break;

        usleep(150 * 1000); //模拟人说话时间间隙
    }

    //主动点击音频结束
    QISRAudioWrite(session_id, (const void *)NULL, 0, MSP_AUDIO_SAMPLE_LAST, &ep_status, &rec_status);

    free(pcm_data);
    pcm_data = NULL;

    //获取识别结果
    while (MSP_REC_STATUS_COMPLETE != rss_status && MSP_SUCCESS == errcode) {
        rec_rslt = QISRGetResult(session_id, &rss_status, 0, &errcode);
        usleep(150 * 1000);
    }
    printf("\n识别结束：\n");
    printf("=============================================================\n");
    if (NULL != rec_rslt)
        printf("%s\n", rec_rslt);
    else
        printf("没有识别结果！\n");
    printf("=============================================================\n");

    goto run_exit;

run_error:
    if (NULL != pcm_data) {
        free(pcm_data);
        pcm_data = NULL;
    }
    if (NULL != f_pcm) {
        fclose(f_pcm);
        f_pcm = NULL;
    }
run_exit:
    QISRSessionEnd(session_id, NULL);
    return errcode;
}
#endif

int save(int len)
{

    struct timeval tv;
    int i = 0;
    string fileName = "pcm.data";
    char val[32];

    gettimeofday(&tv, NULL);
    sprintf(val, "%d", tv.tv_sec);
    fileName.append(val);

    LogDebug("(%s:%s)...", __func__, fileName.c_str());

    QFile file(QString::fromStdString(fileName));

    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        LogError("can't open file %s", fileName.c_str());
        return 0;
    }

    file.write(buffer, len);

    LogDebug("write data %d", len);

    file.flush();
    file.close();

}

void test()
{
    XFGetVer();

    XFEnableWakeup(false);

    string xml = "<?xml version='1.0' encoding='utf-8' standalone='yes' ?><nlp>"
      " <version>1.1</version>"
      " <rawtext>前进</rawtext>"
      " <confidence>3</confidence>"
      " <engine>local</engine>"
      " <result>"
      "   <focus>cmdfront</focus>"
      "   <confidence>12</confidence>"
      "   <object>"
      "     <cmdfront id=\"65535\">前进</cmdfront>"
      "   </object>"
      " </result>"
    " </nlp>";

    ParseXML parser;

    parser.parse(xml);
}

int main(int argc, char* argv[])
{
    const char *login_config    = "appid = 592554fc"; //登录参数
    UserData asr_data;
    int ret                     = 0 ;
    char c;

    test();

    getchar();

    ret = MSPLogin(NULL, NULL, login_config); //第一个参数为用户名，第二个参数为密码，传NULL即可，第三个参数是登录参数
    if (MSP_SUCCESS != ret) {
        printf("登录失败：%d\n", ret);
        goto exit;
    }

    memset(&asr_data, 0, sizeof(UserData));
    printf("构建离线识别语法网络...\n");
    ret = build_grammar(&asr_data);  //第一次使用某语法进行识别，需要先构建语法网络，获取语法ID，之后使用此语法进行识别，无需再次构建
    if (MSP_SUCCESS != ret) {
        printf("构建语法调用失败！\n");
        goto exit;
    }
    while (1 != asr_data.build_fini)
        usleep(300 * 1000);
    if (MSP_SUCCESS != asr_data.errcode)
        goto exit;
    printf("离线识别语法网络构建完成，开始识别...\n");

    while (1)
    {
        ret = run_asr(&asr_data);
        if (MSP_SUCCESS != ret) {
            printf("离线语法识别出错: %d \n", ret);
            goto exit;
        }

        printf("更新离线语法词典...\n");
        ret = update_lexicon(&asr_data);  //当语法词典槽中的词条需要更新时，调用QISRUpdateLexicon接口完成更新
        if (MSP_SUCCESS != ret) {
            printf("更新词典调用失败！\n");
            goto exit;
        }
        while (1 != asr_data.update_fini)
            usleep(300 * 1000);
        if (MSP_SUCCESS != asr_data.errcode)
            goto exit;
        printf("更新离线语法词典完成，按任意继续...\n");
        getchar();
//        ret = run_asr(&asr_data);
//        if (MSP_SUCCESS != ret) {
//            printf("离线语法识别出错: %d \n", ret);
//            goto exit;
//        }

    }
exit:
    MSPLogout();
    printf("exit now.\n");

    return 0;
}

//int main(int argc, char *argv[])
//{
////    QCoreApplication a(argc, argv);

////    return a.exec();



//}

void play()
{
    CAlsa player;

    Log("play");
    string fileName = "pcm.data1532541103";
    QFile file(QString::fromStdString(fileName));

    if (!file.open(QIODevice::ReadOnly))
    {
        LogError("can't open file %s", fileName.c_str());
        return ;
    }

    QByteArray array = file.readAll();

    file.close();



    player.setChannel(AUDIO_CHANNEL_MONO);
    player.setSamplebits(AUDIO_SAMPLE_BIT_16);
    player.setRate(AUDIO_SAMPLE_RATE_16K);
    player.open(ALSA_OPER_PLAY);

    player.play((unsigned char*)array.data(), array.length());

    Log("play...");
    sleep(5);

    player.stop();

    Log("play over");
}
