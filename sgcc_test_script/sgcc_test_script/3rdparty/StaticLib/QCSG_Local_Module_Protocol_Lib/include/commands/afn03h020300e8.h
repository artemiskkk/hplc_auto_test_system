#ifndef AFN03H020300E8_H
#define AFN03H020300E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn03H020300E8 查询本地通信模块运行模式信息
 */
#ifdef UNIT_TEST
class Afn03H020300E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn03H020300E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn03H020300E8();
    /**
     * @brief Afn03H020300E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn03H020300E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar reserve_:4;             //!< 保留
    uchar commu_type_:4;              //!< 通信方式
    ushort max_protocol_len_;        //!< 最大支持的协议报文长度
    ushort max_file_trans_len_;      //!< 文件传输支持的最大单包长度
    uchar upgrade_wait_time_;       //!< 升级操作等待时间，单位：分钟
    QByteArray address_;             //!< 主节点地址，小端模式
    ushort max_sub_node_count_;      //!< 支持的最大从节点数量
    ushort current_sub_node_count_;  //!< 当前从节点数量
    ushort single_operation_count_;  //!< 支持单次读写从节点信息的最大数量
    QString protocol_date_;          //!< 通信模块接口协议发布日期 ,年月日,例“200516"
    QString vendor_code_;            //!< 厂商代码,例“TC"
    QString chip_code_;              //!< 芯片代码,例“R5"
    QString version_time_;           //!< 版本时间,年月日,例“200516"
    QString version_;                //!< 版本,例“0306"

    //FrameLocalProtocolBase interface
public:
    /**
     * @brief 将16进制格式数据，解析为数据单元
     * @param data
     */
    void DecodeFrameDataField(QByteArray data) override;
private:
    /**
     * @brief 将数据单元编码为16进制格式数据
     * @return 反回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;
};

}
#endif // AFN03H020300E8_H
