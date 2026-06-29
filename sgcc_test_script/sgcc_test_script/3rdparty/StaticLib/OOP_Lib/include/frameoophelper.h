#ifndef FRAMEOOPHELPER_H
#define FRAMEOOPHELPER_H

#include <QObject>
#include <memory>
#include "frameoopbase.h"
#include "apdu_def_list.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 面向对象的用电信息数据交换协议解析工具类
 *
 * 通用的协议解析接口
 *
 */

#ifdef UNIT_TEST
class FrameOOPHelper
#else
class OOP_LIB_EXPORT FrameOOPHelper
#endif
{
public:
    /**
     * @brief 构造函数
     */
    FrameOOPHelper();

    /**
     * @brief 从字节串中，查找面向对象的用电信息数据交换协议报文并解析
     * @verbatim
     * 该函数会从输入的array对象中，查找面向对象的用电信息数据交换协议报文，返回解析后的帧，并从输入的字节串中移除已经解析的字节。
     * **当字节串长度小于14时，直接反回空指针，不做检查
     * 例如:
     * 解析前 Data: 682900430543012107152002DE4805030250040200090103002021020000001002000000200200005C8116
     * 解析后 Data:
     *
     * 解析前 Data: 010A682900430543012107152002DE4805030250040200090103002021020000001002000000200200005C811604
     * 解析后 Data: 04
     *
     * 解析前 Data: 010A682900430543012107152002DE48
     * 解析后 Data: 682900430543012107152002DE48
     * @endverbatim
     *
     * @param data 待解析的字节串
     * @param need_continue 是否需要继续解析标志
     * @return 解析成功，返回指向面向对象的用电信息数据交换协议帧对象的智能指针。解析失败，返回空指针
     *
     */
    static shared_ptr<FrameOOPBase> DecodeMsg(QByteArray *data,bool &need_continue);

private:
    /**
     * @brief 将面向对象的用电信息数据交换协议报文串解析为帧结构
     * @param data 面向对象的用电信息数据交换协议报文字节串
     * @return 解析后的面向对象的用电信息数据交换协议的帧结构对象指针，解析失败，抛出异常
     */
    static shared_ptr<FrameOOPBase> DecodeProtocolFrame(QByteArray &data);

};


}
#endif // FRAMEOOPHELPER_H
