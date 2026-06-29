#include "afn05f30_zhejiang.h"

qgdw_3762_protocol::Afn05F30_Zhejiang::Afn05F30_Zhejiang()
{
    this->afn_=0x05;
    this->dt1_=0x20;
    this->dt2_=0x03;

}

qgdw_3762_protocol::Afn05F30_Zhejiang::Afn05F30_Zhejiang(const qgdw_3762_protocol::CtrlField ctrl_field,const  qgdw_3762_protocol::InfoField info_field,const  qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x05,0x20,0x03)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x05;
    this->dt1_=0x20;
    this->dt2_=0x03;
}
void qgdw_3762_protocol::Afn05F30_Zhejiang::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()!=4)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->online_lock_time_=ushort(data.at(0))+ushort(ushort(data.at(1))<<8);
        this->leave_lock_time_=ushort(data.at(2))+ushort(ushort(data.at(3))<<8);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn05F30_Zhejiang::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        data.append(char(this->online_lock_time_));
        data.append(char(this->online_lock_time_>>8));
        data.append(char(this->leave_lock_time_));
        data.append(char(this->leave_lock_time_>>8));
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}
