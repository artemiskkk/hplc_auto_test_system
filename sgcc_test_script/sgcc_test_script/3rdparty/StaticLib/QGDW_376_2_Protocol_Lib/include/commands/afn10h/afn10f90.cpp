#include "afn10f90.h"

qgdw_3762_protocol::Afn10F90::Afn10F90()
{
    this->afn_=0x10;
    this->dt1_=0x02;
    this->dt2_=0x0B;
}

qgdw_3762_protocol::Afn10F90::Afn10F90(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,0x02,0x0B)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x10;
    this->dt1_=0x02;
    this->dt2_=0x0B;
}
void qgdw_3762_protocol::Afn10F90::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()!=9)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int j=0;j<6;j++)
        {
            data_unit_up_.node_address_.addr[j]=data.at(5-j);
        }
        this->data_unit_up_.timing_state_=data.at(6);
        this->data_unit_up_.preserve_=data.at(7);
        this->data_unit_up_.threshold_=uchar(data.at(8));
    }
    else
    {
        if(data.length()!=6)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int j=0;j<6;j++)
        {
            data_unit_down_.node_address_.addr[j]=data.at(5-j);
        }
    }
}
QByteArray qgdw_3762_protocol::Afn10F90::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        for(int j=0;j<6;j++)
            data.append(this->data_unit_up_.node_address_.addr[5-j]);
        data.append(this->data_unit_up_.timing_state_);
        data.append(this->data_unit_up_.preserve_);
        data.append(char(this->data_unit_up_.threshold_));
    }
    else
    {
        for(int j=0;j<6;j++)
            data.append(this->data_unit_down_.node_address_.addr[5-j]);
    }
    return  data;
}
