#ifndef DATATYPEDEF_QCSG_H
#define DATATYPEDEF_QCSG_H

#include <QObject>

namespace  qcsg_local_module_protocol {

static const char kDirDown= 0;  //!< 传输方向位：由集中器、采集器发出的下行报文
static const char kDirUp  = 1;  //!< 传输方向位：通信模块发出的上行报文

static const char kNoAddr  = 0; //!< 地址域标识位：不带地址域
static const char kHasAddr = 1; //!< 地址域标识位：带地址域

static const uchar kFromPassiveSouth    = 0;//!< 启动标志位:从动站
static const uchar kFromActiveSouth     = 1;//!< 启动标志位:启动站

static const uchar kProtocolVer     = 0;//!< 协议版本号 本协议版本号为0
static const uchar kReserve         = 0;//!< 保留

/**
 * @brief 南网本地通信模块接口协议类型，以地区名为区分
 */
enum class Qcsg_Protocol_Type
{
    kStandard = 0,       //!< 0:标准南网协议
    kGuangXi,        //!< 1:广西扩展协议
    kGuangDong,      //!< 2:广东扩展协议
    kTongRen,      //!< 3:贵州铜仁扩展协议
    //GuangDongPlat   //!<
};


/**
 * @brief 控制域
 */
struct CtrlField
{
    uchar dir:1;     //!< 传输方向位 0-下行 1-上行
    uchar prm:1;     //!< 启动标志位 0-从动站 1-启动站
    uchar add:1;     //!< 地址域标识 0-不带地址域 1-带地址域
    uchar ver:2;     //!< 协议版本号 本协议版本号为0
    uchar reserve:3; //!< 保留

    /**
     * @brief operator ==  重载运算符“==”
     * @param ctrl_field
     * @return 返回比较结果
     */
    bool operator==(const CtrlField &ctrl_field) const
    {
        if(this->dir == ctrl_field.dir
                && this->prm == ctrl_field.prm
                && this->add == ctrl_field.add
                && this->ver == ctrl_field.ver
                && this->reserve == ctrl_field.reserve
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
        this->add = ctrl_field.add;
        this->ver = ctrl_field.ver;
        this->reserve = ctrl_field.reserve;

        return *this;
    }
};

/**
 * @brief 地址域
 */
struct AddressField
{
    uchar src_addr[6];   //!< 源地址
    uchar dst_addr[6];   //!< 目的地址

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
     * @return
     */
    AddressField& operator=(const AddressField &addr_field)
    {
        memcpy(this->src_addr,addr_field.src_addr,6);
        memcpy(this->dst_addr,addr_field.dst_addr,6);

        return *this;
    }
};

/**
 * @brief 从节点相位信息结构体
 */
struct NodePhaseInfo
{
    QByteArray node_addr;    //!< 从节点地址，小端模式
    uchar reserve:3;         //!< 节点相位信息:保留
    uchar phase_feature:2;   //!< 节点相位信息:相线特征
    uchar phase_ident:3;     //!< 节点相位信息:相线标识
    uchar protocol;          //!< 节点相位信息:规约类型

    /**
     * @brief operator ==  重载运算符“==”
     * @param node_phase_info
     * @return 返回比较结果
     */
    bool operator==(const NodePhaseInfo &node_phase_info) const
    {
        if(this->node_addr == node_phase_info.node_addr
                && this->reserve==node_phase_info.reserve
                && this->phase_feature==node_phase_info.phase_feature
                && this->phase_ident == node_phase_info.phase_ident
                && this->protocol == node_phase_info.protocol
                )
            return true;
        else
            return false;
    }
};

/**
 * @brief 广东修订：从节点相位信息结构体
 */
struct NodePhaseInfo_GD
{
    QByteArray node_addr;    //!< 从节点地址，小端模式
    uchar phase_type:3;      //!< 节点相位信息:相序类型
    uchar phase_feature:2;   //!< 节点相位信息:相线特征
    uchar phase_ident:3;     //!< 节点相位信息:相线标识
    uchar module_type:2;     //!< 节点相位信息:模块类型
    uchar reserve:1;        //!< 节点相位信息:保留
    uchar protocol:5;       //!< 节点相位信息:规约类型

