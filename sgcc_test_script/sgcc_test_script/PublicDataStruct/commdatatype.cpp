#include "commdatatype.h"


bool rvrsAddr(uchar *src, uchar *des, uchar len)
{
    for(int i=0; i<len; i++)
    {
        des[i]=src[len-1-i];
    }

    return true;
}

bool isArrayEqual(const uchar *dst, uchar *src, uchar len)
{
    if(0==memcmp(dst,src,len))
        return true;
    else
        return false;
}

void reverseAddr(uchar *addr, int len)
{
    uchar tmpAddr[6];
    for(int i=0; i<len; i++)
        tmpAddr[i]=addr[len-1-i];
    memcpy(addr,tmpAddr,uint(len));
}

uchar dec2hex(uchar value)
{
    uchar tmpValue=0;
    tmpValue=value/10*16+value%10;
    return tmpValue;
}

uint bcd2hex(uint value)
{
    bool ok=false;
    uint dstValue=0;
    QString valueStr=("0x"+QString::number(value));
    dstValue=valueStr.toUInt(&ok,16);
    if(false==ok)
        return dstValue;

    return dstValue;
}

uchar calcCs(QByteArray msg)
{
    uchar cs=0;
    for(int i=0; i<msg.size(); i++)
        cs=(cs+(0xff&msg.at(i)))%256;

    return cs;
}

QByteArray reverseArray(QByteArray array)
{
    QByteArray tmpArray;
    for(int i=0; i<array.size(); i++)
        tmpArray.append(array.at(array.size()-1-i));
    return tmpArray;
}
