#include "afn06f66.h"

qgdw_3762_protocol::Afn06F66::Afn06F66()
{
    this->afn_=0x06;
    this->dt1_=0x02;
    this->dt2_=0x08;
}

qgdw_3762_protocol::Afn06F66::Afn06F66(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
    :Frame3762Base(ctrl_field,info_field,address_field,0x06,0x02,0x08)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x06;
    this->dt1_=0x02;
    this->dt2_=0x08;
}
void qgdw_3762_protocol::Afn06F66::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<4)
        {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->frame_type_=uchar(data.at(0));
        this->protocol_=uchar(data.at(1));
        this->frame_len_=uchar(data.at(2))+((data.at(3)&0xff)*256);
        if(this->frame_len_<data.size()-4)
        {
         throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->frame_=data.mid(4,frame_len_);

    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn06F66::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->frame_len_!=this->frame_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(char(this->frame_type_));
        data.append(char(this->protocol_));
        data.append((const char *) (&this->frame_len_),2);
        data.append(this->frame_);

    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}
