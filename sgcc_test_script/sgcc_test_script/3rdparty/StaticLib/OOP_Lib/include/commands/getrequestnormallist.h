#ifndef GETREQUESTNORMALLIST_H
#define GETREQUESTNORMALLIST_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 读取若干个对象属性请求
 */
#ifdef UNIT_TEST
class GetRequestNormalList : public FrameOOPBase
#else
class OOP_LIB_EXPORT GetRequestNormalList : public FrameOOPBase
#endif
{
public:
    GetRequestNormalList();
    ~GetRequestNormalList() override;

    /**
     * @brief GetRequestNormalList
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //GetRequestNormalList(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    GetRequestNormalList(CtrlField ctrl_field,AddressField address_field);

    PIID piid_;//!< 服务序号-优先级    
    QList<OAD> list_oad_;//!< 若干个对象属性描述符
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

    uchar oad_size_;//!< 对象属性描述符个数
};

}
#endif // GETREQUESTNORMALLIST_H
