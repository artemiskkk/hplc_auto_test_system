#include "afnf0f51.h"

qgdw_3762_protocol::AfnF0F51::AfnF0F51()
{
    this->afn_=char(0xF0);
    this->dt1_=0x04;
    this->dt2_=0x06;
}
qgdw_3762_protocol::AfnF0F51::AfnF0F51(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,char(0xF0),0x04,0x06)
{
    this->ctrl_field_=ctrl_field;
    this->info_field_=info_field;//
    this->address_field_=address_field;
    this->afn_=char(0xF0);
    this->dt1_=0x04;
    this->dt2_=0x06;
}
void qgdw_3762_protocol::AfnF0F51::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.size()!=1)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error "));
        }
        this->frequence_division_=data.at(0);
    }
    else
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length error "));
        }
    }
}
QByteArray qgdw_3762_protocol::AfnF0F51::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        data.append(this->frequence_division_);
    }
    return data;
}