    /**
     * @brief operator ==  重载运算符“==”
     * @param node_phase_info
     * @return 返回比较结果
     */
    bool operator==(const NodePhaseInfo_GD &node_phase_info) const
    {
        if(this->node_addr == node_phase_info.node_addr
                && this->phase_type==node_phase_info.phase_type
                && this->phase_feature==node_phase_info.phase_feature
                && this->phase_ident == node_phase_info.phase_ident
                && this->module_type==node_phase_info.module_type
                && this->reserve == node_phase_info.reserve
                && this->protocol == node_phase_info.protocol
                )
            return true;
        else
            return false;
    }
};

/**
 * @brief 节点台区识别结果结构体
 */
struct PlatIdentResult
{
    QByteArray node_addr;       //!< 从节点地址，小端模式
    uchar result = 0;           //!< 节点台区识别结果

    /**
     * @brief operator ==  重载运算符“==”
     * @param plat_ident_result
     * @return 返回比较结果
     */
    bool operator==(const PlatIdentResult &plat_ident_result) const
    {
        if(this->node_addr == plat_ident_result.node_addr
                && this->result == plat_ident_result.result
                )
            return true;
        else
            return false;
    }
};

/**
 * @brief 节点及节点属性结构体
 */
struct PlatIdentAttr
{
    QByteArray node_addr;  //!< 从节点地址，小端模式
    QByteArray cco_addr;   //!< 节点属性:上一所属台区主节点地址
    ushort reserve = 0;        //!< 节点属性:保留

    /**
     * @brief operator ==  重载运算符“==”
     * @param plat_ident_attr
     * @return 返回比较结果
     */
    bool operator==(const PlatIdentAttr &plat_ident_attr) const
    {
        if(this->node_addr == plat_ident_attr.node_addr
                && this->cco_addr == plat_ident_attr.cco_addr
                && this->reserve == plat_ident_attr.reserve
                )
            return true;
        else
            return false;
    }
};

/**
 * @brief 节点信息结构体
 */
struct NodeInfo
{
    uchar node_role:4;    //!< 节点角色,本站点的网络角色,0x0：无效,0x1：末梢节点(STA),0x2：代理节点(PCO),0x3：保留,0x4：主节点(CCO)
    uchar node_level:4;   //!< 节点层级,本站点的网络层级,0级代表0层级,依次类推

    /**
     * @brief operator ==  重载运算符“==”
     * @param node_info
     * @return 返回比较结果
     */
    bool operator==(const NodeInfo &node_info) const
    {
        if(this->node_role == node_info.node_role
                && this->node_level == node_info.node_level
                )
            return true;
        else
            return false;
    }
};

/**
 * @brief 厂家自定义协议-节点网络拓扑信息结构体
 */
struct NodeTopologyInfo
{
    QByteArray node_addr;     //!< 节点地址，小端模式
    ushort node_tei;        //!< 节点标识：本站点的节点标识(TEI),最大不超过1024
    ushort proxy_node_tei;  //!< 代理节点标识：本站点的代理站点节点标识(TEI)
    NodeInfo node_info_st;  //!< 节点信息

    /**
     * @brief operator ==  重载运算符“==”
     * @param node_topology_info
     * @return 返回比较结果
     */
    bool operator==(const NodeTopologyInfo &node_topology_info) const
    {
        if(this->node_addr == node_topology_info.node_addr
                && this->node_tei == node_topology_info.node_tei
                && this->proxy_node_tei == node_topology_info.proxy_node_tei
                && this->node_info_st == node_topology_info.node_info_st
                )
            return true;
        else
            return false;
    }
};

/**
 * @brief 广东扩展协议-节点网络拓扑信息结构体
 */
struct NodeTopologyInfo_GD
{
    QByteArray node_addr;      //!< 节点地址，小端模式
    ushort node_tei;         //!< 节点标识：本站点的节点标识(TEI),最大不超过1024
    ushort proxy_node_tei;   //!< 代理节点标识：本站点的代理站点节点标识(TEI)
    uint join_net_time_len;   //!< 节点加入网络时间： 以主节点上电时为初始时间， 从节点加入网络的时间， 单位为秒
    ushort pco_change_times; //!< 代理变更次数： 节点代理变更次数
    ushort off_line_times;   //!< 离线次数： 节点离线次数
    NodeInfo node_info_st;   //!< 节点信息

