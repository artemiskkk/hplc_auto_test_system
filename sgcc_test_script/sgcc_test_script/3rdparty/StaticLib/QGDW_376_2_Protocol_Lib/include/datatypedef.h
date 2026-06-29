#ifndef DATATYPEDEF3762_H
#define DATATYPEDEF3762_H

#include <QList>
#include <memory>

using namespace std;
namespace  qgdw_3762_protocol {

static const char kStartFlag = 0x68;  //!< 帧起始符：68H
static const char kEndFlag = 0x16;  //!< 帧结束符：16H

static const char kDirDown= 0;  //!< 传输方向位：由集中器、采集器发出的下行报文
static const char kDirUp  = 1;  //!< 传输方向位：通信模块发出的上行报文
static const char kNoAddr  = 0; //!< 地址域标识位：不带地址域
static const char kHasAddr = 1; //!< 地址域标识位：带地址域

static const char kPassive= 0;  //!< 启动方向位：从动站
static const char kActive  = 1;  //!< 启动方向位：启动站

const uchar kCenterRoute      = 1;
const uchar kDistributeRoute  = 2;
const uchar kHplc             = 3;
const uchar kWireless         = 10;
const uchar kEthernet         = 20;

/**
 * @brief 控制域
 */
struct CtrlField
{
    uchar comn_type:6;   //!< 通信方式
    uchar prm:1;         //!< 0-从动站 1-起动站
    uchar dir:1;         //!< 传输方向位 0-下行 1-上行
    /**
     * @brief operator ==  重载运算符“==”
     * @param ctrl_field
     * @return 返回比较结果
     */
    bool operator==(const CtrlField &ctrl_field) const
    {
        if(this->dir == ctrl_field.dir
                && this->comn_type == ctrl_field.comn_type
                && this->prm == ctrl_field.prm
                )
            return true;
        else
            return false;
    }
    /**
     * @brief operator =  重载运算符“=”
     * @param ctrl_field
     * @return  结构体赋值
     */
    CtrlField& operator=(const CtrlField &ctrl_field)
    {
        this->dir = ctrl_field.dir;
        this->prm = ctrl_field.prm;
        this->comn_type = ctrl_field.comn_type;

        return *this;
    }
};

/**
 * @brief 下行信息域子结构
 */
struct InfoFieldDown
{
    uchar route_id:1;           //!< 路由标识
    uchar affiled_node_id:1;    //!< 附属节点标识
    uchar comu_module_ident:1;  //!< 通信模块标识
    uchar cflt_detect:1;        //!< 冲突检测
    uchar relay_level:4;        //!< 中继级别

    uchar channel_id:4;         //!< 信道标识
    uchar err_crct_code_id:4;   //!< 纠错编码标识

    uchar expect_reply_bytes;   //!< 预计应答字节数

    uchar comu_rate;       //!< 通信速率
    uchar rate_unit_flag;   //!< 速率单位标识

    char msg_seq;              //!< 报文序列号

    /**
     * @brief operator = 重载运算符“=”
     * @param info_field
     * @return
     */
    InfoFieldDown& operator=(const InfoFieldDown &info_field)
    {
        this->route_id = info_field.route_id;
        this->affiled_node_id = info_field.affiled_node_id;
        this->comu_module_ident = info_field.comu_module_ident;
        this->cflt_detect = info_field.cflt_detect;
        this->relay_level = info_field.relay_level;

        this->channel_id = info_field.channel_id;
        this->err_crct_code_id = info_field.err_crct_code_id;

        this->expect_reply_bytes = info_field.expect_reply_bytes;

        this->comu_rate = info_field.comu_rate;
        this->rate_unit_flag = info_field.rate_unit_flag;

        this->msg_seq = info_field.msg_seq;

        return *this;
    }
    /**
     * @brief operator == 重载运算符“==”
     * @param info_field
     * @return
     */
    bool operator==(const InfoFieldDown &info_field) const
    {
        if(this->route_id == info_field.route_id
                &&this->affiled_node_id == info_field.affiled_node_id
                &&this->comu_module_ident == info_field.comu_module_ident
                &&this->cflt_detect == info_field.cflt_detect
                &&this->relay_level == info_field.relay_level

                &&this->channel_id == info_field.channel_id
                &&this->err_crct_code_id == info_field.err_crct_code_id

                &&this->expect_reply_bytes == info_field.expect_reply_bytes

                &&this->comu_rate == info_field.comu_rate
                &&this->rate_unit_flag == info_field.rate_unit_flag

                &&this->msg_seq == info_field.msg_seq)

        return true;
        else {
            return false;
        }
    }
};

/**
 * @brief 上行信息域子结构
 */
struct InfoFieldUp
{
    uchar route_id:1;               //!< 路由标识
    uchar reserve_2:1;              //!< 保留
    uchar comu_module_ident:1;      //!< 通信模块标识
    uchar reserve_1:1;              //!< 保留
    uchar relay_level:4;            //!< 中继级别

    uchar channel_id:4;             //!< 信道标识
    uchar reserve_3:4;              //!< 保留


    uchar measured_phase_flag:4;    //!< 实测相线标识
    uchar meter_channel_character:4;//!< 电能表通道特征

