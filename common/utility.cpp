#include "utility.h"
#include <stdio.h>
#include <QDateTime>
#include <QDebug>
#include <time.h>
#include <QFile>
#include <unistd.h>
//#include "cirdefines.h"

#define IGNORED_DIFFERENCE_TIME     3
#define TRAINNUM_MAX_LEN            10
#define LOCOMOTIVENUM_MAX_LEN       10
#define TRAINNUM_LETTER_LEN         4
#define TRAINNUM_DIGTIAL_LEN        3
#define IP_HEX_LEN                  4

int logLevel = CRI_LOG_LEVEL_DEBUG;
time_t logtime;
struct tm *plogtm;


QTextCodec *codec_gbk = QTextCodec::codecForName("gb2312");
QTextCodec *codec_utf8= QTextCodec::codecForName("utf-8");

void UTF2GBK(std::string& strUtf8)
{
    QString strUnicode= codec_utf8->toUnicode(strUtf8.c_str());
    QByteArray ByteGb2312= codec_gbk->fromUnicode(strUnicode);

    strUtf8= ByteGb2312.data();
}
void GBK2UTF(std::string& strGbK)
{
    QString strUnicode= codec_gbk->toUnicode(strGbK.c_str());
    QByteArray ByteUtf8= codec_utf8->fromUnicode(strUnicode);

    strGbK = ByteUtf8.data();
}


Convert::Convert(QObject *parent) :
    QObject(parent)
{
}

unsigned char Convert::Hex2BCD(unsigned char hex)
{
    // hex < 100
    unsigned char BCD=0,CH1=0,CH2=0;
    unsigned char ch;
    QString str;

    str.setNum(hex);
    if(str.length()==1)
    {
        str="0"+str;
    }

    ch = str[0].toAscii();
    CH1=((unsigned char)ch<<4)&0xf0;
    ch = str[1].toAscii();
    CH2=(unsigned char)ch & 0x0f;
    BCD=CH1|CH2;

    return BCD;
}

unsigned char Convert::BCD2Hex(unsigned char bcd)
{
    unsigned char CH1=0,CH2=0,Hex=0;

    CH1=(bcd>>4)&0x0f;
    CH2=(bcd&0x0f);
    Hex=CH1*10+CH2;

    return Hex;
}

QString Convert::BCD2Str(unsigned char *pBCD, int len, char separateFlag)
{
    if ((!pBCD) || (len == 0))
        return "";

    int separateLen = 3;
    if (separateFlag == 0)
    {
        separateLen = 2;
    }

    int i = 0;
    char *pStr = new char[len*3+1];
    unsigned char hex;

    memset(pStr, 0, len*3+1);

    for (i = 0; i < len; i++)
    {
        hex = Convert::BCD2Hex(pBCD[i]);
        sprintf(pStr+i*separateLen, "%02d", hex);
        if ((separateFlag != 0) && (i+1 < len))
        {
            sprintf(pStr+i*separateLen + 2, "%c", separateFlag);
        }
    }

    QString str = StrFromGBK(pStr);

    delete pStr;

    return str;
}

string Convert::BCD2string(unsigned char *pBCD, int len, char separateFlag)
{
    if ((!pBCD) || (len == 0))
        return "";

    int separateLen = 3;
    if (separateFlag == 0)
    {
        separateLen = 2;
    }

    int i = 0;
    char *pStr = new char[len*3+1];
    unsigned char hex;

    memset(pStr, 0, len*3+1);

    for (i = 0; i < len; i++)
    {
        hex = Convert::BCD2Hex(pBCD[i]);
        sprintf(pStr+i*separateLen, "%02d", hex);
        if ((separateFlag != 0) && (i+1 < len))
        {
            sprintf(pStr+i*separateLen + 2, "%c", separateFlag);
        }
    }

    string str = pStr;

    delete pStr;

    return str;
}

int Convert::string2BCD(string &str, unsigned char *pBCD)
{
    QString qs = QString::fromStdString(str);
    return Str2BCD(qs, pBCD);
}

QString Convert::Bytes2IPStr(unsigned char *pData, int len, char separateFlag)
{
    if ((!pData) || (len == 0))
        return "";

    string ip = Bytes2IPstring(pData, len, separateFlag);

    return QString::fromStdString(ip);
}

