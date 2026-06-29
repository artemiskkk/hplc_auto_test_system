#include "afn03f12.h"

qgdw_3762_protocol::Afn03F12::Afn03F12()
{
    this->afn_=0x03;
    this->dt1_=0x08;
    this->dt2_=0x01;
}

qgdw_3762_protocol::Afn03F12::Afn03F12(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,0x08,0x01)
{
    this->ctrl_field_=ctrl_field;
    this->info_field_=info_field;//
    this->address_field_=address_field;
    this->afn_=0x03;
    this->dt1_=0x08;
    this->dt2_=0x01;
}
void qgdw_3762_protocol::Afn03F12::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(uchar(data.at(2))!=data.size()-4)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error "));
        }
        this->local_module_id_data_unit_.module_vendor_code_.append(data.at(1));
        this->local_module_id_data_unit_.module_vendor_code_.append(data.at(0));
        this->local_module_id_data_unit_.module_id_length=uchar(data.at(2));
        this->local_module_id_data_unit_.module_id_format=data.at(3);
        this->local_module_id_data_unit_.module_id_content=data.mid(4);
    }
    else
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length error "));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn03F12::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        data.append(this->local_module_id_data_unit_.module_vendor_code_.at(1).toUpper());
        data.append(this->local_module_id_data_unit_.module_vendor_code_.at(0).toUpper());
        data.append(char(this->local_module_id_data_unit_.module_id_length));
        data.append(this->local_module_id_data_unit_.module_id_format);
        data.append(this->local_module_id_data_unit_.module_id_content);
    }
    return data;
}
