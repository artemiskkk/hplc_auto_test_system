#include "afnf0f16.h"

qgdw_3762_protocol::AfnF0F16::AfnF0F16()
{
    this->afn_=char(0xF0);
    this->dt1_=char(0x80);
    this->dt2_=0x01;
}

qgdw_3762_protocol::AfnF0F16::AfnF0F16(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,char(0xF0),char(0x80),0x01)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=char(0xF0);
    this->dt1_=char(0x80);
    this->dt2_=0x01;
}
void qgdw_3762_protocol::AfnF0F16::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()!=6)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int i=0;i<6;i++)
            this->query_node_address_.addr[i]=data.at(5-i);
    }
    else
    {
        if(data.length()!=9)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->query_meter_box_unit_.report_meter_box_role_=data.at(0);
        for(int i=0;i<6;i++)
            this->query_meter_box_unit_.report_node_address_.addr[i]=data.at(6-i);

        this->query_meter_box_unit_.report_meter_box_id_=ushort(short(data.at(8))<<8)+uchar(data.at(7));
    }
}
QByteArray qgdw_3762_protocol::AfnF0F16::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        for(int i=0;i<6;i++)
            data.append(this->query_node_address_.addr[5-i]);
    }
    else
    {
        data.append(this->query_meter_box_unit_.report_meter_box_role_);
        for(int i=0;i<6;i++)
            data.append(this->query_meter_box_unit_.report_node_address_.addr[5-i]);
        data.append(char(this->query_meter_box_unit_.report_meter_box_id_&0xFF));
        data.append(char(this->query_meter_box_unit_.report_meter_box_id_>>8));
    }
    return  data;
}