string Convert::Bytes2IPstring(unsigned char *pData, int len, char separateFlag)
{
    if ((!pData) || (len == 0))
        return "";

    int i = 0;
    string ip;
    char one[8];

    for (i = 0; i < len; i++)
    {
        if ((i) && separateFlag)
        {
            ip += separateFlag;
        }
        sprintf(one, "%d", pData[i]);
        ip += one;
    }

    return ip;
}

int Convert::IPStr2Bytes(QString &str, unsigned char *pData, char separateFlag)
{
    QString tmp;

    if (str.length() == 0)
    {
        return 0;
    }

    if (separateFlag == 0)
    {
        return 0;
    }

    int len = 0;
    tmp = "";
    for (int j = 0 ; j < str.length(); j++)
    {
        if (str.at(j) == separateFlag)
        {
            if (tmp.length() > 0)
            {
                pData[len++] = tmp.toInt() & 0xFF;
                tmp = "";
                continue;
            }
        }

        tmp += str.at(j);
    }

    if (tmp.length() > 0)
    {
        pData[len++] = tmp.toInt() & 0xFF;
    }

    return len;
}

int Convert::IPstring2Bytes(string &str, unsigned char *pData, char separateFlag)
{
    QString qs = QString::fromStdString(str);

    return IPStr2Bytes(qs, pData, separateFlag);
}

int Convert::Str2BCD(QString &str, unsigned char *pBCD)
{

    QString tmp;
    QString num;

    if (str.length() == 0)
    {
        return 0;
    }

    if (str.length()%2)
    {
        tmp = "0" + str;
    }
    else
    {
        tmp = str;
    }

    int len = 0;
    unsigned char hex;


    //qDebug() << tmp;

    for (int j = 0 ; j < tmp.length(); j += 2)
    {
        num = tmp.mid(j, 2);
        //qDebug() << num;
        hex = num.toInt() & 0xFF;
        pBCD[len++] = Convert::Hex2BCD(hex);
    }

    return len;
}

QString Convert::getTrainNumLetter(char *data, int len)
{
    QString output = "";
    if(data == NULL || len != TRAINNUM_LETTER_LEN)
    {
        return output;
    }
    char temp[TRAINNUM_MAX_LEN] = {0};
    memcpy(temp,data,len);
    int firstIndex = 0;
    while(*(temp + firstIndex) != ' ')
    {
        ++firstIndex;
    }
    if(firstIndex >= len)
    {
        return output;
    }
    output = QString::fromAscii(temp + firstIndex,len - firstIndex);
    return output;
}

QString Convert::getTrainNumDigital(unsigned char *data, int len)
{
    QString output = "";
    if(data == NULL || len != TRAINNUM_DIGTIAL_LEN)
    {
        return output;
    }
    unsigned temp[TRAINNUM_MAX_LEN] = {0};
    uint num = 0;
    memcpy(temp,data,len);
    num += data[0];
    num += data[1] << 8;
    num += data[2] << 16;

    output = QString::number(num);
    return output;
}
/*
int ascii_2_hex(UBYTE *O_data, UBYTE *N_data, int len)
{
    int i,j,tmp_len;
    UBYTE tmpData;
    UBYTE *O_buf = O_data;
    UBYTE *N_buf = N_data;
    for(i = 0; i < len; i++)
    {
        if ((O_buf[i] >= '0') && (O_buf[i] <= '9'))
        {
            tmpData = O_buf[i] - '0';
        }
        else if ((O_buf[i] >= 'A') && (O_buf[i] <= 'F')) //A....F
        {
            tmpData = O_buf[i] - 0x37;
        }
        else if((O_buf[i] >= 'a') && (O_buf[i] <= 'f')) //a....f
        {
            tmpData = O_buf[i] - 0x57;
        }
        else
        {
            return -1;
        }
        O_buf[i] = tmpData;
    }
    for(tmp_len = 0,j = 0; j < i; j+=2)
    {
        N_buf[tmp_len++] = (O_buf[j]<<4) | O_buf[j+1];
    }
    return tmp_len;
}
*/
int Convert::Hexstring2bytes(string str, unsigned char *data)
{
    if (str.empty())
    {
        return 0;
    }

    if (str.length() % 2)
    {
        //str = "0" + str;
        return 0;
    }

    int i, j;
    int len = str.length();
    string abyte;
    int val;

    memset(data, 0, len/2);
    j = 0;

    for(i = 0; i < len; i += 2)
    {
        abyte = str.substr(i, 2);
        sscanf(abyte.c_str(), "%2x", &val);
        data[j++] = val & 0xFF;
    }


    return j;
}