    uchar end_cmd_signal_quality:4; //!< 末级命令信号品质
    uchar end_resp_signal_quality:4;//!< 末级应答信号品质

    uchar event_flag:1;     //!< 事件标志
    uchar line_flag:1;      //!< 线路标志
    uchar zone_flag:1;      //!< 台区标志
    uchar reserve_4:5;      //!< 保留

    char msg_seq;          //!< 报文序列号

    /**
     * @brief operator = 重载运算符“=”
     * @param info_field
     * @return
     */
    InfoFieldUp& operator=(const InfoFieldUp &info_field)
    {
        this->route_id = info_field.route_id;
        this->reserve_2 = info_field.reserve_2;
        this->comu_module_ident = info_field.comu_module_ident;
        this->reserve_1 = info_field.reserve_1;
        this->relay_level = info_field.relay_level;

        this->channel_id = info_field.channel_id;
        this->reserve_3 = info_field.reserve_3;

        this->measured_phase_flag = info_field.measured_phase_flag;
        this->meter_channel_character = info_field.meter_channel_character;

        this->end_cmd_signal_quality = info_field.end_cmd_signal_quality;
        this->end_resp_signal_quality = info_field.end_resp_signal_quality;

        this->event_flag = info_field.event_flag;
        this->line_flag = info_field.line_flag;
        this->zone_flag = info_field.zone_flag;
        this->reserve_4 = info_field.reserve_4;

        this->msg_seq = info_field.msg_seq;

        return *this;
    }
    /**
     * @brief operator == 重载运算符“==”
     * @param info_field
     * @return
     */
    bool operator==(const InfoFieldUp &info_field) const
    {
        if(this->route_id == info_field.route_id
                &&this->reserve_2 == info_field.reserve_2
                &&this->comu_module_ident == info_field.comu_module_ident
                &&this->reserve_1 == info_field.reserve_1
                &&this->relay_level == info_field.relay_level

                &&this->channel_id == info_field.channel_id
                &&this->reserve_3 == info_field.reserve_3

                &&this->measured_phase_flag == info_field.measured_phase_flag
                &&this->meter_channel_character == info_field.meter_channel_character

                &&this->end_cmd_signal_quality == info_field.end_cmd_signal_quality
                &&this->end_resp_signal_quality == info_field.end_resp_signal_quality

                &&this->event_flag == info_field.event_flag
                &&this->line_flag == info_field.line_flag
                &&this->zone_flag == info_field.zone_flag
                &&this->reserve_4 == info_field.reserve_4

                &&this->msg_seq == info_field.msg_seq)

        return true;
        else {
            return false;
        }
    }
};

/**
 * @brief 信息域(上行或者下行)
 */
union InfoField
{
    InfoFieldDown info_field_down;
    InfoFieldUp info_field_up;

    /**
     * @brief operator =  重载运算符“=”
     * @param info_field
     * @return
     */
    InfoField& operator=(const InfoField &info_field)
    {
        this->info_field_down = info_field.info_field_down;
        this->info_field_up = info_field.info_field_up;

        return *this;
    }
    /**
     * @brief operator ==  重载运算符“==”
     * @param info_field
     * @return
     */
    bool operator==(const InfoField &info_field) const
    {
        if(this->info_field_down == info_field.info_field_down
                &&this->info_field_up == info_field.info_field_up)
            return true;
        else
            return false;
    }
};

/**
 * @brief 中继地址结构
 */
struct Address
{
    char addr[6];
    Address(){}
    Address(uchar addr[6])
    {
        memcpy(this->addr,addr,6);
    }
    Address(QByteArray addr)
    {
        addr.size()>=6?memcpy(this->addr,addr,6):memcpy(this->addr,addr,addr.size());
    }
    QString toString()
    {
        return QString(QByteArray(this->addr,6).toHex());
    }
    /**
     * @brief operator 重载运算符“==”
     * @param address
     * @return
     */
    bool operator==(const Address &address) const
    {
        if(memcmp(this->addr,address.addr,6)==0)
            return true;
        else
            return false;
    }
    /**
     * @brief operator =  重载运算符“=”
     * @param Address
     * @return 结构体赋值
     */
    Address& operator=(const Address &addr_)
    {
        memcpy(this->addr,addr_.addr,6);
        return *this;
    }
};

/**
 * @brief 地址域
 */
struct AddressField
{
    char src_addr[6];           //! <源地址
    QList<shared_ptr<Address>> delay_addr;  //! <中继地址
    char dst_addr[6];           //! <目的地址

    /**
     * @brief operator ==  重载运算符“==”
     * @param addr_field
     * @return 返回比较结果
     */
    bool operator==(const AddressField &addr_field) const
    {
        if(memcmp(this->src_addr,addr_field.src_addr,6)
                && memcmp(this->dst_addr,addr_field.dst_addr,6)
                )
            return true;
        else
            return false;
    }

    /**
     * @brief operator =  重载运算符“=”
     * @param addr_field
     * @return 结构体赋值
     */
    AddressField& operator=(const AddressField &addr_field)
    {
        memcpy(this->src_addr,addr_field.src_addr,6);
        memcpy(this->dst_addr,addr_field.dst_addr,6);

        return *this;
    }
};

}

#endif // DATATYPEDEF3762_H
