#include "afn11f225.h"

qgdw_3762_protocol::Afn11F225::Afn11F225()
{
    this->afn_=0x11;
    this->dt1_=0x01;
    this->dt2_=0x1C;
}

qgdw_3762_protocol::Afn11F225::Afn11F225(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x11,0x01,0x1C)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x11;
    this->dt1_=0x01;
    this->dt2_=0x1C;
}
void qgdw_3762_protocol::Afn11F225::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()!=10)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        memcpy(&this->start_time_,data,6);
        this->last_time_=ushort(short(data.at(7))<<8)+uchar(data.at(6));
        this->retransmit_times_=uchar(data.at(8));
        this->wait_time_slice_num_=uchar(data.at(9));
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn11F225::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        char temp[6];
        memcpy(temp,&this->start_time_,6);
        for(int i=0;i<6;i++)
            data.append(temp[i]);
        data.append(char(this->last_time_&0xFF));
        data.append(char(this->last_time_>>8));
        data.append(char(this->retransmit_times_));
        data.append(char(this->wait_time_slice_num_));
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}