int Convert::HexChars2bytes(char *pstr, int len, unsigned char *data)
{
    string str = "";

    if (len % 2)
    {
        return 0;
    }

    for (int i = 0; i < len; i++)
    {
        str += pstr[i];
    }

    return Hexstring2bytes(str, data);
}

string Convert::bytes2Hexstring(unsigned char *src, int srcLen)
{
    char dest[4];
    string str = "";

    for (int i = 0; i < srcLen; i++)
    {
        memset(dest, 0, sizeof(dest));
        sprintf(dest, "%02X", src[i]);
        str.append(dest);
    }

    return str;
}

int Convert::bytes2HEXChars(char *dest, unsigned char *src, int srcLen)
{
    for (int i = 0; i < srcLen; i++)
    {
        sprintf(dest+2*i, "%02X", src[i]);
    }

    dest[2*srcLen] = 0;

    return 2*srcLen;
}

bool isStaertOrEnd(unsigned char *szTBuf,int dwlength)
{
     bool bResult=true;
     int count = 0;
     if(dwlength==0)
     {
         return bResult;
     }
     for(int i = dwlength;i-1>=0;i = i-1)
     {
         if(szTBuf[i] == 0x10)
         {
             count++;
         }
         else
         {
             break;
         }
     }
     if(count%2==1)
     {
         bResult = false;
     }
     else
     {
         bResult = true;
     }
     return bResult;

}

void Change10Ckeck(unsigned char *str, int &len)
{
    for(int i=0;i<len;i++)
    {
        if(str[i] == 0x10 && i + 1 < len && str[i + 1] == 0x10)
        {
            memcpy(&str[i],&str[i + 1],len - i - 1);
            len -= 1;
        }
    }
}

int Add10Ckeck(unsigned char *str, int len, unsigned char *restr)
{
    int ret = 0;

    int j = 0;
    for(int i=0;i<len;i++)
    {
        if(str[i]==0x10)
        {
            restr[j] = 0x10;
            j++;
        }
        restr[j] =str[i];
        j++;
    }

    ret = j+1;
    return ret;
}

unsigned short GetCRC16(unsigned char *pData,int len)
{
    unsigned char CRCHi=0x00,CRCLo=0x00;
    int checklen;
    unsigned char BD;
    unsigned short i;
    bool sCF,lCF,hCF;
    unsigned char CRCGXHi=0x10;
    unsigned char CRCGXLo=0x21;
    unsigned short CRC;

    for(checklen=0;checklen<len;checklen++)
    {
        BD=pData[checklen];
        sCF=false;
        lCF=false;
        hCF=false;
        for(i=0;i<8;i++)
        {
            if((BD&0x80)==0x80)  sCF=true;
            if((CRCHi&0x80)==0x80)  hCF=true;
            if((CRCLo&0x80)==0x80)  lCF=true;
            CRCLo=CRCLo<<1;
            CRCHi=CRCHi<<1;
            if(lCF)CRCHi=CRCHi|0x01;
            if(sCF!=hCF)
            {
                CRCHi=CRCHi^CRCGXHi;
                CRCLo=CRCLo^CRCGXLo;
            }
            BD=BD<<1;
            sCF=false;
            lCF=false;
            hCF=false;
        }
    }

    CRC = CRCHi;
    CRC = CRC << 8;
    CRC |=  CRCLo;

    return CRC;
}

