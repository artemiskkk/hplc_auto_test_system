#ifndef AFN15F1_H
#define AFN15F1_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class Afn15F1 : public Frame3762Base
#else
/**
 * @brief The AFN15F1 文件传输方式
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn15F1 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn15F1
     */
    Afn15F1();
    /**
     * @brief Afn15F1
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn15F1(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    /**
     * @brief The FileTransfer 文件传输结构体
     */
    struct FileTransferUnit
    {
        char file_identify_;//!<文件标识
        char file_property_;//!<文件属性
        char file_instruct_;//!<文件指令
        ushort total_num_;//!<总段数
        uint this_identify_;//!<第i段标识
        ushort file_length_;//!<第i段数据长度
        QByteArray file_content_;//!<文件数据
        /**
         * @brief operator ==
         * @param unit_
         * @return
         */
        bool operator==(const FileTransferUnit &unit_)const
        {
            if(this->file_identify_==unit_.file_identify_
                    &&this->file_property_==unit_.file_property_
                    &&this->file_instruct_==unit_.file_instruct_
                    &&this->total_num_==unit_.total_num_
                    &&this->this_identify_==unit_.this_identify_
                    &&this->total_num_==unit_.total_num_
                    &&this->this_identify_==unit_.this_identify_)
                return true;
            else
                return false;
        }
    };
    FileTransferUnit file_transfer_unit_;//!<文件传输单元
    uint current_identify_;//!<收到当前段标识
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
#endif // AFN15F1_H
