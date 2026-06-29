#ifndef DATATYPEBASEDEF_H
#define DATATYPEBASEDEF_H

#include <QObject>
#include <memory>
#include "enumerated.h"

namespace  object_oriented_electic_data_exchange_protocol {

static const char kStartFlag = 0x68;  //!< 起始位固定为 0x68
static const char kEndFlag = 0x16;    //!< 终止位固定为 0x16

//控制欲
static const char kDirClient = 0;  //!< 传输方向位:此帧是由客户机发出的
static const char kDirServer = 1;  //!< 传输方向位:此帧是由服务器发出的

static const char kPrmClient = 1;  //!< 传输方向位:此帧是由客户机发起的
static const char kPrmServer = 0;  //!< 传输方向位:此帧是由服务器发起的

static const uchar kFrameComplete = 0; //!< 分帧标志位:此帧链路用户数据为完整APDU
static const uchar kFramePart     = 1; //!< 分帧标志位:此帧链路用户数据为APDU片段

static const uchar kScTrue   = 1; //!< 扰码标志位:此帧链路用户数据为扰码编码格式
static const uchar kScFlase    = 0; //!< 扰码标志位:此帧链路用户数据为默认编码格式

static const uchar kFunCodeLinkManage = 1; //!< 功能码:链路管理--链路连接管理（登录，心跳，退出登录）
static const uchar kFunCodeUserData   = 3; //!< 功能码:用户数据--应用连接管理及数据交换服务


//服务类型--预连接
static const uchar LINK_REQUEST    = 1;//!< 预连接请求
static const uchar LINK_RESPONSE        = 129;//!< 预连接响应

//服务类型--客户机
static const uchar CONNECT_REQUEST_CLIENT    = 2;//!< 建立应用连接请求
static const uchar RELEASE_REQUEST_CLIENT    = 3;//!< 断开应用连接请求
static const uchar GET_REQUEST_CLIENT        = 5;//!< 读取请求
static const uchar SET_REQUEST_CLIENT        = 6;//!< 设置请求
static const uchar ACTION_REQUEST_CLIENT     = 7;//!< 操作请求
static const uchar REPORT_RESPONSE_CLIENT    = 8;//!< 上报应答
static const uchar PROXY_REQUEST_CLIENT      = 9;//!< 代理请求
static const uchar ERROR_RESPONSE_CLIENT     = 110;//!< 异常响应
static const uchar SECURITY_RESPONSE_CLIENT  = 16;//!< 安全请求

//服务类型--服务器
static const uchar CONNECT_RESPONSE_SERVER     = 130;//!< 建立应用连接响应
static const uchar RELEASE_RESPONSE_SERVER     = 131;//!< 断开应用连接响应
static const uchar RELEASE_NOTIFICATION_SERVER = 132;//!< 断开应用连接通知
static const uchar GET_RESPONSE_SERVER         = 133;//!< 读取响应
static const uchar SET_RESPONSE_SERVER         = 134;//!< 设置响应
static const uchar ACTION_RESPONSE_SERVER      = 135;//!< 操作响应
static const uchar REPORT_NOTIFICATION_SERVER  = 136;//!< 上报通知
static const uchar PROXY_RESPONSE_SERVER       = 137;//!< 代理响应
static const uchar ERROR_RESPONSE_SERVER       = 238;//!< 异常响应
static const uchar SECURITY_RESPONSE_SERVER    = 144;//!< 安全响应

//服务子类型
/**
 * @brief 读取请求的数据类型（GET-Request）
 */
enum class GetRequestType
{
    kGetRequestNormal=1,   //!< 读取一个对象属性请求
    kGetRequestNormalList, //!< 读取若干个对象属性请求
    kGetRequestRecord,     //!< 读取一个记录型对象属性请求
    kGetRequestRecordList, //!< 读取若干个记录型对象属性请求
    kGetRequestNext,       //!< 读取分帧响应的下一个数据块请求
    kGetRequestMD5         //!< 读取一个对象属性的MD5值
};
/**
 * @brief 读取响应的数据类型（GET-Response）
 */
enum class GetResponseType
{
    kGetResponseNormal=1, //!< 读取一个对象属性的响应
    kGetResponseNormalList, //!< 读取若干个对象属性的响应
    kGetResponseRecord,//!< 读取一个记录型对象属性的响应
    kGetResponseRecordList, //!< 读取若干个记录型对象属性的响应
    kGetResponseNext, //!< 分帧响应一个数据块
    kGetResponseMD5 //!< 读取一个对象属性的MD5值的响应
};
/**
 * @brief 设置请求的数据类型（SET-Request）
 */
enum class SetRequestType
{
    kSetRequestNormal=1,//!< 设置一个对象属性请求
    kSetRequestNormalList,//!< 设置若干个对象属性请求
    kSetThenGetRequestNormalList//!< 设置后读取若干个对象属性请求
};
/**
 * @brief 设置响应的数据类型（SET-Response）
 */
enum class SetResponseType
{
    kSetResponseNormal=1,//!< 设置一个对象属性的确认信息响应
    kSetResponseNormalList,//!< 设置若干个对象属性的确认信息响应
    kSetThenGetResponseNormalList//!< 设置的确认信息以及读取的响应
};
/**
 * @brief 操作请求的数据类型（ACTION-Request）
 */
enum class ActionRequestType
{
    kActionRequest=1,//!< 操作一个对象方法请求
    kActionRequestList,//!< 操作若干个对象方法请求
    kActionThenGetRequestNormalList//!< 操作若干个对象方法后读取若干个对象属性请求
};
/**
 * @brief 操作响应的数据类型（ACTION-Response）
 */
enum class ActionResponseType
{
    kActionResponseNormal=1,//!< 操作一个对象方法的响应
    kActionResponseNormalList,//!< 操作若干个对象方法的响应
    kActionThenGetResponseNormalList//!< 操作若干个对象方法后读取若干个属性的响应
};
/**
 * @brief 上报通知的数据类型（REPORT-Notification）
 */
enum class ReportNotificationType
{
    kReportNotificationList=1,//!< 上报若干个对象属性
    kReportNotificationRecordList,//!< 上报若干个记录型对象属性
    kReportNotificationTransData, //!< 上报透明数据
    kReportRequestClientService, //!<请求客户机服务
    ReportNotificationConsistentRecord,//!<通知上报连贯的记录型对象属性
    kReportNotificationSimplifyRecord //!<通知上报精简的记录型对象
};
/**
 * @brief 上报响应的数据类型（REPORT-Response）
 */
enum class ReportResponseType
{
    kReportResponseList=1,//!< 上报若干个对象属性的响应
    kReportResponseRecordList,//!< 上报若干个记录型对象属性的响应
    kReportResponseTransData//!< 上报透明数据的响应
};
/**
 * @brief 代理请求的数据类型（PROXY-Request）
 */
enum class ProxyRequestType
{
    kProxyGetRequestList=1,//!< 代理读取若干个服务器的若干个对象属性请求
    kProxyGetRequestRecord,//!< 代理读取一个服务器的一个记录型对象属性请求
    kProxySetRequestList,//!< 代理设置若干个服务器的若干个对象属性请求
    kProxySetThenGetRequestList,//!< 代理设置后读取若干个服务器的若干个对象属性请求
    kProxyActionRequestList,//!< 代理操作若干个服务器的若干个对象方法请求
    kProxyActionThenGetRequestList,//!< 代理操作后读取若干个服务器的若干个对象方法和属性请求
    kProxyTransCommandRequest,//!< 代理透明转发命令请求
    kProxyInnerTransCommandRequest//!< 请求代理设备内部透明转发
};
/**
 * @brief 代理响应的数据类型（PROXY-Response）
 */
enum class ProxyResponseType
{
    kProxyGetResponseList=1,//!< 代理读取若干个服务器的若干个对象属性响应
    kProxyGetResponseRecord,//!< 代理读取一个服务器的一个记录型对象属性响应
    kProxySetResponseList,//!< 代理设置若干个服务器的若干个对象属性响应
    kProxySetThenGetResponseList,//!< 代理设置后读取若干个服务器的若干个对象属性响应
    kProxyActionResponseList,//!< 代理操作若干个服务器的若干个对象方法响应
    kProxyActionThenGetResponseList,//!< 代理操作后读取若干个服务器的若干个对象方法和属性响应
    kProxyTransCommandResponse,//!< 代理透明转发命令响应
    kProxyInnerTransCommandResponse,//!< 响应代理设备内部透明转发命令
};


/**
 * @brief 长度域
 */
struct LengthField
{
    uchar reserve:2;     //!< 保留
    ushort length:14;    //!< 用户数据长度

