#include "afn06f230.h"
qgdw_3762_protocol::Afn06F230::Afn06F230()
{
    this->afn_=0x06;
    this->dt1_=0x20;
    this->dt2_=0x1C;
}

qgdw_3762_protocol::Afn06F230::Afn06F230(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x06,0x20,0x1C)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x06;
    this->dt1_=0x20;
    this->dt2_=0x1C;
}
void qgdw_3762_protocol::Afn06F230::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<16)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }

        this->task_num_=data.at(0);
        this->address_=data.mid(1,6);
        this->commun_type_=data.at(7);
        this->time_stamp_=data.mid(8,6);
        memcpy(&this->frame_len_,data.data()+14,2);
        data.remove(0,16);
        this->frame_content_=data;
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn06F230::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        data.append(this->task_num_);
        data.append(this->address_);
        data.append(this->commun_type_);
        data.append(this->time_stamp_);
        data.append((const char*)&this->frame_len_,2);
        data.append(this->frame_content_);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}
