#include "afn03f100.h"


qgdw_3762_protocol::Afn03F100::Afn03F100()
{
    this->afn_=0x03;
    this->dt1_=0x08;
    this->dt2_=0x0C;
}

qgdw_3762_protocol::Afn03F100::Afn03F100(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field,const  qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,0x08,0x0C)
{
    this->ctrl_field_=ctrl_field;
    this->info_field_=info_field;//
    this->address_field_=address_field;
    this->afn_=0x03;
    this->dt1_=0x08;
    this->dt2_=0x0C;
}
void qgdw_3762_protocol::Afn03F100::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()!=20)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }

        for(int i=0;i<6;i++)
        {
            this->address.append(QString::fromLatin1(data.mid(0,6).toHex()).at(5-i));
        }

        memcpy(reinterpret_cast<char*>(&this->unkonwn),data.data()+6,2);

        this->vrn1=static_cast<uchar>(data.at(8));
        for(int i=0;i<3;i++)
        {
            this->data1.append(QString::fromLatin1(data.mid(9,3).toHex()).at(2-i));
        }
        for(int i=0;i<2;i++)
        {
            this->no1.append(QString::fromLatin1(data.mid(12,2).toHex()).at(1-i));
        }
        this->vrn2=static_cast<uchar>(data.at(14));
        for(int i=0;i<3;i++)
        {
            this->data2.append(QString::fromLatin1(data.mid(15,3).toHex()).at(2-i));
        }
        for(int i=0;i<2;i++)
        {
            this->no2.append(QString::fromLatin1(data.mid(18,2).toHex()).at(1-i));
        }

    }
    else
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn03F100::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
      for(int i=0;i<6;i++)
      {
          data.append(QByteArray::fromHex(this->address.toLatin1()).at(5-i));
      }
      data.append((const char*)(&this->unkonwn),2);
      data.append(static_cast<char>(this->vrn1));
      for(int i=0;i<3;i++)
      {
          data.append(QByteArray::fromHex(this->data1.toLatin1()).at(2-i));
      }
      for(int i=0;i<2;i++)
      {
          data.append(QByteArray::fromHex(this->no1.toLatin1()).at(1-i));
      }


    }
    return  data;
}
