#include "afn04f3.h"

qgdw_3762_protocol::Afn04F3::Afn04F3()
{
    this->afn_=0x04;
    this->dt1_=0x04;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn04F3::Afn04F3(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x04,0x04,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x04;
    this->dt1_=0x04;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn04F3::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()<9)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->communication_test_unit_.test_rate_no_=data.at(0);
        for(int i=0;i<6;i++)
        {
            this->communication_test_unit_.dst_address_.addr[i]=data.at(6-i);
        }
        this->communication_test_unit_.protocol_type_=data.at(7);
        this->communication_test_unit_.frame_length_=uchar(data.at(8));
        data.remove(0,9);
        if(data.length()!=this->communication_test_unit_.frame_length_)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->communication_test_unit_.frame_content.clear();
        this->communication_test_unit_.frame_content.append(data);
    }
    else
    {
        throw DecodeException(ExceptionCatalogue::kDataLengthError,"Unknow Error");
    }
}
QByteArray qgdw_3762_protocol::Afn04F3::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        data.append(this->communication_test_unit_.test_rate_no_);
        for(int i=0;i<6;i++)
        {
            data.append(this->communication_test_unit_.dst_address_.addr[5-i]);
        }
        data.append(this->communication_test_unit_.protocol_type_);
        data.append(char(this->communication_test_unit_.frame_length_));
        data.append(this->communication_test_unit_.frame_content);
    }
    else
    {
        throw DecodeException(ExceptionCatalogue::kDataLengthError,"Unknow Error");
    }
    return  data;
}
