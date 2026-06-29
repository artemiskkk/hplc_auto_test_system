#include "afnf0f1.h"

qgdw_3762_protocol::AfnF0F1::AfnF0F1()
{
    this->afn_ = char(0xF0);
    this->dt1_ = 0x01;
    this->dt2_ = 0x00;
}

qgdw_3762_protocol::AfnF0F1::AfnF0F1(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
    : Frame3762Base(ctrl_field, info_field, address_field, char(0xF0), 0x01, 0x00)
{
    this->ctrl_field_ = ctrl_field;
    this->address_field_ = address_field;
    this->info_field_ = info_field; //

    this->afn_ = char(0xF0);
    this->dt1_ = 0x01;
    this->dt2_ = 0x00;
}
void qgdw_3762_protocol::AfnF0F1::DecodeFrameDataField(QByteArray data)
{
    if ((this->ctrl_field_.dir & 0x01) == qgdw_3762_protocol::kDirDown)
    {
        if (24 != data.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError, QString("DataField Length Error"));
        }
        this->config_chip_id_unit_.id_content_.clear();
        this->config_chip_id_unit_.id_content_ = data;
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError, QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::AfnF0F1::EncodeFrameDataField()
{
    QByteArray data;
    if ((this->ctrl_field_.dir & 0x01) == qgdw_3762_protocol::kDirDown)
    {
        if (24 != this->config_chip_id_unit_.id_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError, QString("DataField Length Error"));
        }
        data.append(this->config_chip_id_unit_.id_content_);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError, QString("Unknow Error"));
    }
    return data;
}
