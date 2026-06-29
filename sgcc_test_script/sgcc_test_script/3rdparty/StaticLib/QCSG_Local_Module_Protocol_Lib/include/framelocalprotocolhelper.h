#ifndef FRAMELOCALPROTOCOLHELPER_H
#define FRAMELOCALPROTOCOLHELPER_H

#include <QObject>
#include <memory>
#include "afn_def_list.h"
#include "exceptions/decodeexception.h"
#include "framelocalprotocolbase.h"

using namespace std;

namespace  qcsg_local_module_protocol {
/**
 * @brief 南网本地通信模块接口协议解析工具类
 *
 * 通用的协议解析接口
 *
 */

#ifdef UNIT_TEST
class FrameLocalProtocolHelper
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT FrameLocalProtocolHelper
#endif
{
public:
    /**
     * @brief 构造函数
     */
    FrameLocalProtocolHelper();

    /**
     * @brief 从字节串中，查找本地通信模块协议报文并解析
     * @verbatim
     * 该函数会从输入的array对象中，查找本地通信模块协议报文，返回解析后的帧，并从输入的字节串中移除已经解析的字节。
     * **当字节串长度小于9时，直接反回空指针，不做检查
     * 例如:
     * 解析前 Data: 680C00400300010300E82F16
     * 解析后 Data:
     *
     * 解析前 Data: 010A680C00400300010300E82F1604
     * 解析后 Data: 04
     *
     * 解析前 Data: 010A680C00400300010300E82F
     * 解析后 Data: 680C00400300010300E82F
     * @endverbatim
     *
     * @param data 待解析的字节串
     * @param need_continue 是否需要继续解析标志
     * @param protocol_type 本地通信模块协议所属协议类型（以省份区分）
     * @return 解析成功，返回指向本地通信模块协议帧对象的智能指针。解析失败，返回空指针
     *
     */
    static shared_ptr<FrameLocalProtocolBase> DecodeLocalMsg(QByteArray *data,bool &need_continue,Qcsg_Protocol_Type protocol_type = Qcsg_Protocol_Type::kStandard);

private:
    /**
     * @brief 将本地通信模块协议报文串解析为帧结构
     * @param data 本地通信模块协议报文字节串
     * @param protocol_type 本地通信模块协议所属协议类型（以省份区分）
     * @return 解析后的本地通信模块协议的帧结构对象指针，解析失败，抛出异常
     */
    static shared_ptr<FrameLocalProtocolBase> DecodeLocalProtocolFrame(QByteArray &data,Qcsg_Protocol_Type protocol_type);



    static const char kStartFlag = 0x68;  //!< 起始位固定为 0x68
    static const char kEndFlag = 0x16;    //!< 终止位固定为 0x16

//    static const char kDirDown= 0;  //!< 传输方向位：由集中器、采集器发出的下行报文
//    static const char kDirUp  = 1;  //!< 传输方向位：通信模块发出的上行报文

//    static const char kNoAddr  = 0; //!< 地址域标识位：不带地址域
//    static const char kHasAddr = 1; //!< 地址域标识位：带地址域
};

}
#endif // FRAMELOCALPROTOCOLHELPER_H
