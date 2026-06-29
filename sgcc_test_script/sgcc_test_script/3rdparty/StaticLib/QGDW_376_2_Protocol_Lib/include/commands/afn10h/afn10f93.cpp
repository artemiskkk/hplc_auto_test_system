#include "afn10f93.h"

qgdw_3762_protocol::Afn10F93::Afn10F93()
{
    this->afn_=0x10;
    this->dt1_=0x10;
    this->dt2_=0x0B;
}

qgdw_3762_protocol::Afn10F93::Afn10F93(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,0x10,0x0B)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x10;
    this->dt1_=0x10;
    this->dt2_=0x0B;
}
void qgdw_3762_protocol::Afn10F93::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()!=2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->period_=ushort(short(data.at(1))<<8)+uchar(data.at(0));
    }
    else
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn10F93::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        data.append(char(this->period_&0xFF));
        data.append(char(this->period_>>8));
    }
    return  data;
}
