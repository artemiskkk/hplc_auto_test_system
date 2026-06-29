#include "afnf0f90.h"

qgdw_3762_protocol::AfnF0F90::AfnF0F90()
{
    this->afn_=char(0xF0);
    this->dt1_=0x02;
    this->dt2_=0x0B;
}

qgdw_3762_protocol::AfnF0F90::AfnF0F90(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,char(0xF0),0x02,0x0B)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=char(0xF0);
    this->dt1_=0x02;
    this->dt2_=0x0B;
}
void qgdw_3762_protocol::AfnF0F90::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::AfnF0F90::EncodeFrameDataField()
{
    QByteArray data;
    return  data;
}