    /**
     * @brief operator ==  重载运算符“==”
     * @param node_topology_info
     * @return 返回比较结果
     */
    bool operator==(const NodeTopologyInfo_GD &node_topology_info) const
    {
        if(this->node_addr == node_topology_info.node_addr
                && this->node_tei == node_topology_info.node_tei
                && this->proxy_node_tei == node_topology_info.proxy_node_tei
                && this->join_net_time_len == node_topology_info.join_net_time_len
                && this->pco_change_times == node_topology_info.pco_change_times
                && this->off_line_times == node_topology_info.off_line_times
                && this->node_info_st == node_topology_info.node_info_st
                )
            return true;
        else
            return false;
    }
};

/**
 * @brief 广东扩展协议-周边节点信道信息结构体
 */
struct NeighborNodeChannelInfo_GD
{
    QByteArray node_addr;      //!< 节点地址，小端模式
    ushort node_tei;           //!< 节点标识：本站点的节点标识(TEI),最大不超过1024
    ushort proxy_node_tei;     //!< 代理节点标识：本站点的代理站点节点标识(TEI)
    uchar node_level;          //!< 节点层级,本站点的网络层级,0级代表0层级,依次类推
    uchar up_success_rate;     //!< 上行通信成功率： 节点与周边节点之间的上行通信成功率
    uchar down_success_rate;   //!< 上行通信成功率： 节点与周边节点之间的上行通信成功率
    uchar up_down_success_rate;//!< 上下行通信成功率： 节点与周边节点之间的上下行通信成功率
    uchar snr;                 //!< 信噪比： 节点与周边节点之间的信噪比， 单位为 dB， 取值范围（ 符号数） ： -20~80
    uchar attenuation;         //!< 衰减： 节点与周边节点之间的衰减， 单位为 dB， 取值范围（ 无符号数） ： 0~150

    /**
     * @brief operator ==  重载运算符“==”
     * @param node_channel_info
     * @return 返回比较结果
     */
    bool operator==(const NeighborNodeChannelInfo_GD &node_channel_info) const
    {
        if(this->node_addr == node_channel_info.node_addr
                && this->node_tei == node_channel_info.node_tei
                && this->proxy_node_tei == node_channel_info.proxy_node_tei
                && this->node_level == node_channel_info.node_level
                && this->up_success_rate == node_channel_info.up_success_rate
                && this->down_success_rate == node_channel_info.down_success_rate
                && this->up_down_success_rate == node_channel_info.up_down_success_rate
                && this->snr == node_channel_info.snr
                && this->attenuation == node_channel_info.attenuation
                )
            return true;
        else
            return false;
    }
};

/**
 * @brief 广东扩展协议-应用层报文信息结构体
 */
struct AppMsgInfo_GD
{
    ushort msg_len;  //!< 报文长度
    QByteArray msg;  //!< 报文内容

    /**
     * @brief operator ==  重载运算符“==”
     * @param msg_info
     * @return 返回比较结果
     */
    bool operator==(const AppMsgInfo_GD &msg_info) const
    {
        if(this->msg_len == msg_info.msg_len
                && this->msg == msg_info.msg
                )
            return true;
        else
            return false;
    }
};

/**
 * @brief 广西扩展协议-节点版本信息结构体
 */
struct NodeVersionInfo_GX
{
    QByteArray node_addr;      //!< 节点地址，小端模式
    QString vendor_code;    //!< 厂商代码,例“TC"
    QString chip_code;      //!< 芯片代码,例“R5"
    QString version_time;   //!< 版本时间,年月日,例“200516"
    QString version;        //!< 版本,例“0306"

    /**
     * @brief operator ==  重载运算符“==”
     * @param vrsn_info
     * @return 返回比较结果
     */
    bool operator==(const NodeVersionInfo_GX &vrsn_info) const
    {
        if(this->node_addr == vrsn_info.node_addr
                && this->vendor_code == vrsn_info.vendor_code
                && this->chip_code == vrsn_info.chip_code
                && this->version_time == vrsn_info.version_time
                && this->version == vrsn_info.version
                )
            return true;
        else
            return false;
    }
};

/**
 * @brief 厂家自定义协议-从节点信息结构体
 */
struct SlaveNodeInfo_TC
{
  QByteArray node_addr;      //!< 节点地址，小端模式
  uchar day:5;//!< 日1-31
  uchar month:4;//!< 月1-12
  uchar year:7;//!< 年：0-127
  uchar bootloader_version;//!< 从节点bootloader版本
  uchar patch_version:6;//!< 补丁版本号
  uchar sub_version:6;//!< 次版本号
  uchar main_version:4;//!< 主版本号

