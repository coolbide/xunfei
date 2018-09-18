#ifndef I2COPER_H
#define I2COPER_H

#include <string>

using namespace std;

//设备地址 0x47(7 bit)。
//实际应用中需添加读写位，对应的 8 字节地址分别是 0x8F（读）和 0x8E（写）。

#define     EEPROM_ADDR    0x47
#define     I2C_READ_ADDR   0x8F
#define     I2C_WRITE_ADDR   0x8E

class I2COper
{
private:
    int i2c_fd;

public:
    I2COper();
    int open(unsigned char addr);
    int close();
    int iic_write(unsigned char reg, unsigned char* val, int len);
    int iic_read(unsigned char reg, unsigned char* val, int len);

};

string XFGetVer();
bool XFEnableWakeup(bool isEnable);

#endif // I2COPER_H
