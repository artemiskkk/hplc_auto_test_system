#include "afn03f21.h"

qgdw_3762_protocol::Afn03F21::Afn03F21()
{
    this->afn_=0x03;
    this->dt1_=0x10;
    this->dt2_=0x02;
}

qgdw_3762_protocol::Afn03F21::Afn03F21(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,0x10,0x02)
{
    this->ctrl_field_=ctrl_field;
    this->info_field_=info_field;//
    this->address_field_=address_field;
    this->afn_=0x03;
    this->dt1_=0x10;
    this->dt2_=0x02;
}
void qgdw_3762_protocol::Afn03F21::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.size()!=4)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error "));
        }
        this->concurrent_parameter_unit_.module_support_max_node_num_=uchar(data.at(0));
        this->concurrent_parameter_unit_.module_support_max_frame_num_=uchar(data.at(1));
        this->concurrent_parameter_unit_.module_support_max_timeout=ushort(short(data.at(3))<<8)+uchar(data.at(2));
    }
    else
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length error "));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn03F21::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        data.append(char(this->concurrent_parameter_unit_.module_support_max_node_num_));
        data.append(char(this->concurrent_parameter_unit_.module_support_max_frame_num_));
        data.append(char(this->concurrent_parameter_unit_.module_support_max_timeout&0xFF));
        data.append(char(this->concurrent_parameter_unit_.module_support_max_timeout>>8));
    }
    return data;
}