  /**
   * @brief operator ==  重载运算符“==”
   * @param node_info
   * @return 返回比较结果
   */
  bool operator==(const SlaveNodeInfo_TC &node_info) const
  {
      if(this->node_addr == node_info.node_addr
              && this->day == node_info.day
              && this->month == node_info.month
              && this->bootloader_version == node_info.bootloader_version
              && this->patch_version == node_info.patch_version
              && this->sub_version == node_info.sub_version
              && this->main_version == node_info.main_version
              )
          return true;
      else
          return false;
  }
};

/**
 * @brief 厂家自定义协议-宽带载波芯片信息结构体
 */
struct ChipInfo_TC
{
    QByteArray node_addr;      //!< 节点地址，小端模式
    uchar dvc_type;            //!< 节点设备类型
    QByteArray chip_id;        //!< 节点芯片ID信息,固定24字节
    QString version;         //!< 节点芯片软件版本信息,例“0306"

    /**
     * @brief operator ==  重载运算符“==”
     * @param chip_info
     * @return 返回比较结果
     */
    bool operator==(const ChipInfo_TC &chip_info) const
    {
        if(this->node_addr == chip_info.node_addr
                && this->dvc_type == chip_info.dvc_type
                && this->chip_id == chip_info.chip_id
                && this->version == chip_info.version
                )
            return true;
        else
            return false;
    }
};

/**
 * @brief 厂家自定义协议-路由工作模式数据结构体
 */
struct WorkMode_TC
{
    uchar power_off_Rpt_flag:1;
    uchar power_on_Rpt_flag:1;
    uchar clock_sync_flag:1;
    uchar loop_impedance_flag:1;
    uchar meter_09_to_13_flag:1;
    uchar prevent_steal_elect_flag:1;
    uchar meter_box_cluster_flag:1;
    uchar reserve_flag:1;
    uchar reserve;


    /**
     * @brief operator ==  重载运算符“==”
     * @param work_mode
     * @return 返回比较结果
     */
    bool operator==(const WorkMode_TC& work_mode) const
    {
        if (this->power_off_Rpt_flag==work_mode.power_off_Rpt_flag
                &&this->power_on_Rpt_flag==work_mode.power_on_Rpt_flag
                &&this->clock_sync_flag==work_mode.clock_sync_flag
                &&this->loop_impedance_flag==work_mode.loop_impedance_flag
                &&this->meter_09_to_13_flag==work_mode.meter_09_to_13_flag
                &&this->prevent_steal_elect_flag==work_mode.prevent_steal_elect_flag
                &&this->meter_box_cluster_flag==work_mode.meter_box_cluster_flag
                &&this->reserve_flag==work_mode.reserve_flag
                &&this->reserve==work_mode.reserve
            )
        return true;
    else
        return false;
    }
};


/**
 * @brief 贵州铜仁扩展协议：从节点其他信息结构体
 */
struct NodeOtherInfo_TR
{
    QByteArray node_addr;    //!< 从节点地址，小端模式
    uchar reserve:1;        //!< 保留
    uchar dvc_type:2;      //!< 设备类型
    uchar phase_feature:2;   //!< 相线特征
    uchar phase_ident:3;     //!< 相线标识
    uchar signal_quality:4;   //!< 信号品质
    uchar relay_level:4;     //!< 中继级别
    uchar protocol:4;       //!< 规约类型
    uchar node_state:4;     //!< 从节点状态
    QByteArray cco_addr;    //!< 从节点当前所属台区主节点地址，小端模式

    /**
     * @brief operator ==  重载运算符“==”
     * @param node_other_info
     * @return 返回比较结果
     */
    bool operator==(const NodeOtherInfo_TR &node_other_info) const
    {
        if(this->reserve == node_other_info.reserve
                && this->dvc_type==node_other_info.dvc_type
                && this->phase_feature==node_other_info.phase_feature
                && this->phase_ident == node_other_info.phase_ident
                && this->signal_quality==node_other_info.signal_quality
                && this->relay_level == node_other_info.relay_level
                && this->protocol == node_other_info.protocol
                && this->node_state == node_other_info.node_state
                && this->cco_addr == node_other_info.cco_addr
                )
            return true;
        else
            return false;
    }
};

}

#endif // DATATYPEDEF_QCSG_H