    /**
     * @brief operator ==  重载运算符“==”
     * @param len_field
     * @return 返回比较结果
     */
    bool operator==(const LengthField &len_field) const
    {
        if(this->reserve == len_field.reserve
                && this->length == len_field.length
                )
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param len_field
     * @return
     */
    LengthField& operator=(const LengthField &len_field)
    {
        this->reserve = len_field.reserve;
        this->length = len_field.length;

        return *this;
    }
};


/**
 * @brief 控制域
 */
struct CtrlField
{
    uchar dir:1;     //!< 传输方向位：DIR=0表示此帧是由客户机发出的；DIR=1表示此帧是由服务器发出的
    uchar prm:1;     //!< 启动标志位：PRM=1表示此帧是由客户机发起的；PRM=0表示此帧是由服务器发起的
    uchar fra:1;     //!< 分帧标志位：FRA=1，表示此帧链路用户数据为APDU片段，收齐所有片段按片段序号合并后为完整APDU；FRA=0表示此帧链路用户数据为完整APDU
    uchar res:1;     //!< 保留
    uchar sc:1;      //!< 扰码标志位：SC=1，表示此帧链路用户数据为扰码编码格式，发送时链路用户数据按字节加33H；SC=0，表示此帧链路用户数据为默认编码格式，链路用户数据不加33H。响应数据帧扰码编码格式应与请求/上报数据帧扰码编码格式一致
    uchar func:3;    //!< 功能码,采用BIN编码

