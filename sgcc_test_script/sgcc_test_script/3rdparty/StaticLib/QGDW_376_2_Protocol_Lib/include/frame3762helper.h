#ifndef FRAME3762HELPER_H
#define FRAME3762HELPER_H


#include <memory>
#include <QObject>
#include <QtMath>

#include "afn_def_list.h"
#include "exceptions/decodeexception.h"
#include "frame3762base.h"
#include "datatypedef.h"


using namespace std;

namespace  qgdw_3762_protocol {
enum ProtocolProvince
{
    StateGrid,
    Hunan,
    Shaanxi,
    Beijing,
    Zhejiang,
    STA,
    Controller
};

/**
 * @brief 376.2协议解析工具类
 *
 * 通用的协议解析接口
 *
 */
#ifdef UNIT_TEST
class Frame3762Helper
#else
class QGDW_376_2_PROTOCOL_LIB_EXPORT Frame3762Helper
#endif
{
public:
    /**
     * @brief Frame3762Helper 构造函数
     */
    Frame3762Helper();
    /**
     * @brief 从字节串中，查找376.2报文并解析
     * @verbatim
     * 该函数会从输入的array对象中，查找376.2报文，返回解析后的帧，并从输入的字节串中移除已经解析的字节。
     * **当字节串长度小于9时，直接反回空指针，不做检查
     * 例如:
     * 解析前 Data: 680f004300000025800010400d4516
     * 解析后 Data:
     *
     * 解析前 Data: 010A680f004300000025800010400d451604
     * 解析后 Data: 04
     *
     * 解析前 Data:010A680f00430000002580
     * 解析后 Data:680f00430000002580
     * @endverbatim
     *
     * @param data 待解析的字节串
     * @return 解析成功，返回指向376.2帧对象的智能指针。解析失败，返回空指针
     *
     */
    static shared_ptr<Frame3762Base> DecodeLocalMsg(QByteArray *data,bool &need_continue,ProtocolProvince protocol_province=StateGrid);

private:
    /**
     * @brief 将376.2报文串解析为帧结构
     * @param data 376.2报文字节串
     * @return 解析后的376.2的帧结构对象指针，解析失败，抛出异常
     */
    static shared_ptr<Frame3762Base> Decode3762Frame(QByteArray &data,ProtocolProvince protocol_province);

    /**
     * @brief FnGetDTvalue 由Dt值获得Fn值
     * @param dt1
     * @param dt2
     * @return 返回数据单元标识Fn
     */
    static ushort FnGetDTvalue(char dt1,char dt2);

    /**
     * @brief FnFindBit1Location
     * @param dest_byte dt1的值
     * @return 返回该dt1处于的Fn表格中的行数
     */
    static uchar FnFindBit1Location(uchar dest_byte);

//    static const char kStartFlag = 0x68;
//    static const char kEndFlag = 0x16;
//    static const char kDirDown= 0;
//    static const char kDirUp  = 1;
    /**
     * @brief Decode3762FrameAfn00H AFN=01H 确认∕否认
     * @param ctrl_field
     * @param info_field
     * @param address_field
     * @param data_info
     * @param fn
     * @return 解析后的376.2的帧结构对象指针，解析失败，抛出异常
     */
    static shared_ptr<Frame3762Base> Decode3762FrameAfn00H(CtrlField ctrl_field,InfoField info_field,AddressField address_field,QByteArray data_info,ushort fn);
    /**
     * @brief Decode3762FrameAfn03H AFN=03H 查询数据
     * @param ctrl_field
     * @param info_field
     * @param address_field
     * @param data_info
     * @param fn
     * @return 解析后的376.2的帧结构对象指针，解析失败，抛出异常
     */
    static shared_ptr<Frame3762Base> Decode3762FrameAfn03H(CtrlField ctrl_field,InfoField info_field,AddressField address_field,QByteArray data_info,ushort fn);
};

}
#endif // FRAME3762HELPER_H
