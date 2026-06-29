#include "afn05f2.h"

qgdw_3762_protocol::Afn05F2::Afn05F2()
{
    this->afn_=0x05;
    this->dt1_=0x02;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn05F2::Afn05F2(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x05,0x02,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x05;
    this->dt1_=0x02;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn05F2::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()!=1)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->event_report_state_flag_=data.at(0);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn05F2::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(!(this->event_report_state_flag_==0x00||this->event_report_state_flag_==0x01))
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        data.append(this->event_report_state_flag_);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}