    /**
     * @brief operator ==  重载运算符“==”
     * @param ctrl_field
     * @return 返回比较结果
     */
    bool operator==(const CtrlField &ctrl_field) const
    {
        if(this->dir == ctrl_field.dir
                && this->prm == ctrl_field.prm
                && this->fra == ctrl_field.fra
                && this->res == ctrl_field.res
                && this->sc == ctrl_field.sc
                && this->func == ctrl_field.func
                )
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param ctrl_field
     * @return
     */
    CtrlField& operator=(const CtrlField &ctrl_field)
    {
        this->dir = ctrl_field.dir;
        this->prm = ctrl_field.prm;
        this->fra = ctrl_field.fra;
        this->res = ctrl_field.res;
        this->sc = ctrl_field.sc;
        this->func = ctrl_field.func;

        return *this;
    }
};


/**
 * @brief 服务器通信地址
 */
struct ServerAddress
{
    uchar addr_type:2;   //!< 地址类型:为服务器地址的地址类型，0表示单地址，1表示通配地址，2表示组地址，3表示广播地址
    uchar logic_addr:2;  //!< 逻辑地址
    uchar addr_len:4;    //!< 地址长度N:地址的字节数，取值范围：0…15，对应表示1…16个字节长度
    QString address;     //!< 地址

    /**
     * @brief operator ==  重载运算符“==”
     * @param s_addr
     * @return 返回比较结果
     */
    bool operator==(const ServerAddress &s_addr) const
    {
        if(this->addr_type == s_addr.addr_type
                && this->logic_addr == s_addr.logic_addr
                && this->addr_len == s_addr.addr_len
                && this->address == s_addr.address
                )
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param s_addr
     * @return
     */
    ServerAddress& operator=(const ServerAddress &s_addr)
    {
        this->addr_type = s_addr.addr_type;
        this->logic_addr = s_addr.logic_addr;
        this->addr_len = s_addr.addr_len;
        this->address = s_addr.address;

        return *this;
    }
};

/**
 * @brief 客户机通信地址
 */
struct ClientAddress
{
    uchar address;   //!< 地址

    /**
     * @brief operator ==  重载运算符“==”
     * @param c_addr
     * @return 返回比较结果
     */
    bool operator==(const ClientAddress &c_addr) const
    {
        if(this->address == c_addr.address)
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param c_addr
     * @return
     */
    ClientAddress& operator=(const ClientAddress &c_addr)
    {
        this->address = c_addr.address;
        return *this;
    }
};

/**
 * @brief 地址域
 */
struct AddressField
{
    ServerAddress sa;   //!< 服务器地址SA
    ClientAddress ca;   //!< 客户机地址CA

