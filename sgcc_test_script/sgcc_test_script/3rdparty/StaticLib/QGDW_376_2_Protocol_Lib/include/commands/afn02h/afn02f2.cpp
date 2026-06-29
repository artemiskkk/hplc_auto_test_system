#include "afn02f2.h"


qgdw_3762_protocol::Afn02F2::Afn02F2()
{
    this->afn_=0x02;
    this->dt1_=0x02;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn02F2::Afn02F2(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
    :Frame3762Base(ctrl_field,info_field,address_field,0x02,0x02,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x02;
    this->dt1_=0x02;
    this->dt2_=0x00;
}

void qgdw_3762_protocol::Afn02F2::DecodeFrameDataField(QByteArray data)
{
    if(data.length()<8)
    {
        throw DecodeException(ExceptionCatalogue::kDataLengthError,"DataField Length < 8");
    }
    this->protocol_type_ = data.at(0);
    this->test_count_=ushort(short(data.at(2))<<8)+uchar(data.at(1));
    this->tmi_ = data.at(3);
    this->block_count_ =uchar(data.at(4));
    this->reserve_ = data.at(5);
    this->frame_length_=ushort(short(data.at(7))<<8)+uchar(data.at(6));

    if(this->frame_length_!=(data.length()-8))
    {
        throw DecodeException(ExceptionCatalogue::kDataLengthError,"报文长度于数据不匹配!");
    }
    this->frame_content_.clear();
    this->frame_content_.append(data.mid(8));
}

QByteArray qgdw_3762_protocol::Afn02F2::EncodeFrameDataField()
{
    QByteArray frame;
    frame.append(this->protocol_type_);
    frame.append(char(this->test_count_&0xFF));
    frame.append(char(this->test_count_>>8));
    frame.append(this->tmi_);
    frame.append(char(this->block_count_));
    frame.append(this->reserve_);

    frame.append(char(this->frame_length_&0xFF));
    frame.append(char(this->frame_length_>>8));
    frame.append(this->frame_content_);

    return frame;
}
