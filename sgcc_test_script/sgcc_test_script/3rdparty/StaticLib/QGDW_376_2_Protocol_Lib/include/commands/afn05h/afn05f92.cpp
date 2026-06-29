#include "afn05f92.h"

qgdw_3762_protocol::Afn05F92::Afn05F92()
{
    this->afn_=0x05;
    this->dt1_=0x08;
    this->dt2_=0x0B;
}
qgdw_3762_protocol::Afn05F92::Afn05F92(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x05,0x08,0x0B)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x05;
    this->dt1_=0x08;
    this->dt2_=0x0B;
}
void qgdw_3762_protocol::Afn05F92::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()<8)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int i=0;i<6;i++)
        {
            this->node_address_ .addr[i]=data.at(5-i);
        }
        this->frame_len_=ushort(ushort(data.at(7))<<8)+ushort(data.at(6));
        data.remove(0,8);
        if(data.length()!=this->frame_len_)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->frame_content_=data;
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn05F92::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        for(int i=0;i<6;i++)
        {
            data.append(this->node_address_ .addr[5-i]);
        }
        data.append(char(this->frame_len_));
        data.append(char(this->frame_len_>>8));
        data.append(this->frame_content_);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}