    /**
     * @brief operator ==  重载运算符“==”
     * @param addr_field
     * @return 返回比较结果
     */
    bool operator==(const AddressField &addr_field) const
    {
        if(this->sa == addr_field.sa
                && this->ca == addr_field.ca
                )
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param addr_field
     * @return
     */
    AddressField& operator=(const AddressField &addr_field)
    {
        this->sa = addr_field.sa;
        this->ca = addr_field.ca;
        return *this;
    }
};


/**
 * @brief 分帧格式域
 */
struct FrameFormatField
{
    uchar frame_type:2;    //!< 分帧传输类型：0-数据起始帧，1-最后帧，2-确认帧（确认帧不包含APDU片段域），3-中间帧
    uchar reserve:2;     //!< 保留
    ushort frame_seq:12;   //!< 分帧传输过程的帧序号，取值范围0…4095，循环使用

    /**
     * @brief operator ==  重载运算符“==”
     * @param frame_field
     * @return 返回比较结果
     */
    bool operator==(const FrameFormatField &frame_field) const
    {
        if(this->frame_type == frame_field.frame_type
                && this->reserve == frame_field.reserve
                && this->frame_seq == frame_field.frame_seq
                )
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param frame_field
     * @return
     */
    FrameFormatField& operator=(const FrameFormatField &frame_field)
    {
        this->frame_type = frame_field.frame_type;
        this->reserve = frame_field.reserve;
        this->frame_seq = frame_field.frame_seq;
        return *this;
    }
};





/**
 * @brief 数据定义
 */
//struct Data
//{
//    DataType type;//!< 数据类型
//    uchar length;//!< 数据长度或者成员个数
//    QByteArray data_array;//!< 数据
//    QList<Data> data_member;//!< 数据成员

//    /**
//     * @brief operator ==  重载运算符“==”
//     * @param data
//     * @return 返回比较结果
//     */
//    bool operator==(const Data &data) const
//    {
//        if(this->type == data.type
//                && this->length == data.length)
//        {
//            if(this->type == DataType::kArray || this->type == DataType::kStructure)
//            {
//                if(this->data_member == data.data_member)
//                    return true;
//                else
//                    return false;
//            }
//            else if(this->type == DataType::kNULL)
//            {
//                return true;
//            }
//            else
//            {
//                if(this->data_array == data.data_array)
//                    return true;
//                else
//                    return false;
//            }
//        }
//        else
//            return false;
//    }

//    /**
//     * @brief operator =  重载运算符“=”
//     * @param data
//     * @return
//     */
//    Data& operator=(const Data &data)
//    {
//        this->type = data.type;
//        this->length = data.length;
//        if(this->type == DataType::kArray || this->type == DataType::kStructure)
//        {
//            this->data_member = data.data_member;
//        }
//        else if(this->type == DataType::kNULL)
//        {

//        }
//        else
//        {
//            this->data_array = data.data_array;
//        }
//        return *this;
//    }
//};



/**
 * @brief
 * APDU序号及优先标志PIID（Priority and Invoke ID）的数据类型定义
 *
 * @details
 * PIID是用于客户机APDU（Client-APDU）的各服务数据类型中，基本定义如下，更具体应用约定应根据实际系统要求而定
 */
struct PIID
{
    uchar serve_priority:1;    //!< 服务优先级：0：一般的，1：高级的
    uchar reserve:1;    //!< 保留
    uchar serve_seq:6;    //!< 服务序号：二进制编码表示0…63

    /**
     * @brief operator =  重载运算符“=”
     * @param piid
     * @return
     */
    PIID& operator=(const PIID &piid)
    {
        this->serve_priority = piid.serve_priority;
        this->reserve = piid.reserve;
        this->serve_seq = piid.serve_seq;
        return *this;
    }

    /**
     * @brief operator ==  重载运算符“==”
     * @param piid
     * @return 返回比较结果
     */
    bool operator==(const PIID &piid) const
    {
        if(this->serve_priority == piid.serve_priority
                && this->reserve == piid.reserve
                && this->serve_seq == piid.serve_seq)
            return true;
        else
            return false;
    }
};


/**
 * @brief
 * 带ACD标志位的APDU序号及优先标志PIID-ACD（Priority and Invoke ID with ACD）数据类型定义
 *
 * @details
 * PIID-ACD是用于服务器APDU（Server-APDU）的各服务数据类型中，基本定义如下，更具体应用约定应根据实际系统要求而定
 */
struct PIID_ACD
{
    uchar serve_priority:1;    //!< 服务优先级: 0：一般的，1：高级的
    uchar request_acd:1;    //!< 请求访问ACD: 0：不请求，1：请求
    uchar serve_seq:6;    //!< 服务序号:二进制编码表示0…63

