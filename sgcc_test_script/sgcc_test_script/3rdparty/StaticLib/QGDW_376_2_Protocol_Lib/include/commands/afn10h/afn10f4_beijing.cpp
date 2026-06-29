#include "afn10f4_beijing.h"

qgdw_3762_protocol::Afn10F4_Beijing::Afn10F4_Beijing()
{
    this->afn_=0x10;
    this->dt1_=0x08;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn10F4_Beijing::Afn10F4_Beijing(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,0x08,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x10;
    this->dt1_=0x08;
    this->dt2_=0x00;


}
void qgdw_3762_protocol::Afn10F4_Beijing::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()!=16)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        memcpy(&this->router_operate_state_unit_.operate_state_word_,data,1);
        this->router_operate_state_unit_.node_total_num_=ushort((short(data.at(2))<<8))+uchar(data.at(1));
        this->router_operate_state_unit_.have_read_node_num_=ushort((short(data.at(4))<<8))+uchar(data.at(3));
        this->router_operate_state_unit_.read_by_relay_node_num_=ushort((short(data.at(6))<<8))+uchar(data.at(5));
        data.remove(0,7);
        memcpy(&this->router_operate_state_unit_.work_switch_,data,1);
        this->router_operate_state_unit_.communication_rate_=ushort((short(data.at(2))<<8))+uchar(data.at(1));
        this->router_operate_state_unit_.relay_level_phase_1_=uchar(data.at(3));
        this->router_operate_state_unit_.relay_level_phase_2_=uchar(data.at(4));
        this->router_operate_state_unit_.relay_level_phase_3_=uchar(data.at(5));

        this->router_operate_state_unit_.work_procedure_phase_1_=data.at(6);
        this->router_operate_state_unit_.work_procedure_phase_2_=data.at(7);
        this->router_operate_state_unit_.work_procedure_phase_3_=data.at(8);
    }
    else
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn10F4_Beijing::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        data.append(*reinterpret_cast<char*>(&this->router_operate_state_unit_.operate_state_word_));
        data.append(char(this->router_operate_state_unit_.node_total_num_&0xFF));
        data.append(char(this->router_operate_state_unit_.node_total_num_>>8));
        data.append(char(this->router_operate_state_unit_.have_read_node_num_&0xFF));
        data.append(char(this->router_operate_state_unit_.have_read_node_num_>>8));
        data.append(char(this->router_operate_state_unit_.read_by_relay_node_num_&0xFF));
        data.append(char(this->router_operate_state_unit_.read_by_relay_node_num_>>8));
        data.append(*reinterpret_cast<char*>(&this->router_operate_state_unit_.work_switch_));
        data.append(char(this->router_operate_state_unit_.communication_rate_&0xFF));
        data.append(char(this->router_operate_state_unit_.communication_rate_>>8));

        data.append(char(this->router_operate_state_unit_.relay_level_phase_1_));
        data.append(char(this->router_operate_state_unit_.relay_level_phase_2_));
        data.append(char(this->router_operate_state_unit_.relay_level_phase_3_));

        data.append(this->router_operate_state_unit_.work_procedure_phase_1_);
        data.append(this->router_operate_state_unit_.work_procedure_phase_2_);
        data.append(this->router_operate_state_unit_.work_procedure_phase_3_);
    }
    return  data;
}
