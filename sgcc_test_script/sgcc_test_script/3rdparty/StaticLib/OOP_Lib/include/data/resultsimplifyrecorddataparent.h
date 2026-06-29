#ifndef RESULTSIMPLIFYRECORDDATAPARENT_H
#define RESULTSIMPLIFYRECORDDATAPARENT_H

#include <QObject>
#include "datatypebasedef.h"
#include "enumerated.h"
#include "datatype.h"
#include "recordrow.h"
#include "asimplifyrecord.h"
#include "../OOP_Lib_global.h"
#include "../exceptions/decodeexception.h"

namespace  object_oriented_electic_data_exchange_protocol {
/**
 * @brief The ResultSimplifyRecordDataParent class一个精简的记录型对象属性及其结果的基类
 */
#ifdef UNIT_TEST
class ResultSimplifyRecordDataParent
#else
class OOP_LIB_EXPORT ResultSimplifyRecordDataParent
#endif
{
public:
    ResultSimplifyRecordDataParent(SimplifyRecordResponseChoice  choice_);
   virtual  ~ResultSimplifyRecordDataParent();

    SimplifyRecordResponseChoice choice_;
    uchar data_column_number_;
    virtual QByteArray EncodeFrame()=0;
    virtual void DecodeFrame(QByteArray *data)=0;

};

#ifdef UNIT_TEST
class ResultSimplifyRecordDAR:public ResultSimplifyRecordDataParent
 #else
class OOP_LIB_EXPORT ResultSimplifyRecordDAR:public ResultSimplifyRecordDataParent
#endif
{
    public:
    ResultSimplifyRecordDAR();
    ~ResultSimplifyRecordDAR() override;

     DAR dar_;
     QByteArray EncodeFrame() override;
     void DecodeFrame(QByteArray *data) override;
};


#ifdef UNIT_TEST
class ResultSimplifyRecordData:public ResultSimplifyRecordDataParent
 #else
class OOP_LIB_EXPORT ResultSimplifyRecordData:public ResultSimplifyRecordDataParent
#endif
{
    public:
    ResultSimplifyRecordData();
   ~  ResultSimplifyRecordData() override;

      AsimplifyRecord asimplifyrecord;

     QByteArray EncodeFrame() override;
     void DecodeFrame(QByteArray *data) override;
};

#ifdef UNIT_TEST
class ResultSimplifyRecordFactory
#else
class OOP_LIB_EXPORT ResultSimplifyRecordFactory
#endif
{
   public:
    ResultSimplifyRecordFactory(){}
    ~ResultSimplifyRecordFactory(){}

   std::shared_ptr<ResultSimplifyRecordDataParent>  CreatResultSimplifyRecord(uchar choice_);
};




}
#endif // RESULTSIMPLIFYRECORDDATAPARENT_H