    /**
     * @brief operator =  重载运算符“=”
     * @param piid_acd
     * @return
     */
    PIID_ACD& operator=(const PIID_ACD &piid_acd)
    {
        this->serve_priority = piid_acd.serve_priority;
        this->request_acd = piid_acd.request_acd;
        this->serve_seq = piid_acd.serve_seq;
        return *this;
    }

    /**
     * @brief operator ==  重载运算符“==”
     * @param piid
     * @return 返回比较结果
     */
    bool operator==(const PIID_ACD &piid_acd) const
    {
        if(this->serve_priority == piid_acd.serve_priority
                && this->request_acd == piid_acd.request_acd
                && this->serve_seq == piid_acd.serve_seq)
            return true;
        else
            return false;
    }
};

/**
 * @brief 对象标识数据类型OI定义
 */
struct OI
{
    ushort oi; //!< 对象标识数据
    /**
     * @brief operator ==  重载运算符“==”
     * @param o_i
     * @return 返回比较结果
     */
    bool operator==(const OI &o_i) const
    {
        if(this->oi == o_i.oi)
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param o_i
     * @return
     */
    OI& operator=(const OI &o_i)
    {
        this->oi = o_i.oi;
        return *this;
    }

};

/**
 * @brief
 * 对象属性描述符OAD--属性标识及其特征
 */
struct Attribute
{
    uchar feature:4;    //!< 属性特征,属性特征是对象同一个属性在不同快照环境下取值模式，取值0…7，特征含义在具体类属性中描述
    uchar seq:4;        //!< 对象属性编号,其中0表示整个对象属性，即对象的所有属性

    /**
     * @brief operator ==  重载运算符“==”
     * @param attribute
     * @return 返回比较结果
     */
    bool operator==(const Attribute &attribute) const
    {
        if(this->feature == attribute.feature
                && this->seq == attribute.seq
                )
            return true;
        else
            return false;
    }

    /**
     * @brief operator !=  重载运算符“==”
     * @param attribute
     * @return 返回比较结果
     */
    bool operator!=(const Attribute &attribute) const
    {
        if(this->feature != attribute.feature
                || this->seq != attribute.seq
                )
            return true;
        else
            return false;
    }

};

/**
 * @brief
 * 对象属性描述符OAD（Object Attribute Descriptor）的数据类型定义
 */
struct OAD
{
    ushort OI;  //!< 对象标识
    Attribute attribute;  //!< 属性标识及其特征
    uchar element_index;  //!< 属性内元素索引:00H表示整个属性全部内容。如果属性是一个结构或数组，01H指向对象属性的第一个元素；如果属性是一个记录型的存储区，非零值n表示最近第n次的记录

    QByteArray EncodeFrame(){
        QByteArray oad;
        oad.append(static_cast<char>(this->OI>>8));
        oad.append(static_cast<char>(this->OI));
        oad.append(static_cast<char>((this->attribute.feature<<4) | this->attribute.seq));
        oad.append(static_cast<char>(this->element_index));
        return oad;
    }

    /**
     * @brief operator ==  重载运算符“==”
     * @param oad
     * @return 返回比较结果
     */
    bool operator==(const OAD &oad) const
    {
        if(this->OI == oad.OI
                && this->attribute == oad.attribute
                && this->element_index == oad.element_index
                )
            return true;
        else
            return false;
    }

    /**
     * @brief operator !=  重载运算符“==”
     * @param oad
     * @return 返回比较结果
     */
    bool operator!=(const OAD &oad) const
    {
        if(this->OI != oad.OI
                || this->attribute != oad.attribute
                || this->element_index != oad.element_index
                )
            return true;
        else
            return false;
    }

};

/**
 * @brief
 * 记录型对象属性描述符ROAD（Record Object Attribute Descriptor）的数据类型定义
 *
 * @details
 * ROAD用于描述记录型对象中的一个或若干个关联对象属性
 */
struct ROAD
{
    OAD oad;             //!< 对象属性描述符
    uchar size;          //!< 关联对象属性描述符个数
    QList<OAD> list_oad; //!< 关联对象属性描述符

    QByteArray EncodeFrame(){
        QByteArray road;
        road.append(this->oad.EncodeFrame());
        road.append(static_cast<char>(this->list_oad.size()));
        foreach (OAD oad_temp, this->list_oad) {
            road.append(oad_temp.EncodeFrame());
        }

        return road;
    }

