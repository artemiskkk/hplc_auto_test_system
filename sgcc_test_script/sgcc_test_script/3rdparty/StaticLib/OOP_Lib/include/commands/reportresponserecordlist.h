#ifndef REPORTRESPONSERECORDLIST_H
#define REPORTRESPONSERECORDLIST_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 上报若干个记录型对象属性的响应
 */
#ifdef UNIT_TEST
class ReportResponseRecordList : public FrameOOPBase
#else
class OOP_LIB_EXPORT ReportResponseRecordList : public FrameOOPBase
#endif
{
public:
    ReportResponseRecordList();
    ~ReportResponseRecordList() override;

    /**
     * @brief ReportResponseRecordList
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //ReportResponseRecordList(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    ReportResponseRecordList(CtrlField ctrl_field,AddressField address_field);

    PIID piid_;//!< 服务序号-优先级
    QList<OAD> list_oad_;//!< 对应上报的若干个对象属性描述符
    //TimeTagField time_tag_field_;
public:
    /**
     * @brief 将16进制格式数据，解析为数据单元
     * @param data
     */
    void DecodeFrameDataField(QByteArray data) override;

    /**
     * @brief GetOadListSize
     * @return OAD列表元素个数
     */
    uchar GetOadListSize();

private:
    /**
     * @brief 将数据单元编码为16进制格式数据
     * @return 返回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;

    uchar size_;//!< 对象属性描述符个数
};

}
#endif // REPORTRESPONSERECORDLIST_H
