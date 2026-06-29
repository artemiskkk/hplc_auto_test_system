#ifndef DECODEEXCEPTION_H
#define DECODEEXCEPTION_H
#include <QException>

namespace  dlt_645_Protocol {



/**
 * @brief 异常类型
 */
enum ExceptionCatalogue
{
    kUnknow=0,         //<! 未知异常
    kUnSupportCtrlCode,     //<! 不支持的控制码
    kDataLengthError,  //<! 用户数据域长度错误
    kDataContentError   //<! 数据域内容错误
};

/**
 * @brief 报文错误异常
 */
class DecodeException : public QException
{
public:
    DecodeException(ExceptionCatalogue catalogue,QString msg);  //catalogue 异常类型; msg 描述信息
    QString GetMsg();  //返回描述信息
    ExceptionCatalogue GetCatalogue();  //返回异常类型
    void AddFrameInfo(QByteArray frame);  //@brief 追加原始报文信息; frame 报文信息
    QString GetFrame();  //返回异常报文


public:
    QException *clone() const override;


private:
    QString message_;  //<! 描述信息
    ExceptionCatalogue catalogue_; //<! 异常类型
    QString frame_;    //<! 异常报文
};



}
#endif // DECODEEXCEPTION_H
