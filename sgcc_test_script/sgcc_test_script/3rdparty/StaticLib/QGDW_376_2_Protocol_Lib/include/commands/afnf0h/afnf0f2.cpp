#include "afnf0f2.h"

qgdw_3762_protocol::AfnF0F2::AfnF0F2()
{
    this->afn_ = char(0xF0);
    this->dt1_ = 0x02;
    this->dt2_ = 0x00;
}

qgdw_3762_protocol::AfnF0F2::AfnF0F2(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
    : Frame3762Base(ctrl_field, info_field, address_field, char(0xF0), 0x02, 0x00)
{
    this->ctrl_field_ = ctrl_field;
    this->address_field_ = address_field;
    this->info_field_ = info_field; //

    this->afn_ = char(0xF0);
    this->dt1_ = 0x02;
    this->dt2_ = 0x00;
}
void qgdw_3762_protocol::AfnF0F2::DecodeFrameDataField(QByteArray data)
{
    if ((this->ctrl_field_.dir & 0x01) == qgdw_3762_protocol::kDirDown)
    {
        if (11 != data.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError, QString("DataField Length Error"));
        }
        this->config_module_id_unit_.id_content_.clear();
        this->config_module_id_unit_.id_content_ = data;
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError, QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::AfnF0F2::EncodeFrameDataField()
{
    QByteArray data;
    // p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("In EncodeFrameDataField, this=%1, id_content addr=%2").arg(reinterpret_cast<quintptr>(this)).arg(reinterpret_cast<quintptr>(&(this->config_module_id_unit_.id_content_))));
    // p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("EncodeFrameDataField this->ctrl_field_.dir %1 ").arg(this->ctrl_field_.dir));
    if ((this->ctrl_field_.dir & 0x01) == qgdw_3762_protocol::kDirDown)
    {

        // p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("idContent raw pointer: %1, size: %2").arg(reinterpret_cast<quintptr>(rawData)).arg(idContent.size()));
        // p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("this->config_module_id_unit_.id_content_.size() = %1").arg(this->config_module_id_unit_.id_content_.size()));
        if (11 != idContent.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError, QString("DataField Length Error"));
        }
        data.append(this->id_content_);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError, QString("Unknow Error"));
    }
    return data;
}