    /**
     * @brief operator ==  重载运算符“==”
     * @param road
     * @return 返回比较结果
     */
    bool operator==(const ROAD &road) const
    {
        if(this->oad == road.oad
                && this->list_oad == road.list_oad
                )
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param road
     * @return
     */
    ROAD& operator=(const ROAD &road)
    {
        this->oad = road.oad;
        this->list_oad = road.list_oad;
        return *this;
    }
};


/**
 * @brief
 * 对象方法描述符OMD（Object Method Descriptor）的数据类型定义
 */
struct OMD
{
    ushort OI;  //!< 对象标识
    uchar method_index;  //!< 方法标识——即对象方法编号
    uchar operate_mode;  //!< 操作模式——值默认为0

    QByteArray EncodeFrame(){
        QByteArray omd;
        omd.append(static_cast<char>(this->OI>>8));
        omd.append(static_cast<char>(this->OI));
        omd.append(static_cast<char>(this->method_index));
        omd.append(static_cast<char>(this->operate_mode));
        return omd;
    }

    /**
     * @brief operator ==  重载运算符“==”
     * @param omd
     * @return 返回比较结果
     */
    bool operator==(const OMD &omd) const
    {
        if(this->OI == omd.OI
                && this->method_index == omd.method_index
                && this->operate_mode == omd.operate_mode
                )
            return true;
        else
            return false;
    }

    /**
     * @brief operator !=  重载运算符“==”
     * @param omd
     * @return 返回比较结果
     */
    bool operator!=(const OMD &omd) const
    {
        if(this->OI != omd.OI
                || this->method_index != omd.method_index
                || this->operate_mode != omd.operate_mode
                )
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param omd
     * @return
     */
    OMD& operator=(const OMD &omd)
    {
        this->OI = omd.OI;
        this->method_index = omd.method_index;
        this->operate_mode = omd.operate_mode;
        return *this;
    }

};

/**
 * @brief 日期时间数据类型date_time定义
 */
struct DateTime
{
    ushort year;//!< 年
    uchar month;//!< 月
    uchar day_of_month;//!< 日
    uchar day_of_week;//!< 星期：0表示周日，1…6分别表示周一到周六
    uchar hour;//!< 时
    uchar minute;//!< 分
    uchar second;//!< 秒
    ushort millseconds;//!< 毫秒

    QByteArray EncodeFrame(){
        QByteArray connect;
        connect.append(static_cast<char>(this->year>>8));
        connect.append(static_cast<char>(this->year));
        connect.append(static_cast<char>(this->month));
        connect.append(static_cast<char>(this->day_of_month));
        connect.append(static_cast<char>(this->day_of_week));
        connect.append(static_cast<char>(this->hour));
        connect.append(static_cast<char>(this->minute));
        connect.append(static_cast<char>(this->second));
        connect.append(static_cast<char>(this->millseconds>>8));
        connect.append(static_cast<char>(this->millseconds));
        return connect;
    }

    /**
     * @brief operator ==  重载运算符“==”
     * @param date_time
     * @return 返回比较结果
     */
    bool operator==(const DateTime &date_time) const
    {
        if(this->year == date_time.year
                && this->month == date_time.month
                && this->day_of_month == date_time.day_of_month
                && this->day_of_week == date_time.day_of_week
                && this->hour == date_time.hour
                && this->minute == date_time.minute
                && this->second == date_time.second
                && this->millseconds == date_time.millseconds
                )
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param date_time
     * @return
     */
    DateTime& operator=(const DateTime &date_time)
    {
        this->year = date_time.year;
        this->month = date_time.month;
        this->day_of_month = date_time.day_of_month;
        this->day_of_week = date_time.day_of_week;
        this->hour = date_time.hour;
        this->minute = date_time.minute;
        this->second = date_time.second;
        this->millseconds = date_time.millseconds;
        return *this;
    }
};

/**
 * @brief 日期时间数据类型date_time_s定义
 */
struct DateTimeS
{
    ushort year;//!< 年
    uchar month;//!< 月
    uchar day;//!< 日
    uchar hour;//!< 时
    uchar minute;//!< 分
    uchar second;//!< 秒


