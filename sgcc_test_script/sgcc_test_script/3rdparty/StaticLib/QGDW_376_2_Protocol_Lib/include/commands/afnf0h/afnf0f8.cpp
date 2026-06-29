#include "afnf0f8.h"

qgdw_3762_protocol::AfnF0F8::AfnF0F8()
{
    this->afn_=char(0xF0);
    this->dt1_=char(0x80);
    this->dt2_=0x00;
}

qgdw_3762_protocol::AfnF0F8::AfnF0F8(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,char(0xF0),char(0x80),0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=char(0xF0);
    this->dt1_=char(0x80);
    this->dt2_=0x00;
}
void qgdw_3762_protocol::AfnF0F8::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()<4)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data_unit_down_.parameter_id_[0]=data.at(1);
        data_unit_down_.parameter_id_[1]=data.at(0);
        data_unit_down_.parameter_length_=ushort(ushort(data.at(3)<<8)+ushort(data.at(2)));
        if(data_unit_down_.parameter_length_>0)
            data_unit_down_.parameter_content_.append(data.mid(4));
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kUnknow,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::AfnF0F8::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        data.append(data_unit_down_.parameter_id_[1]);
        data.append(data_unit_down_.parameter_id_[0]);
        data.append(char(data_unit_down_.parameter_length_));
        data.append(char(data_unit_down_.parameter_length_>>8));
        if(data_unit_down_.parameter_length_>0)
            data.append(data_unit_down_.parameter_content_);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kUnknow,QString("Unknow Error"));
    }
    return  data;
}
