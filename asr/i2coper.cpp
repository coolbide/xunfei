#include "i2coper.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "../common/utility.h"


I2COper::I2COper()
{
    i2c_fd = -1;
}

int I2COper::open(unsigned char addr)
{
    int ret = -1;

    ////mutex.lock();

    i2c_fd = ::open("/dev/i2c-3", O_RDWR);

    if(i2c_fd < 0)
    {
        i2c_fd = -1;
        ////mutex.unlock();

        LogError("Error %s", __func__);

        return -1;
    }

    ::ioctl(i2c_fd, I2C_TENBIT, 0); //7 bit address;

    ret = ::ioctl(i2c_fd, I2C_SLAVE, addr); //Use this slave address
    if(ret < 0)
    {
        ::close(i2c_fd);
        i2c_fd = -1;

        LogError("Error I2COper::addr %d", addr);
        //mutex.unlock();
        return -1;
    }

    ::ioctl(i2c_fd, I2C_TIMEOUT, 1);
    ::ioctl(i2c_fd, I2C_RETRIES, 3);


    ////mutex.unlock();
    return 0;
}

int I2COper::close()
{
    ////mutex.lock();


    ::close(i2c_fd);
    i2c_fd = -1;


    ////mutex.unlock();
    return 0;
}

int I2COper::iic_write(unsigned char reg, unsigned char *val, int len)
{
    int ret = -1;
    unsigned char i2c_buf[128];

    if (i2c_fd < 0)
    {
        LogError("I2COper::%s() please open I2C first", __func__);
        return ret;
    }

    i2c_buf[0] = reg;
    if (len > 0)
    {
        memcpy(i2c_buf+1, val, len);
        len++;
    }
    else
    {
        len = 1;
    }

    ret = ::write(i2c_fd,i2c_buf, len);
    if(ret != len)
    {
        LogError("I2COper::%s() write error: %d", __func__, ret);
        return -1;
    }

    return 0;
}

int I2COper::iic_read(unsigned char reg, unsigned char *val, int len)
{
    int ret = -1;

    if ((len <= 0) || !val)
    {
        LogError("I2COper::%s() param error, val %u %d", __func__, (unsigned int)val, len);
        return ret;
    }

    if(i2c_fd == -1)
    {
        LogError("I2COper::%s() please open I2C first", __func__);
        return ret;
    }

    ret = ::write(i2c_fd, &reg, 1);
    if (ret != 1)
    {
        LogError("I2COper::%s() reg %d, error: %d", __func__, reg, ret);
        return ret;
    }

    ret = ::read(i2c_fd, val, len);
    //LogDebug("%s, %d: %d (len %d)", __func__, __LINE__, ret, len);
    //printbyte(val, len);

    if (ret != len)
    {
        LogError("I2COper::%s() read error: %d", __func__, ret);

        ret = -1;
    }

    return ret;
}

string XFGetVer()
{
    int ret;
    I2COper oper;
    string ver = "";
    int len;
    unsigned char buf[128];

    if (0 != oper.open(EEPROM_ADDR))
    {
        return ver;
    }

    buf[0] = 0;
    buf[1] = 0xF;
    buf[2] = 0;
    buf[3] = 0;
    len = 4;
    ret = oper.iic_write(0, buf, len);
    if (ret != 0)
    {
        oper.close();
        LogError("%s write error %d", __func__,ret);
        return ver;
    }

    usleep(10000);

    memset(buf, 0, sizeof(buf));

    len = 4;
    ret = oper.iic_read(0, buf, len);
    LogDebug("%s, %d: %d", __func__, __LINE__, ret);
    printbyte(buf, len);

    memset(buf, 0, sizeof(buf));

    len = 4;
    ret = oper.iic_read(1, buf, len);
    LogDebug("%s, %d: %d", __func__, __LINE__, ret);
    printbyte(buf, len);

    memset(buf, 0, sizeof(buf));

    len = 4;
    ret = oper.iic_read(2, buf, len);
    LogDebug("%s, %d: %d", __func__, __LINE__, ret);
    printbyte(buf, len);


    oper.close();

    return ver;
}

bool XFEnableWakeup(bool isEnable)
{

    int ret = -1;
    I2COper oper;

    int len;
    unsigned char buf[128];

    if (0 != oper.open(EEPROM_ADDR))
    {
        return false;
    }

    buf[0] = 0;
    buf[1] = 0x13;
    if (isEnable)
    {
        buf[2] = 1; //0 表示关闭唤醒， 1 表示开启唤醒
    }
    else
    {
        buf[2] = 0; //0 表示关闭唤醒， 1 表示开启唤醒
    }

    buf[3] = 0;
    len = 4;
    ret = oper.iic_write(0, buf, len);
    if (ret != 0)
    {
        oper.close();
        LogError("write error %d", ret);
        return false;
    }

    memset(buf, 0, sizeof(buf));
    usleep(20000);

    len = 4;
    ret = oper.iic_read(0, buf, len);
    LogDebug("%s, %d:(len %d) %d", __func__, __LINE__, len, ret);
    printbyte(buf, len);


    oper.close();
    return true;
}


