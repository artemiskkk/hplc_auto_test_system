#include "afnf0f15.h"

qgdw_3762_protocol::AfnF0F15::AfnF0F15()
{
    this->afn_=char(0xF0);
    this->dt1_=0x40;
    this->dt2_=0x01;
}

qgdw_3762_protocol::AfnF0F15::AfnF0F15(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,char(0xF0),0x40,0x01)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=char(0xF0);
    this->dt1_=0x40;
    this->dt2_=0x01;
}
void qgdw_3762_protocol::AfnF0F15::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()!=9)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 9"));
        }
        QByteArray temp;
        QByteArray reverse;
        temp = data.mid(0,2);
        reverse.clear();
        reverse.append(temp[1]);
        reverse.append(temp[0]);
        this->vendor_code_ = QString::fromLatin1(reverse);
        temp = data.mid(2,2);
        reverse.clear();
        reverse.append(temp[1]);
        reverse.append(temp[0]);
        this->chip_code_ = QString::fromLatin1(reverse);
        temp = data.mid(4,3);
        reverse.clear();
        reverse.append(temp[2]);
        reverse.append(temp[1]);
        reverse.append(temp[0]);
        this->version_time_ = QString::fromLatin1(reverse.toHex());
        temp = data.mid(7,2);
        reverse.clear();
        reverse.append(temp[1]);
        reverse.append(temp[0]);
        this->version_ = QString::fromLatin1(reverse.toHex());
    }
    else
    {
        if(data.length() != 0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 0"));
        }
    }
}
QByteArray qgdw_3762_protocol::AfnF0F15::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        //QByteArray temp = QByteArray::fromHex(this->vendor_code_.mid(0,2).toLatin1());
        QByteArray temp = this->vendor_code_.mid(0,2).toUpper().toLatin1();
        for(int index = 1;index>-1;index--)
        {
            data.append(temp[index]);
        }

        //temp = QByteArray::fromHex(this->chip_code_.mid(0,2).toLatin1());
        temp = this->chip_code_.mid(0,2).toUpper().toLatin1();
        for(int index = 1;index>-1;index--)
        {
            data.append(temp[index]);
        }

        temp = QByteArray::fromHex(this->version_time_.mid(0,6).toLatin1());
        for(int index = 2;index>-1;index--)
        {
            data.append(temp[index]);
        }

        temp = QByteArray::fromHex(this->version_.mid(0,4).toLatin1());
        for(int index = 1;index>-1;index--)
        {
            data.append(temp[index]);
        }
    }
    return  data;
}