unsigned short GetCRC16(unsigned short initCRC, unsigned char *pData,int len)
{
    unsigned char CRCHi=0x00,CRCLo=0x00;
    int checklen;
    unsigned char BD;
    unsigned short i;
    bool sCF,lCF,hCF;
    unsigned char CRCGXHi=0x10;
    unsigned char CRCGXLo=0x21;
    unsigned short CRC;

    CRCHi = (initCRC & 0xFF00) >> 8;
    CRCLo = initCRC & 0xFF;

    for(checklen=0;checklen<len;checklen++)
    {
        BD=pData[checklen];
        sCF=false;
        lCF=false;
        hCF=false;
        for(i=0;i<8;i++)
        {
            if((BD&0x80)==0x80)  sCF=true;
            if((CRCHi&0x80)==0x80)  hCF=true;
            if((CRCLo&0x80)==0x80)  lCF=true;

            CRCLo=CRCLo<<1;
            CRCHi=CRCHi<<1;
            if(lCF)CRCHi=CRCHi|0x01;
            if(sCF!=hCF)
            {
                CRCHi=CRCHi^CRCGXHi;
                CRCLo=CRCLo^CRCGXLo;
            }
            BD=BD<<1;
            sCF=false;
            lCF=false;
            hCF=false;
        }
    }

    CRC = CRCHi;
    CRC = CRC << 8;
    CRC |=  CRCLo;

    return CRC;
}


unsigned short GetCRC16(QString fileName)
{
    QFile file(fileName);
    int len = 0;
    unsigned short crc = 0xFFFF;

    if (!file.open(QIODevice::ReadOnly))
    {
        LogError("can't open file %s", fileName.toLocal8Bit().data());

        return crc;
    }

    len = file.size();

    if (len <= 0)
    {
        LogError("file size is 0 !");
        file.close();

        return crc;
    }


    #define CRI_SW_UPGRADE_PACKAGE_SIZE   512

    unsigned char buf[CRI_SW_UPGRADE_PACKAGE_SIZE];
    crc = 0;

    while (true)
    {
        len = file.read((char*)buf, CRI_SW_UPGRADE_PACKAGE_SIZE);
        if (len > 0)
        {
            crc = GetCRC16(crc, buf, len);
        }
        else
        {
            break;
        }
    }

    file.close();
    return crc;
}

void printbyte(unsigned char *pData, int len)
{
    if ((pData == NULL) || (len <= 0))
        return;

    char* str = new char[len*3+2];

    for (int i = 0; i < len; i++)
    {
        sprintf(str + i*3, "%02X ", pData[i]);
    }

    printf("%s\n", str);

    delete str;
}

void testTime()
{

    time_t now;
    //struct tm *area;
    now = time(NULL);

    qDebug() << "-- local: " << now << endl;


    now += 1000;

    //setSysTime(now);
    updateSysTime(now);

    now = time(NULL);

    qDebug() << "-- local: " << now << endl;

}

bool updateSysTime(QDateTime &qtime)
{
    QDateTime now = QDateTime::currentDateTime();    

    int val = qtime.toTime_t() - now.toTime_t();


    val = abs(val);

    if (val > IGNORED_DIFFERENCE_TIME)
    {
        qDebug() << "time:  recv: " << qtime.toString() << " -- local: " << now.toString();
        //qDebug() << "time:  recv: " << time.toTime_t() << " -- local: " << now.toTime_t();

        setSysTime(qtime);
        LogDebug("update system time");
        return true;
    }
    else
    {
        return false;
    }
}

bool updateSysTime(time_t& t)
{
    time_t now;
    //struct tm *area;
    now = time(NULL);

    if (abs(t - now) > IGNORED_DIFFERENCE_TIME)
    {
        qDebug() << "time:  recv: " << t<< " -- local: " << now << endl;

        setSysTime(t);
        LogDebug("update system time");
        return true;
    }
    else
    {
        return false;
    }
}

void setSysTime(QDateTime &dateTime)
{
    time_t t =dateTime.toTime_t();

    setSysTime(t);
}

void setSysTime(time_t& timet)
{
    stime(&timet);
    system("hwclock -w");
}

int BCD2DateTime(char *pData, int len, QDateTime& dateTime)
{
    int i = 0;

    if (len < 6)
    {
        return -1;
    }

    QDate date;
    QTime time;

    int year, month, day, hour, minute, second;

    year = Convert::BCD2Hex(pData[i++]) + 2000;
    month = Convert::BCD2Hex(pData[i++]);
    day = Convert::BCD2Hex(pData[i++]);
    hour = Convert::BCD2Hex(pData[i++]);
    minute = Convert::BCD2Hex(pData[i++]);
    second = Convert::BCD2Hex(pData[i++]);

    date.setDate(year, month, day);
    time.setHMS(hour, minute, second);

    dateTime.setDate(date);
    dateTime.setTime(time);

    return 0;
}


