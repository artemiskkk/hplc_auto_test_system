#include "afn10f9.h"

qgdw_3762_protocol::Afn10F9::Afn10F9()
{
    this->afn_=0x10;
    this->dt1_=0x01;
    this->dt2_=0x01;
}

qgdw_3762_protocol::Afn10F9::Afn10F9(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,0x01,0x01)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x10;
    this->dt1_=0x01;
    this->dt2_=0x01;
}
void qgdw_3762_protocol::Afn10F9::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()!=2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->network_scale_=ushort((short(data.at(1))<<8))+uchar(data.at(0));
    }
    else
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn10F9::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        data.append(char(this->network_scale_&0xFF));
        data.append(char(this->network_scale_>>8));
    }
    return  data;
}
