#include "afnf1f2.h"

qgdw_3762_protocol::AfnF1F2::AfnF1F2()
{
    this->afn_=char(0xF1);
    this->dt1_=0x02;
    this->dt2_=0x00;
}

qgdw_3762_protocol::AfnF1F2::AfnF1F2(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,char(0xF1),0x02,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=char(0xF1);
    this->dt1_=0x02;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::AfnF1F2::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()<3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->protocol_type_=data.at(0);
        this->frame_length_=ushort((short(data.at(2))<<8))+uchar(data.at(1));
        if(this->frame_length_!=data.size()-3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->frame_content_.clear();
        this->frame_content_.append(data.mid(3));
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
    }
}
QByteArray qgdw_3762_protocol::AfnF1F2::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(this->frame_length_!=this->frame_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(this->protocol_type_);
        data.append(char(this->frame_length_&0xFF));
        data.append(char(this->frame_length_>>8));
        data.append(this->frame_content_);
    }
    return  data;
}
