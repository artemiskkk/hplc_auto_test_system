#ifndef ENUMERATED_H
#define ENUMERATED_H

#include <QObject>
#include "../OOP_Lib_global.h"

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 数据类型定义
 */
enum DataType
{
    kNULL=0,//!< 空
    kArray,//!< SEQUENCE OF Data，数组的元素在对象属性或方法的描述中定义
    kStructure,//!< SEQUENCE OF Data，结构的元素在对象属性或方法的描述中定义
    kBool,//!< 布尔值（BOOLEAN）：1或0
    kBit_string,//!< 比特位串（BIT STRING）
    kDouble_long,//!< 32比特位整数（Integer32）
    kDouble_long_unsigned,//!< 32比特位正整数（double-long-unsigned）
    kOctet_string=9,//!< 8比特位位组（字节）串（OCTET STRING）
    kVisible_string,//!< ASCII字符串（VisibleString）
    kUTF8_string=12,//!< UTF-8编码的字符串
    kInteger=15,//!< 8比特位整数（integer）
    kLong,//!< 16比特位整数（long）
    kUnsigned,//!< 8比特位正整数（Unsigned8）
    kLong_unsigned,//!< 16比特位正整数（Unsigned16）
    kLong64=20,//!< 64比特位整数（Integer64）
    kLong64_unsigned,//!< 64比特位正整数（Unsigned64）
    kEnum,//!< 枚举的元素在对象属性或方法的描述中定义
    kFloat32,//!< octet-string（SIZE（4））
    kFloat64,//!< octet-string（SIZE（8））
    kDate_time,//!< octet-string（SIZE（10））
    kDate,//!< octet-string（SIZE（5））
    kTime,//!< octet-string（SIZE（3））
    kDate_time_s,//!< octet-string（SIZE（7））
    kOI=80,//!< 对象标识数据类型
    kOAD,//!< 对象属性描述符数据类型
    kROAD,//!< 记录型对象属性描述符数据类型
    kOMD,//!< 对象方法描述符数据类型
    kTI,//!< 时间间隔数据类型
    kTSA,//!< 目标服务器地址
    kMAC,//!< 数据安全
    kRN,//!< 随机数
    kRegion,//!< 区间类型
    kScaler_Unit,//!< 换算及单位
    kRSD,//!< 记录选择描述符数据类型
    kCSD,//!< 列选择描述符数据类型
    kMS,//!< 电能表集合数据类型
    kSID,//!< 安全标识数据类型
    kSID_MAC,//!< SID_MAC的数据类型
    kCOMDCB,//!< 串口控制块数据类型
    kRCSD,//!< 记录列选择描述符数据类型

    kDataUndefined //!< 数据类型未定义
};


/**
 * @brief 数据访问结果DAR（Data Access Result）的数据类型定义
 */
enum DAR
{
    kSuccess,//!< 成功
    kHardwareFailure,//!< 硬件失效
    kTemporaryFailure,//!< 暂时失效
    kRejectReadWrite,//!< 拒绝读写
    kObjctUndefine,//!< 对象未定义
    kIC_Inconformity,//!< 对象接口类不符合
    kObjctNotExist,//!< 对象不存在
    kTypeMismatch,//!< 类型不匹配
    kSlopOver,//!< 越界
    kDataBlockNotAvailable,//!< 数据块不可用
    kSubFrameCancel,//!< 分帧传输已取消
    kNotInSubFrameState,//!< 不处于分帧传输状态
    kBlockWriteCancel,//!< 块写取消
    kBlockWriteNotExist,//!< 不存在块写状态
    kBlockSeqInvalid,//!< 数据块序号无效
    kPasswordErrorOrUnauthorized,//!< 密码错/未授权
    kCommRateCannotChange,//!< 通信速率不能更改
    kTimeZoneExceed,//!< 年时区数超
    kTimeIntervalExceed,//!< 日时段数超
    kRatesExceed,//!< 费率数超
    kSecurityAuthNotMatch,//!< 安全认证不匹配
    kReCharge,//!< 重复充值
    kESAMErr,//!< ESAM验证失败
    kAuthenErr,//!< 安全认证失败
    kCustomerIdMismach,//!< 客户编号不匹配
    kChargeTimesErr,//!< 充值次数错误
    kOverStocking,//!< 购电超囤积
    kAddrAbnormal,//!< 地址异常
    kSymmetricDecErr,//!< 对称解密错误
    kAsymmetricDecErr,//!< 非对称解密错误
    kSignErr,//!< 签名错误
    kMeterHangs,//!< 电能表挂起
    kTimeTagsInvalid,//!< 时间标签无效
    kRqstTimeOut,//!< 请求超时
    kP1p2Err,//!< ESAM的P1P2不正确
    kLCErr,//!< ESAM的LC错误
    kElse=255//!< 其它
};

/**
 * @brief 记录选择描述符RSD的选择方法枚举
 */
enum RsdChoice
{
    kNoSelector = 0,//!< 不选择
    kSelector1,//!< 选择方法1
    kSelector2,//!< 选择方法2
    kSelector3,//!< 选择方法3
    kSelector4,//!< 选择方法4
    kSelector5,//!< 选择方法5
    kSelector6,//!< 选择方法6
    kSelector7,//!< 选择方法7
    kSelector8,//!< 选择方法8
    kSelector9,//!< 选择方法9
    kSelector10,//!< 选择方法10
};

