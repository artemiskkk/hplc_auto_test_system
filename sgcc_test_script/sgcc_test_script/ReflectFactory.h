#ifndef REFLECTFACTORY_H
#define REFLECTFACTORY_H

#include <QObject>
#include <QMap>
#include <QMetaObject>
#include <QDebug>
#include "PublicDataStruct/abstractscript.h"
#include <memory>
#include <cstddef>

//class ReflectFactory
//{
//public:
//    template<typename T>
//    static int registerClass()
//    {
//        qInfo()<<"registerClass 注册";
//        constructors().insert( T::staticMetaObject.className(), &constructorHelper<T> );
//        qInfo()<<QString(T::staticMetaObject.className());
//        return 1;
//    }

//    static std::shared_ptr<AbstractScript> newInstance( const QByteArray& className)
//    {
//        Constructor constructor = constructors().value( className );
//        if ( constructor == nullptr )
//            {
//            qInfo()<<"createObject 空指针";
//            return nullptr;
//            }
//        return (*constructor)();
//    }

//private:
//    typedef std::shared_ptr<AbstractScript> (*Constructor)();

//    template<typename T>
//    static std::shared_ptr<AbstractScript> constructorHelper()
//    {
//        qInfo()<<"constructorHelper:"+QString(T::staticMetaObject.className())+" 创建";
//        return std::make_shared<T>();
//    }

//    static QMap<QByteArray, Constructor>& constructors()
//    {
//        static QMap<QByteArray, Constructor> instance;
//        return instance;
//    }
//};

class ReflectFactory
{
public:
    static ReflectFactory & Instance()
    {
        static ReflectFactory fac;
        return fac;
    }

    template<typename T>
    static bool registerClass(QByteArray classname)
    {
        if(classname.isEmpty())
        {
            return false;
        }
        //constructors().insert( T::staticMetaObject.className(), &constructorHelper<T> );
        constructors().insert( classname, &constructorHelper<T> );
//        qInfo()<<"registerClass:"+QString(T::staticMetaObject.className())+" 注册1";
//        qInfo()<<"registerClass:"+QString(typeid(T).name())+" 注册2";
        return true;
    }

    static std::shared_ptr<AbstractScript> newInstance( const QByteArray& className)
    {
        Constructor constructor = constructors().value( className );
        if ( constructor == nullptr )
            return nullptr;
        return (*constructor)();
    }

private:
    typedef std::shared_ptr<AbstractScript> (*Constructor)();

    template<typename T>
    static std::shared_ptr<AbstractScript> constructorHelper()
    {
//        qInfo()<<"constructorHelper:"+QString(T::staticMetaObject.className())+" 创建";
        return std::make_shared<T>();
    }

    static QMap<QByteArray, Constructor>& constructors()
    {
        static QMap<QByteArray, Constructor> instance;
        return instance;
    }
};

#endif // REFLECTFACTORY_H
