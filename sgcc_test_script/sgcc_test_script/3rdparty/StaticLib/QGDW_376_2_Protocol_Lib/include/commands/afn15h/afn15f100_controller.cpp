#include "afn15f100_controller.h"

qgdw_3762_protocol::Afn15F100_Controller::Afn15F100_Controller()
{
    this->afn_=0x15;
    this->dt1_=0x08;
    this->dt2_=0x0C;
}
qgdw_3762_protocol::Afn15F100_Controller::Afn15F100_Controller(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x15,0x08,0x0C)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x15;
    this->dt1_=0x08;
    this->dt2_=0x0C;
}
void qgdw_3762_protocol::Afn15F100_Controller::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn15F100_Controller::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {

    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}
