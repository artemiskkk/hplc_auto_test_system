#include "afn15f1.h"

qgdw_3762_protocol::Afn15F1::Afn15F1()
{
    this->afn_=0x15;
    this->dt1_=0x01;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn15F1::Afn15F1(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x15,0x01,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x15;
    this->dt1_=0x01;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn15F1::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()<11)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->file_transfer_unit_.file_identify_=data.at(0);
        this->file_transfer_unit_.file_property_=data.at(1);
        this->file_transfer_unit_.file_instruct_=data.at(2);
        this->file_transfer_unit_.total_num_=ushort((short(data.at(4))<<8))+uchar(data.at(3));
        this->file_transfer_unit_.this_identify_=uint(int(data.at(8))<<24)+uint(int(data.at(7))<<16)+uint((int(data.at(6))<<8))+uchar(data.at(5));
        this->file_transfer_unit_.file_length_=ushort(short(data.at(10))<<8)+uchar(data.at(9));
        this->file_transfer_unit_.file_content_.clear();
        this->file_transfer_unit_.file_content_.append(data.mid(11,this->file_transfer_unit_.file_length_));
    }
    else
    {
        if(data.length()!=4)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->current_identify_=uint(int(data.at(3))<<24)+uint(int(data.at(2))<<16)+uint(int(data.at(1))<<8)+uchar(data.at(0));
    }
}
QByteArray qgdw_3762_protocol::Afn15F1::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(this->file_transfer_unit_.file_length_!=this->file_transfer_unit_.file_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(this->file_transfer_unit_.file_identify_);
        data.append(this->file_transfer_unit_.file_property_);
        data.append(this->file_transfer_unit_.file_instruct_);
        data.append(char(this->file_transfer_unit_.total_num_&0xFF));
        data.append(char(this->file_transfer_unit_.total_num_>>8));
        data.append(char(this->file_transfer_unit_.this_identify_&0xFF));
        data.append(char(this->file_transfer_unit_.this_identify_>>8));
        data.append(char(this->file_transfer_unit_.this_identify_>>16));
        data.append(char(this->file_transfer_unit_.this_identify_>>24));
        data.append(char(this->file_transfer_unit_.file_length_&0xFF));
        data.append(char(this->file_transfer_unit_.file_length_>>8));
        data.append(this->file_transfer_unit_.file_content_);
    }
    else
    {
        data.append(char(this->current_identify_&0xFF));
        data.append(char(this->current_identify_>>8));
        data.append(char(this->current_identify_>>16));
        data.append(char(this->current_identify_>>24));
    }
    return  data;
}
