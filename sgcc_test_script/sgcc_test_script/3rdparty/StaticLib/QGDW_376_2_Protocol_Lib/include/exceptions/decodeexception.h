#ifndef DECODEEXCEPTION_H
#define DECODEEXCEPTION_H
#include <QException>

namespace  qgdw_3762_protocol {

/**
 * @brief 异常类型
 */
enum ExceptionCatalogue
{
    kUnknow=0,         //<! 未知异常
    kFrameLenghErr,    //<! 帧长度错误
    kFieldTypeErr,     //<! 数据类型错误
    kDataLengthError,  //<! 用户数据域长度错误
    kUnSupportAfn,     //<! 不支持的Afn
    kUnSupportDt,       //<! 不支持的dt
    kDataContentError,   //<! 用户数据域内容错误format
    kDataContentFormatError //<! 用户数据域内容格式错误
};

/**
 * @brief 报文错误异常
 */
class DecodeException : public QException
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
    QString message_;  //<! 描述信息
    ExceptionCatalogue catalogue_; //<! 异常类型
    QString frame_;    //<! 异常报文
};

}
#endif // DECODEEXCEPTION_H
