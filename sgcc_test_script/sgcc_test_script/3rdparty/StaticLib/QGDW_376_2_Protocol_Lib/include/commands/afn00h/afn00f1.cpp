#include "afn00f1.h"
qgdw_3762_protocol::Afn00F1::Afn00F1()
{
    this->afn_=0x00;
    this->dt1_=0x01;
    this->dt2_=0x00;
}
qgdw_3762_protocol::Afn00F1::Afn00F1(const qgdw_3762_protocol::CtrlField ctrl_field,const qgdw_3762_protocol::InfoField info_field,const qgdw_3762_protocol::AddressField address_field)
    :Frame3762Base(ctrl_field,info_field,address_field,0x00,0x01,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//
    memset(this->data_info_,0,6);

    this->afn_=0x00;
    this->dt1_=0x01;
    this->dt2_=0x00;
}

void qgdw_3762_protocol::Afn00F1::DecodeFrameDataField(QByteArray data)
{
    if(data.length()==4)
     {
         memcpy(this->data_info_,data,4);
     }
     else if(data.length()==6)
     {
         memcpy(this->data_info_,data,6);
     }
     else
     {
         throw DecodeException(ExceptionCatalogue::kDataLengthError,"DataField Length != 4 or 6");
     }
}
QByteArray qgdw_3762_protocol::Afn00F1::EncodeFrameDataField()
{    
    QByteArray data = QByteArray::fromRawData(this->data_info_,6);
    return data;
}