    QByteArray EncodeFrame(){
        QByteArray connect;
        connect.append(static_cast<char>(this->year>>8));
        connect.append(static_cast<char>(this->year));
        connect.append(static_cast<char>(this->month));
        connect.append(static_cast<char>(this->day));
        connect.append(static_cast<char>(this->hour));
        connect.append(static_cast<char>(this->minute));
        connect.append(static_cast<char>(this->second));
        return connect;
    }


    /**
     * @brief operator ==  重载运算符“==”
     * @param date_time_s
     * @return 返回比较结果
     */
    bool operator==(const DateTimeS &date_time_s) const
    {
        if(this->year == date_time_s.year
                && this->month == date_time_s.month
                && this->day == date_time_s.day
                && this->hour == date_time_s.hour
                && this->minute == date_time_s.minute
                && this->second == date_time_s.second
                )
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param date_time_s
     * @return
     */
    DateTimeS& operator=(const DateTimeS &date_time_s)
    {
        this->year = date_time_s.year;
        this->month = date_time_s.month;
        this->day = date_time_s.day;
        this->hour = date_time_s.hour;
        this->minute = date_time_s.minute;
        this->second = date_time_s.second;
        return *this;
    }
};

/**
 * @brief 日期数据类型date定义
 */
struct Date
{
    ushort year;//!< 年
    uchar month;//!< 月
    uchar day_of_month;//!< 日
    uchar day_of_week;//!< 星期：0表示周日，1…6分别表示周一到周六

    QByteArray EncodeFrame(){
        QByteArray connect;
        connect.append(static_cast<char>(this->year>>8));
        connect.append(static_cast<char>(this->year));
        connect.append(static_cast<char>(this->month));
        connect.append(static_cast<char>(this->day_of_month));
        connect.append(static_cast<char>(this->day_of_week));
        return connect;
    }

    /**
     * @brief operator ==  重载运算符“==”
     * @param date
     * @return 返回比较结果
     */
    bool operator==(const Date &date) const
    {
        if(this->year == date.year
                && this->month == date.month
                && this->day_of_month == date.day_of_month
                && this->day_of_week == date.day_of_week
                )
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param date
     * @return
     */
    Date& operator=(const Date &date)
    {
        this->year = date.year;
        this->month = date.month;
        this->day_of_month = date.day_of_month;
        this->day_of_week = date.day_of_week;
        return *this;
    }
};

/**
 * @brief 时间数据类型time定义
 */
struct Time
{
    uchar hour;//!< 时
    uchar minute;//!< 分
    uchar second;//!< 秒

    QByteArray EncodeFrame(){
        QByteArray connect;
        connect.append(static_cast<char>(this->hour));
        connect.append(static_cast<char>(this->minute));
        connect.append(static_cast<char>(this->second));
        return connect;
    }

    /**
     * @brief operator ==  重载运算符“==”
     * @param time
     * @return 返回比较结果
     */
    bool operator==(const Time &time) const
    {
        if(this->hour == time.hour
                && this->minute == time.minute
                && this->second == time.second
                )
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param time
     * @return
     */
    Time& operator=(const Time &time)
    {
        this->hour = time.hour;
        this->minute = time.minute;
        this->second = time.second;
        return *this;
    }
};

/**
 * @brief 时间间隔TI数据类型定义
 */
struct TI
{
    //uchar time_unit; //!< 时间单位：0-秒，1-分，2-时，3-日，4-月，5-年
    TIChoice  time_unit;
    ushort time_interval; //!< 时间间隔的间隔值,0表示无间隔

    QByteArray EncodeFrame(){
        QByteArray connect;
        connect.append(static_cast<char>(this->time_unit));
        connect.append(static_cast<char>(this->time_interval>>8));
        connect.append(static_cast<char>(this->time_interval));
        return connect;
    }

    /**
     * @brief operator ==  重载运算符“==”
     * @param ti
     * @return 返回比较结果
     */
    bool operator==(const TI &ti) const
    {
        if(this->time_unit == ti.time_unit
                && this->time_interval == ti.time_interval
                )
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param ti
     * @return
     */
    TI& operator=(const TI &ti)
    {
        this->time_unit = ti.time_unit;
        this->time_interval = ti.time_interval;
        return *this;
    }

};



/**
 * @brief 数据安全MAC的数据类型定义
 */
struct MAC
{
    uchar size;//!< MAC字节个数
    QByteArray mac_content;//!< 数据安全MAC内容

