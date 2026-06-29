#ifndef GETREQUESTRECORDLIST_H
#define GETREQUESTRECORDLIST_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 读取若干个记录型对象属性请求
 */
#ifdef UNIT_TEST
class GetRequestRecordList : public FrameOOPBase
#else
class OOP_LIB_EXPORT GetRequestRecordList : public FrameOOPBase
#endif
{
public:
    GetRequestRecordList();
    ~GetRequestRecordList() override;

    /**
     * @brief GetRequestRecordList
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //GetRequestRecordList(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    GetRequestRecordList(CtrlField ctrl_field,AddressField address_field);

    PIID piid_;//!< 服务序号-优先级
    QList<GetRecord> list_get_record_;//!< 读取若干个记录型对象属性
    //TimeTagField time_tag_field_;

public:
    /**
     * @brief 将16进制格式数据，解析为数据单元
     * @param data
     */
    void DecodeFrameDataField(QByteArray data) override;

    uchar GetGetRecordSize();
private:
    /**
     * @brief 将数据单元编码为16进制格式数据
     * @return 返回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;

    uchar size_;
};

}
#endif // GETREQUESTRECORDLIST_H