int DataTime2BCD(QDateTime &dataTime, char *pData, int len)
{
    int i = 0;

    memset(pData, 0, len);

    if (len < 6)
    {
        return -1;
    }
    pData[i++] = Convert::Hex2BCD(dataTime.date().year()%100);
    pData[i++] = Convert::Hex2BCD(dataTime.date().month());
    pData[i++] = Convert::Hex2BCD(dataTime.date().day());
    pData[i++] = Convert::Hex2BCD(dataTime.time().hour());
    pData[i++] = Convert::Hex2BCD(dataTime.time().minute());
    pData[i++] = Convert::Hex2BCD(dataTime.time().second());

    return 0;
}


const unsigned short wCRCTalbeAbs[] =
{
0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401, 0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,
};
unsigned short CRC16_2(unsigned char* pchMsg, unsigned short wDataLen)
{
        unsigned short wCRC = 0xFFFF;
        unsigned short i;
        unsigned char chChar;
        for (i = 0; i < wDataLen; i++)
        {
                chChar = *pchMsg++;
                wCRC = wCRCTalbeAbs[(chChar ^ wCRC) & 15] ^ (wCRC >> 4);
                wCRC = wCRCTalbeAbs[((chChar >> 4) ^ wCRC) & 15] ^ (wCRC >> 4);
        }
        return wCRC;
}

QString getTrainNumLetter(unsigned char *data, int len)
{
    QString output = "";
    if(data == NULL || len != TRAINNUM_LETTER_LEN)
    {
        return output;
    }
    char temp[TRAINNUM_MAX_LEN] = {0};
    memcpy(temp,data,len);
    int firstIndex = 0;
    while(*(temp + firstIndex) != ' ')
    {
        ++firstIndex;
    }
    if(firstIndex >= len)
    {
        return output;
    }
    output = QString::fromAscii(temp + firstIndex,len - firstIndex);
    return output;
}


QString getTrainNumDigital(unsigned char *data, int len)
{
    QString output = "";
    if(data == NULL || len != TRAINNUM_DIGTIAL_LEN)
    {
        return output;
    }
    unsigned temp[TRAINNUM_MAX_LEN] = {0};
    uint num = 0;
    memcpy(temp,data,len);
    num += data[0];
    num += data[1] << 8;
    num += data[2] << 16;

    output = QString::number(num);
    return output;
}

QString getStringFromGbk(unsigned char *data, int len)
{
    if(data == NULL)
    {
        return "";
    }
    if(len == 0)
    {
        return "";
    }
    return codec_gbk->toUnicode((char*)data,len);
}


int getGbkFromString(QString input, unsigned char *output, int &len)
{
    int ret = -1;
    if(input.isNull())
    {
        return ret;
    }
    ret = 0;
    if(input.isEmpty())
    {
        len = 0;
        return ret;
    }
    QByteArray byteArray = codec_utf8->fromUnicode(input);
    len = byteArray.length();
    if(output == NULL)
    {
        return ret;
        //output =NULL ,get len
    }
    memcpy(output,byteArray.data(),len);
    return ret;

}


/*
 * 将字符转换为数值
 * */
int c2i(char ch)
{
        // 如果是数字，则用数字的ASCII码减去48, 如果ch = '2' ,则 '2' - 48 = 2
        if(isdigit(ch))
                return ch - 48;

        // 如果是字母，但不是A~F,a~f则返回
        if( ch < 'A' || (ch > 'F' && ch < 'a') || ch > 'z' )
                return -1;

        // 如果是大写字母，则用数字的ASCII码减去55, 如果ch = 'A' ,则 'A' - 55 = 10
        // 如果是小写字母，则用数字的ASCII码减去87, 如果ch = 'a' ,则 'a' - 87 = 10
        if(isalpha(ch))
                return isupper(ch) ? ch - 55 : ch - 87;

        return -1;
}

/*
 * 功能：将十六进制字符串转换为整型(int)数值
 * */
