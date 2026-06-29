#ifndef TIMETAGFIELD_H
#define TIMETAGFIELD_H

#include "datatypebasedef.h"
#include "../OOP_Lib_global.h"

#include "../exceptions/decodeexception.h"


namespace  object_oriented_electic_data_exchange_protocol {


/**
 * @brief 时间标签域
 */
#ifdef UNIT_TEST
class TimeTagField
#else
class OOP_LIB_EXPORT TimeTagField
#endif
{
public:
    TimeTagField();
    TimeTagField(uchar optional);

    uchar optional_;//!< 可选，0：表示无时间标签，1：表示有时间标签
    TimeTag time_tag_;//!< 时间标签

    QByteArray EncodeFrame();
    void DecodeFrame(QByteArray *data);

    /**
     * @brief operator ==  重载运算符“==”
     * @param time_tag_field
     * @return 返回比较结果
     */
    bool operator==(const TimeTagField &time_tag_field) const
    {
        if(this->optional_ == time_tag_field.optional_)
        {
            if(this->optional_ == 0x00)
            {
                return true;
            }
            else
            {
                if(this->time_tag_ == time_tag_field.time_tag_)
                {
                    return true;
                }
                else
                    return false;
            }
        }
        else
            return false;
    }

};

}
#endif // TIMETAGFIELD_H
