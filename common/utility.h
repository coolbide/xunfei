#ifndef UTILITY_H
#define UTILITY_H

#include <QObject>
#include <stdio.h>
#include <QTextCodec>
#include <QDateTime>
#include <QString>
#include<time.h>

using namespace std;

enum CRI_LOG_LEVEL
{
    CRI_LOG_LEVEL_NONE = 0,
    CRI_LOG_LEVEL_ERROR,
    CRI_LOG_LEVEL_RUN,
    CRI_LOG_LEVEL_INFO ,
    CRI_LOG_LEVEL_DEBUG,
    CRI_LOG_LEVEL_MAX
};

class Convert : public QObject
{
    Q_OBJECT
public:
    explicit Convert(QObject *parent = 0);
    static unsigned char Hex2BCD(unsigned char hex);
    static unsigned char BCD2Hex(unsigned char bcd);
    static QString BCD2Str(unsigned char* pBCD, int len, char separateFlag);    
    static int Str2BCD(QString& str, unsigned char* pBCD);
    static string BCD2string(unsigned char* pBCD, int len, char separateFlag);
    static int string2BCD(string& str, unsigned char* pBCD);

    static QString getTrainNumLetter(char *data, int len);
    static QString getTrainNumDigital(unsigned char* data,int len);

    static int Hexstring2bytes(string str, unsigned char* data); // 将16进制字符串转化为数字数组 "1002231618A3" --> 0x10 0x02 0x23 0x16 0x18 0xA3
    static int HexChars2bytes(char* pstr, int len, unsigned char* data); // 将16进制字符串数组转化为数字数组 "1002231618A3" --> 0x10 0x02 0x23 0x16 0x18 0xA3
    static string bytes2Hexstring(unsigned char* src, int srcLen); //数字数组转化为16进制字符串，如0x10 0x02 0x23 0x16 0x18 --> "1002231618"
    static int bytes2HEXChars(char *dest, unsigned char* src, int srcLen); //数字数组转化为16进制字符串，如0x10 0x02 0x23 0x16 0x18 --> "1002231618"

    static QString Bytes2IPStr(unsigned char *pData, int len, char separateFlag); // byte array to String; if no separate flag, set separateFlag to 0;
    static string Bytes2IPstring(unsigned char *pData, int len, char separateFlag); // byte array to String; if no separate flag, set separateFlag to 0;
    static int IPStr2Bytes(QString &str, unsigned char *pData, char separateFlag);
    static int IPstring2Bytes(string &str, unsigned char *pData, char separateFlag);

signals:

public slots:

};


int c2i(char ch);
int BCD2DateTime(char* pData, int len, QDateTime &dateTime);
int DataTime2BCD(QDateTime& dataTime, char* pData, int len);
int getDateTimeBCD(unsigned char dateTimeBCD[6]);

QString getTrainNumLetter(unsigned char* data,int len);
QString getTrainNumDigital(unsigned char* data,int len);
QString getStringFromGbk(unsigned char *data, int len);
int getGbkFromString(QString input, unsigned char *output, int &len);
unsigned short GetCRC16(unsigned char *pData,int len); //计算CRC16
unsigned short GetCRC16(unsigned short initCRC, unsigned char *pData,int len); //计算CRC16
unsigned short GetCRC16(QString fileName);

void testTime();
bool updateSysTime(QDateTime &qtime);
bool updateSysTime(time_t& t);
void setSysTime(QDateTime& dateTime);
void setSysTime(time_t &timet);


extern QTextCodec *codec_gbk;
#define StrToGBK(_astring)  codec_gbk->fromUnicode((_astring))
#define StrFromGBK(_astring)    codec_gbk->toUnicode((_astring))

void UTF2GBK(std::string& strUtf8);
void GBK2UTF(std::string& strGbK);

/*
 StrToGBK(string): string - QString, return - QByteArray
    QByteArray array = StrToGBK(string);
    char* pStr = array.data()

 StrFromGBK(string): string - char *, return - QString
    QTextCodec *codec = QTextCodec::codecForName("gb18030");
    QString lineName = codec->toUnicode(pStr); // char* pStr

*/

/*
 * 功能：将十六进制字符串转换为整型(int)数值
 * */
int hex2dec(char *hex);

//对std::string如何去除前后的空格
// trim from start
inline std::string &ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
inline std::string &rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

inline std::string &trim(std::string &s)
{
    return ltrim(rtrim(s));
}


int string2IP(std::string strIP, unsigned char ip[4]); //字符串ip转换成4字节IP return 0: 成功； 非0：失败;
std::string IP2string(unsigned char ip[4]); //4字节IP 转换成 字符串ip
unsigned int HexString2UInt(std::string str);   //16进制字符转换成整型数字
unsigned int NumberString2UInt(std::string str);   //10进制字符转换成整型数字

bool isValidIPAddr(unsigned char *ip, int len = 4);  //IP地址是否有效，全0 或全0xFF的认为无效

int Remove10Check(unsigned char *str,int len);     //CIR2.0协议帧中，去除“传输需要在数据中遇到10后面添加的10”
int Add10Ckeck(unsigned char *str, int len, unsigned char *restr); //CIR2.0协议帧中，传输需要在数据中遇到10后面添加的10

void printbyte(unsigned char *pData, int len);

void getShellResult(const char *shell, string& result); //执行并获取shell脚本返回的内容

extern int logLevel;
extern time_t logtime;
extern struct tm *plogtm;

//printf("%02d%02d %02d:%02d:%02d ",
//(1+plogtm->tm_mon),
//plogtm->tm_mday,


#define Log(fmt...)                   \
    if(logLevel >= CRI_LOG_LEVEL_RUN)						\
    {                                           \
        time(&logtime); \
        plogtm = localtime(&logtime); \
        printf("%02d:%02d:%02d ", \
        plogtm->tm_hour, \
        plogtm->tm_min,  \
        plogtm->tm_sec); \
                         \
        printf(fmt);                            \
        printf("\n");                           \
    }


#if 0
#define LogDebug(fmt...)                        \
    if(logLevel >= CRI_LOG_LEVEL_DEBUG)         \
    {                                           \
        time(&logtime); \
        plogtm = localtime(&logtime); \
        printf("%02d:%02d:%02d ", \
        plogtm->tm_hour, \
        plogtm->tm_min,  \
        plogtm->tm_sec); \
                         \
        printf(fmt);                            \
        printf("\n");                           \
    }
#else
#define LogDebug(fmt...)                        \
    if(logLevel >= CRI_LOG_LEVEL_DEBUG)         \
    {                                           \
        time(&logtime); \
        plogtm = localtime(&logtime); \
        printf("%02d:%02d:%02d ", \
        plogtm->tm_hour, \
        plogtm->tm_min,  \
        plogtm->tm_sec); \
                         \
        printf(fmt);                            \
        printf("\n");                           \
    }
#endif

#define LogError(fmt...)                   \
    if(logLevel >= CRI_LOG_LEVEL_ERROR)						\
    {                                           \
        time(&logtime); \
        plogtm = localtime(&logtime); \
        printf("%02d:%02d:%02d ", \
        plogtm->tm_hour, \
        plogtm->tm_min,  \
        plogtm->tm_sec); \
                         \
        printf(fmt);                            \
        printf("\n");                           \
    }
#endif // UTILITY_H
