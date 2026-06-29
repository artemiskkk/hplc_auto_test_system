#ifndef CONNECTMECHANISM_H
#define CONNECTMECHANISM_H

#include <QObject>
#include "datatypebasedef.h"
#include "enumerated.h"
#include "../OOP_Lib_global.h"

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 应用连接请求认证的机制信息ConnectMechanismInfo的数据类型定义基类
 */
#ifdef UNIT_TEST
class ConnectMechanismInfoParent
#else
class OOP_LIB_EXPORT ConnectMechanismInfoParent
#endif
{
public:
    ConnectMechanismInfoParent(){}
    ConnectMechanismInfoChoice choice;//!< 应用连接请求认证的机制信息ConnectMechanismInfo种类选择

    /**
     * @brief operator ==  重载运算符“==”
     * @param connect_base
     * @return 返回比较结果
     */
    bool operator==(const ConnectMechanismInfoParent &connect_base) const
    {
        if(this->choice == connect_base.choice)
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param connect_base
     * @return
     */
    ConnectMechanismInfoParent& operator=(const ConnectMechanismInfoParent &connect_base)
    {
        this->choice = connect_base.choice;
        return *this;
    }
};


/**
 * @brief 应用连接请求认证的机制信息:NullSecurity
 */
#ifdef UNIT_TEST
class NullSecurity : public ConnectMechanismInfoParent
#else
class OOP_LIB_EXPORT NullSecurity : public ConnectMechanismInfoParent
#endif
{
public:
    NullSecurity(){}

    /**
     * @brief operator ==  重载运算符“==”
     * @param connect
     * @return 返回比较结果
     */
    bool operator==(const NullSecurity &connect) const
    {
        if(this->choice == connect.choice)
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param connect
     * @return
     */
    NullSecurity& operator=(const NullSecurity &connect)
    {
        this->choice = connect.choice;
        return *this;
    }
};

/**
 * @brief 应用连接请求认证的机制信息:PasswordSecurity
 */
#ifdef UNIT_TEST
class PasswordSecurity : public ConnectMechanismInfoParent
#else
class OOP_LIB_EXPORT PasswordSecurity : public ConnectMechanismInfoParent
#endif
{
public:
    PasswordSecurity(){}

    uchar size;//!< 密码内容字节个数
    QByteArray content;//!< 密码内容

    /**
     * @brief operator ==  重载运算符“==”
     * @param connect
     * @return 返回比较结果
     */
    bool operator==(const PasswordSecurity &connect) const
    {
        if(this->choice == connect.choice
                && this->size == connect.size
                && this->content == connect.content)
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param connect
     * @return
     */
    PasswordSecurity& operator=(const PasswordSecurity &connect)
    {
        this->choice = connect.choice;
        this->size = connect.size;
        this->content = connect.content;

        return *this;
    }
};


/**
 * @brief 应用连接请求认证的机制信息:SymmetrySecurity
 */
#ifdef UNIT_TEST
class SymmetrySecurity : public ConnectMechanismInfoParent
#else
class OOP_LIB_EXPORT SymmetrySecurity : public ConnectMechanismInfoParent
#endif
{
public:
    SymmetrySecurity(){}

    uchar ciphertext_size;//!< 密文1内容字节个数
    QByteArray ciphertext_content;//!< 密文1内容

    uchar signature_size;//!< 客户机签名1内容字节个数
    QByteArray signature_content;//!< 客户机签名1内容

    /**
     * @brief operator ==  重载运算符“==”
     * @param connect
     * @return 返回比较结果
     */
    bool operator==(const SymmetrySecurity &connect) const
    {
        if(this->choice == connect.choice
                && this->ciphertext_size == connect.ciphertext_size
                && this->ciphertext_content == connect.ciphertext_content
                && this->signature_size == connect.signature_size
                && this->signature_content == connect.signature_content)
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param connect
     * @return
     */
    SymmetrySecurity& operator=(const SymmetrySecurity &connect)
    {
        this->choice = connect.choice;
        this->ciphertext_size = connect.ciphertext_size;
        this->ciphertext_content = connect.ciphertext_content;
        this->signature_size = connect.signature_size;
        this->signature_content = connect.signature_content;

        return *this;
    }
};



/**
 * @brief 应用连接请求认证的机制信息:SignatureSecurity
 */
#ifdef UNIT_TEST
class SignatureSecurity : public ConnectMechanismInfoParent
#else
class OOP_LIB_EXPORT SignatureSecurity : public ConnectMechanismInfoParent
#endif
{
public:
    SignatureSecurity(){}

    uchar ciphertext_size;//!< 密文2内容字节个数
    QByteArray ciphertext_content;//!< 密文2内容

    uchar signature_size;//!< 客户机签名2内容字节个数
    QByteArray signature_content;//!< 客户机签名2内容

    /**
     * @brief operator ==  重载运算符“==”
     * @param connect
     * @return 返回比较结果
     */
    bool operator==(const SignatureSecurity &connect) const
    {
        if(this->choice == connect.choice
                && this->ciphertext_size == connect.ciphertext_size
                && this->ciphertext_content == connect.ciphertext_content
                && this->signature_size == connect.signature_size
                && this->signature_content == connect.signature_content)
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param connect
     * @return
     */
    SignatureSecurity& operator=(const SignatureSecurity &connect)
    {
        this->choice = connect.choice;
        this->ciphertext_size = connect.ciphertext_size;
        this->ciphertext_content = connect.ciphertext_content;
        this->signature_size = connect.signature_size;
        this->signature_content = connect.signature_content;

        return *this;
    }
};

/**
 * @brief 认证附加信息
 */
struct SecurityData
{
    RN rn;//!< 服务器随机数
    uchar signature_size;//!< 服务器签名信息字节个数
    QByteArray signature_content;//!< 服务器签名信息

    /**
     * @brief operator ==  重载运算符“==”
     * @param security_data
     * @return 返回比较结果
     */
    bool operator==(const SecurityData &security_data) const
    {
        if(this->rn == security_data.rn
                && this->signature_size == security_data.signature_size
                && this->signature_content == security_data.signature_content)
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param security_data
     * @return
     */
    SecurityData& operator=(const SecurityData &security_data)
    {
        this->rn = security_data.rn;
        this->signature_size = security_data.signature_size;
        this->signature_content = security_data.signature_content;

        return *this;
    }
};

/**
 * @brief 应用连接请求的认证响应信息ConnectResponseInfo的数据类型定义
 */
struct ConnectResponseInfo
{
    ConnectResult result;//!< 认证结果
    uchar optional;//!< 是否有认证附加信息
    SecurityData security_data;//!< 认证附加信息

    /**
     * @brief operator ==  重载运算符“==”
     * @param response
     * @return 返回比较结果
     */
    bool operator==(const ConnectResponseInfo &response) const
    {
        if(this->result == response.result
                && this->optional == response.optional
                && this->security_data == response.security_data)
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param response
     * @return
     */
    ConnectResponseInfo& operator=(const ConnectResponseInfo &response)
    {
        this->result = response.result;
        this->optional = response.optional;
        this->security_data = response.security_data;

        return *this;
    }
};



}

#endif // CONNECTMECHANISM_H