    /**
     * @brief operator ==  重载运算符“==”
     * @param mac
     * @return 返回比较结果
     */
    bool operator==(const MAC &mac) const
    {
        if(this->size == mac.size
                && this->mac_content == mac.mac_content)
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param mac
     * @return
     */
    MAC& operator=(const MAC &mac)
    {
        this->size = mac.size;
        this->mac_content = mac.mac_content;
        return *this;
    }
};


/**
 * @brief 随机数RN的数据类型定义
 */
struct RN
{
    uchar size;//!< RN字节个数
    QByteArray rn_content;//!< 随机数RN内容

    /**
     * @brief operator ==  重载运算符“==”
     * @param rn
     * @return 返回比较结果
     */
    bool operator==(const RN &rn) const
    {
        if(this->size == rn.size
                && this->rn_content == rn.rn_content)
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param rn
     * @return
     */
    RN& operator=(const RN &rn)
    {
        this->size = rn.size;
        this->rn_content = rn.rn_content;
        return *this;
    }
};


/**
 * @brief 安全标识SID的数据类型定义
 */
struct SID
{
    uint identification;//!< 标识
    uchar size;//!<  附加数据字节个数
    QByteArray additional_data;//!<  附加数据内容

    /**
     * @brief operator ==  重载运算符“==”
     * @param sid
     * @return 返回比较结果
     */
    bool operator==(const SID &sid) const
    {
        if(this->identification == sid.identification
                && this->size == sid.size
                && this->additional_data == sid.additional_data)
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param sid
     * @return
     */
    SID& operator=(const SID &sid)
    {
        this->size = sid.size;
        this->identification = sid.identification;
        this->additional_data = sid.additional_data;
        return *this;
    }
};

/**
 * @brief 安全标识SID的数据类型定义
 */
struct SID_MAC
{
    SID sid;
    MAC mac;

    /**
     * @brief operator ==  重载运算符“==”
     * @param sid_mac
     * @return 返回比较结果
     */
    bool operator==(const SID_MAC &sid_mac) const
    {
        if(this->sid == sid_mac.sid
                && this->mac == sid_mac.mac)
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param sid_mac
     * @return
     */
    SID_MAC& operator=(const SID_MAC &sid_mac)
    {
        this->sid = sid_mac.sid;
        this->mac = sid_mac.mac;
        return *this;
    }
};


/**
 * @brief 时间标签的数据类型TimeTag的定义
 */
struct TimeTag
{
    DateTimeS send_time_scale;//!< 发送时标
    TI allow_trans_delay_time;//!<  允许传输延时时间

    QByteArray EncodeFrame(){
        QByteArray time_tag;
        time_tag.append(this->send_time_scale.EncodeFrame());
        time_tag.append(this->allow_trans_delay_time.EncodeFrame());
        return time_tag;
    }

    /**
     * @brief operator ==  重载运算符“==”
     * @param time_tag
     * @return 返回比较结果
     */
    bool operator==(const TimeTag &time_tag) const
    {
        if(this->send_time_scale == time_tag.send_time_scale
                && this->allow_trans_delay_time == time_tag.allow_trans_delay_time)
            return true;
        else
            return false;
    }
};
/**
 * @brief The ComDcb struct串口控制块COMDCB的数据类型定义
 */
struct ComDcb
{
   BandRate band_rate_;//!<波特率
   CrcBit crc_bit_;//!<校验位
   DataBit data_bit_;//!<数据位
   StopBit stop_bit_;//!<停止位
   FlowControl flow_control_;//!<流控

   QByteArray EncodeFrame()
   {
       QByteArray comdcb;
       comdcb.append(static_cast<char>(this->band_rate_));
       comdcb.append(static_cast<char>(this->crc_bit_));
       comdcb.append(static_cast<char>(this->data_bit_));
       comdcb.append(static_cast<char>(this->stop_bit_));
       comdcb.append(static_cast<char>(this->flow_control_));
       return comdcb;
   }

   bool operator ==(const ComDcb &com_dcb) const
   {
       if(this->band_rate_==com_dcb.band_rate_
               &&this->crc_bit_==com_dcb.crc_bit_
               &&this->data_bit_==com_dcb.data_bit_
               &&this->stop_bit_==com_dcb.stop_bit_
               &&this->flow_control_==com_dcb.flow_control_)
           return true;
       else
           return false;
   }

};


}
#endif // DATATYPEBASEDEF_H
