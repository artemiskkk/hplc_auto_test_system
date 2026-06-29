#include "afn10f1.h"

qgdw_3762_protocol::Afn10F1::Afn10F1()
{
    this->afn_=0x10;
    this->dt1_=0x01;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn10F1::Afn10F1(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,0x01,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x10;
    this->dt1_=0x01;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn10F1::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<4)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->node_total_num_=ushort(short(data.at(1))<<8)+uchar(data.at(0));
        this->router_support_max_num_=ushort((short(data.at(3))<<8))+uchar(data.at(2));
        if(this->node_total_num_>this->router_support_max_num_)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
    }
    else
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn10F1::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->node_total_num_>this->router_support_max_num_)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        data.append(char(this->node_total_num_&0xFF));
        data.append(char(this->node_total_num_>>8));
        data.append(char(this->router_support_max_num_&0xFF));
        data.append(char(this->router_support_max_num_>>8));
    }
    return  data;
}