int hex2dec(char *hex)
{
        int len;
        int num = 0;
        int temp;
        int bits;
        int i;

        // 此例中 hex = "1de" 长度为3, hex是main函数传递的
        len = strlen(hex);

        for (i=0, temp=0; i<len; i++, temp=0)
        {
                // 第一次：i=0, *(hex + i) = *(hex + 0) = '1', 即temp = 1
                // 第二次：i=1, *(hex + i) = *(hex + 1) = 'd', 即temp = 13
                // 第三次：i=2, *(hex + i) = *(hex + 2) = 'd', 即temp = 14
                temp = c2i( *(hex + i) );
                // 总共3位，一个16进制位用 4 bit保存
                // 第一次：'1'为最高位，所以temp左移 (len - i -1) * 4 = 2 * 4 = 8 位
                // 第二次：'d'为次高位，所以temp左移 (len - i -1) * 4 = 1 * 4 = 4 位
                // 第三次：'e'为最低位，所以temp左移 (len - i -1) * 4 = 0 * 4 = 0 位
                bits = (len - i - 1) * 4;
                temp = temp << bits;

                // 此处也可以用 num += temp;进行累加
                num = num | temp;
        }

        // 返回结果
        return num;
}


std::string IP2string(unsigned char ip[4])
{
    char value[64];

    sprintf(value,"%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    return value;
}

int string2IP(std::string strIP, unsigned char ip[4])
{
    int ret = -1;

    memset(ip, 0, 4);

    if(strIP.length()>=7)
    {
      int t=strIP.find(".");
      if(t!=-1)
      {
        ip[0] = atoi(strIP.substr(0,t+1).c_str());
        strIP = strIP.substr(t+1,strIP.length()-t-1);
      }
      else
      {
          return ret;
      }

      t=strIP.find(".");
      if(t!=-1)
      {
         ip[1] = atoi(strIP.substr(0,t+1).c_str());
         strIP = strIP.substr(t+1,strIP.length()-t-1);
      }
      else
      {
          return ret;
      }

      t=strIP.find(".");
      if(t!=-1)
      {
        ip[2] = atoi(strIP.substr(0,t+1).c_str());
        strIP = strIP.substr(t+1,strIP.length()-t-1);
      }
      else
      {
          return ret;
      }
      ip[3] = atoi(strIP.substr(0,strIP.length()).c_str());

      return 0;

    }else{
        memset(ip,0,4);
    }

    return -1;
}

unsigned int HexString2UInt(std::string str)
{
   unsigned int  ret = 0;

   if (str.length() > 8)
   {
       return ret;
   }

   sscanf(str.c_str(), "%x", &ret);

   return ret;
}



unsigned int NumberString2UInt(string str)
{
    unsigned int  ret = 0;

    if (str.length() > 8)
    {
        return ret;
    }

    sscanf(str.c_str(), "%d", &ret);

    return ret;
}

int Remove10Check(unsigned char *str, int len)
{
    for(int i=0;i<len;i++)
    {
        if(str[i]==0x10 && i+1<len && str[i+1]==0x10)
        {
            memcpy(&str[i],&str[i+1],len-i-1);
            len-=1;
        }
    }

    return len;
}


int getDateTimeBCD(unsigned char dateTimeBCD[6])
{
    int i = 0;
    QDateTime now = QDateTime::currentDateTime();

    dateTimeBCD[i++] = Convert::Hex2BCD(now.date().year()%100);
    dateTimeBCD[i++] = Convert::Hex2BCD(now.date().month());
    dateTimeBCD[i++] = Convert::Hex2BCD(now.date().day());
    dateTimeBCD[i++] = Convert::Hex2BCD(now.time().hour());
    dateTimeBCD[i++] = Convert::Hex2BCD(now.time().minute());
    dateTimeBCD[i++] = Convert::Hex2BCD(now.time().second());

    return 0;
}



bool isValidIPAddr(unsigned char *ip, int len)
{
    bool ret = false;

    if ((len != 4) && (len != 6))
    {
        return ret;
    }

    unsigned char addr[6] = {0};
    unsigned char addrss[6] = {0xFF};

    if (memcmp(ip, addr, len) != 0 && memcmp(ip, addrss, len) != 0)
    {
        return true;
    }
    else
    {
        return false;
    }

    return ret;
}

void getShellResult(const char *shell, string& result)
{
    FILE *stream;
    char buf[1024*2] = {0}; //初始化buf,以免后面写如乱码到文件中

    result = "";

    stream = popen(shell , "r" );   //将命令的输出 通过管道读取（“r”参数）到FILE* stream
    fread( buf, sizeof(char), sizeof(buf), stream); //将刚刚FILE* stream的数据流读取到buf中
    pclose( stream );

    result = buf;
}
