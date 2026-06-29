#ifndef DYNAMICCREATE_H
#define DYNAMICCREATE_H

#include "ReflectFactory.h"

// 动态对象创建器
template<typename T>
class DynamicCreate
{
public:
//	static DynamicObject * CreateObject()
//	{
//		return new T();
//	}

    struct Registor
    {
        Registor()
        {
            if (!ReflectFactory::registerClass<T>(T::staticMetaObject.className()))
            {
//                assert(false);
            }
        }

        inline void do_nothing()const { }
    };

    static Registor s_registor;

public:
    DynamicCreate()
    {
        s_registor.do_nothing();
    }

    virtual ~DynamicCreate()
    {
        s_registor.do_nothing();
    }
};

template <typename T>
typename DynamicCreate<T>::Registor DynamicCreate<T>::s_registor;

#endif // DYNAMICCREATE_H