/**
 * @brief 列选择描述符CSD数据类型枚举
 */
enum CsdChoice
{
    kCsdOad=0,//!< 对象属性描述符 OAD
    kCsdRoad//!< 记录型对象属性描述符 ROAD
};

/**
 * @brief 电能表集合MS的种类枚举
 */
enum MSChoice
{
    kNoMeter = 0,//!< 无电能表
    kAllUserAddr,//!< 全部用户地址
    kGroupUserType,//!< 一组用户类型
    kGroupUserAddr, //!< 一组用户地址
    kGroupConfigSeq, //!< 一组配置序号
    kGroupUserTypeRegion,//!< 一组用户类型
    kGroupUserAddrRegion, //!< 一组用户地址
    kGroupConfigSeqRegion //!< 一组配置序号
};


/**
 * @brief 记录选择描述符RSD的选择方法枚举
 */
enum RegionChoice
{
    kBeforeCloseAfterOpen = 0,//!< 前闭后开
    kBeforeOpenAfterClose,//!< 前开后闭
    kBeforeCloseAfterClose,//!< 前闭后闭
    kBeforeOpenAfterOpen //!< 前开后开
};

/**
 * @brief 时间间隔TI时间单位枚举
 */
enum TIChoice
{
    kTiSecond = 0,  //!< 秒
    kTiMinute,      //!< 分
    kTiHour,        //!< 时
    kTiDay,        //!< 日
    kTiMonth,      //!< 月
    kTiYear,        //!< 年
};


/**
 * @brief The BandRate enum 串口波特率
 */
enum BandRate
{
    kBandRate300,//!<300bps
    kBandRate600,
    kBandRate1200,
    kBandRate2400,
    kBandRate4800,
    kBandRate7200,
    kBandRate9600,
    kBandRate19200,
    kBandRate38400,
    kBandRate57600,
    kBandRate115200,
    kBandRateAdapter,//!<自适应
};
/**
 * @brief The CrcBit enum 串口校验位
 */
enum CrcBit
{
    kNoCrc,//!<无校验
    kOdd,//!<奇校验
    kEven,//!<偶校验
};
/**
 * @brief The DataBit enum 串口数据位
 */
enum DataBit
{
    k5Bit=5,//!<5
    k6Bit,//!<6
    k7Bit,//!<7
    k8Bit,//!<8
};

/**
 * @brief The StopBit enum 串口停止位
 */
enum StopBit
{
    k1StopBit=1,//!<1
    k2StopBit,//!<2
};

/**
 * @brief The FlowControl enum 串口流控
 */
enum FlowControl
{
    kNoFlow=0,//!<无
    kHardFlow,//!<硬件
    kSoftFlow,//!<软件
};

/**
 * @brief 跟随上报信息枚举
 */
enum class FollowReportChoice
{
    kResultNormal = 1,  //!< 若干个对象属性及其数据
    kResultRecord,      //!< 若干个记录型对象属性及其数据
};

/**
 * @brief 一个对象属性的响应的数据的结果枚举
 */
enum class GetResultChoice
{
    kDar = 0,  //!< 错误信息
    kData,      //!< 数据
};

/**
 * @brief 一个记录型对象属性的响应数据的结果枚举
 */
enum class RecordResponseChoice
{
    kDar = 0,  //!< 错误信息
    kRecord,      //!< M条记录
};


/**
 *@brief 一个精简的记录型对象对象属性及其结果枚举
 */
enum class SimplifyRecordResponseChoice
{
    kDar = 0, //!<错误信息
    kSimplifyReored, //!<记录
    kSimplifyReored_lastrecord = 200, //!<最末记录
};


/**
 * @brief 分帧响应数据的结果枚举
 */
enum class SubFramingResponseChoice
{
    kDar = 0,  //!< 错误信息
    kResultNormal,      //!< 对象属性
    kResultRecord,      //!< 对象属性
};


/**
 * @brief 分读取对象属性MD5值结果枚举
 */
enum class MD5ResultChoice
{
    kDar = 0,  //!< 错误信息
    kMD5,      //!< MD5值
};

enum class TransResultChoice
{
    kDar,//!<错误信息
    kOctet_string,//!<<返回数据
};












/**
 * @brief 应用连接请求认证的结果ConnectResult的数据类型定义
 */
enum ConnectResult
{
    kAllowConnect = 0,//!< 允许建立应用连接
    kPasswordError,//!< 密码错误
    kSymmetricDecryptionError,//!< 对称解密错误
    kAsymmetricDecryptionError,//!< 非对称解密错误
    kSignatureError,//!< 签名错误
    kProtocolVersionMismatching,//!< 协议版本不匹配
    kElseError = 255  //!< 其他错误
};

/**
 * @brief 应用连接请求认证的机制信息ConnectMechanismInfo的数据类型定义
 */
enum ConnectMechanismInfoChoice
{
    kNullSecurity = 0,//!< 公共连接
    kPasswordSecurity,//!< 一般密码
    kSymmetrySecurity,//!< 对称加密
    kSignatureSecurity,//!< 数字签名
};





}
#endif // ENUMERATED_H
