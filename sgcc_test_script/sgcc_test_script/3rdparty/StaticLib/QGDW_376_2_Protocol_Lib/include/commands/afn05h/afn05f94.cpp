#include "afn05f94.h"

qgdw_3762_protocol::Afn05F94::Afn05F94()
{
    this->afn_=0x05;
    this->dt1_=0x20;
    this->dt2_=0x0B;
}
qgdw_3762_protocol::Afn05F94::Afn05F94(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x05,0x20,0x0B)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x05;
    this->dt1_=0x20;
    this->dt2_=0x0B;
}
void qgdw_3762_protocol::Afn05F94::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()!=7)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int i=0;i<6;i++)
        {
            this->node_address_ .addr[i]=data.at(5-i);
        }
        this->threshold_=uchar(data.at(6));
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn05F94::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        for(int i=0;i<6;i++)
        {
            data.append(this->node_address_ .addr[5-i]);
        }
        data.append(char(this->threshold_));
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}
