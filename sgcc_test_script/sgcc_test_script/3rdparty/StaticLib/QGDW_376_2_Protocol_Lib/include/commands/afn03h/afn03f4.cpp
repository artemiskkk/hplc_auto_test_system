#include "afn03f4.h"

qgdw_3762_protocol::Afn03F4::Afn03F4()
{
    this->afn_=0x03;
    this->dt1_=0x08;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn03F4::Afn03F4(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
    :Frame3762Base(ctrl_field,info_field,address_field,0x03,0x08,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->info_field_=info_field;//
    this->address_field_=address_field;
    this->afn_=0x03;
    this->dt1_=0x08;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn03F4::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()!=6)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 6"));
        }
        for(int i=0;i<6;i++)
        {
            this->master_node_address_.addr[i]=data.at(5-i);
        }
    }
    else
    {
        if(data.length() != 0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 0"));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn03F4::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        for(int i=0;i<6;i++)
        {
            data.append(this->master_node_address_.addr[5-i]);
        }
    }
    return  data;
}
