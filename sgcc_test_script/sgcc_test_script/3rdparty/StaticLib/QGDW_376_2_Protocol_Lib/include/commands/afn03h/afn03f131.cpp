#include "afn03f131.h"

qgdw_3762_protocol::Afn03F131::Afn03F131()
{
    this->afn_=0x03;
    this->dt1_=0x04;
    this->dt2_=0x10;
}

qgdw_3762_protocol::Afn03F131::Afn03F131(const qgdw_3762_protocol::CtrlField ctrl_field, qgdw_3762_protocol::InfoField info_field, qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,0x04,0x10)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x03;
    this->dt1_=0x04;
    this->dt2_=0x10;
}
void qgdw_3762_protocol::Afn03F131::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()!=20)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 9"));
        }
        compare_data_.clear();
        this->compare_data_=data;
    }
    else
    {
        if(data.length() != 7)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 0"));
        }
        this->data_unit_down_.cpu_no_=data.at(0);
        data.remove(0,1);
        mempcpy(this->data_unit_down_.compare_start_address_,data,4);
        data.remove(0,4);
        this->data_unit_down_.compare_data_len_=ushort(ushort(data.at(1))<<8)+uchar(data.at(0));
    }
}
QByteArray qgdw_3762_protocol::Afn03F131::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->compare_data_.length() != 20)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 0"));
        }
        data.append(this->compare_data_);
    }
    else
    {
        data.append(this->data_unit_down_.cpu_no_);
        data.append(this->data_unit_down_.compare_start_address_,4);
        data.append(char(this->data_unit_down_.compare_data_len_&0xFF));
        data.append(char(this->data_unit_down_.compare_data_len_>>8));
    }
    return  data;
}
