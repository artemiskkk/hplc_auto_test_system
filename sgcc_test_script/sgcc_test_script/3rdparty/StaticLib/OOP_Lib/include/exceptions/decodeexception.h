#ifndef DECODEEXCEPTION_H
#define DECODEEXCEPTION_H

#include <QException>
#include "../OOP_Lib_global.h"

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 异常类型
 */
enum ExceptionCatalogue
{
    kUnknow=0,         //!< 未知异常
    kFrameLenghErr,    //!< 帧长度错误
    kSeverAddrErr,     //!< 服务器地址错误
    kUnSupportSeverType,     //!< 不支持的服务类型
    kUnSupportSeverSubType,     //!< 不支持的服务子类型
    kApduLengthError,  //!< 应用层协议数据单元长度错误
    kDataTypeError,  //!< 应用层协议数据类型错误
    knullptrError,        //!< 野指针错误

    kFieldTypeErr,     //!< 数据类型错误
    kDataLengthError,  //!< 用户数据域长度错误
    kUnSupportAfn,     //!< 不支持的Afn
    kUnSupportDi       //!< 不支持的di
};

/**
 * @brief 报文错误异常
 */
#ifdef UNIT_TEST
class DecodeException : public QException
#else
class OOP_LIB_EXPORT DecodeException : public QException
#endif
{
public:
    /**
     * @brief DecodeException
     * @param catalogue 异常类型
     * @param msg 描述信息
     */
    DecodeException(ExceptionCatalogue catalogue,QString msg);

    /**
     * @brief GetMsg
     * @return 返回描述信息
     */
    QString GetMsg();
    /**
     * @brief GetCatalogue
     * @return 返回异常类型
     */
    ExceptionCatalogue GetCatalogue();

    /**
     * @brief 追加原始报文信息
     * @param frame 报文信息
     */
    void AddFrameInfo(QByteArray frame);
    /**
     * @brief GetFrame
     * @return 返回异常报文
     */
    QString GetFrame();
    // QException interface
public:
    QException *clone() const override;

private:
    QString message_;  //!< 描述信息
    ExceptionCatalogue catalogue_; //!< 异常类型
    QString frame_;    //!< 异常报文

};

}

#endif // DECODEEXCEPTION_H
