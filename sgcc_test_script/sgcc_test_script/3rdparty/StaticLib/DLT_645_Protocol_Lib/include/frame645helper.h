#ifndef FRAME645HELPER_H
#define FRAME645HELPER_H


#include <memory>
#include <QObject>
#include <QtMath>

#include "ctrlcode_def_list.h"
#include "exceptions/decodeexception.h"
#include "frame645base.h"
#include "datatypedef.h"


using namespace std;

namespace  dlt_645_Protocol {
/**
 * @brief 645协议解析工具类
 *
 * 通用的协议解析接口
 *
 */
#ifdef UNIT_TEST
class Frame645Helper
#else
class DLT_645_PROTOCOL_LIB_EXPORT Frame645Helper
#endif
{
public:
    Frame645Helper();
    /**
     * @brief 从字节串中，查找645报文并解析
     * @verbatim
     * 该函数会从输入的array对象中，查找645报文，返回解析后的帧，并从输入的字节串中移除已经解析的字节。
     * @endverbatim
     * @param data 待解析的字节串
     * @return 解析成功，返回指向645帧对象的智能指针。解析失败，返回空指针
     */
    static shared_ptr<Frame645Base> DecodeLocalMsg(QByteArray *data,bool &need_continue);


private:
    /**
     * @brief 将645报文串解析为帧结构
     * @param data 645报文字节串
     * @return 解析后的645的帧结构对象指针，解析失败，抛出异常
     */
    static shared_ptr<Frame645Base> Decode645Frame(QByteArray &data);
};

}
#endif // FRAME645HELPER_H
