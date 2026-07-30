#pragma once

#include <Glacier/Runtime/ZGEOMCLASSINFO.h>
#include <Glacier/Runtime/ZROUTCLASSINFO.h>
#include <Glacier/Runtime/ZFactory.h>
#include <Glacier/Geom/ZGeomValidator.h>


#define REGISTER_GLACIER_GEOM_CLASS(Class, BaseClass, ClassID, OldClassInfoAddr) \
    ZFactoryProducer<ZGEOM, Class> Class##_Producer( \
        ClassID, \
        ZGEOMCLASSINFO( \
            #Class, \
            sizeof(Class), \
            #BaseClass, \
            ClassID, \
            0, \
            nullptr, \
            ZGeomValidator<Class>::SetTypeIDAndMask, \
            &Class::m_Id, \
            &Class::m_Mask \
        ) \
    ); \
    STATIC_CLASS_VAR_IMPL(Class, ZGEOMCLASSINFO*, m_OldClassInfo, OldClassInfoAddr, &Class##_Producer.m_Data)

#define REGISTER_GLACIER_GEOM_BASE_CLASS(Class, ClassID, OldClassInfoAddr) \
    ZFactoryProducerPure<Class> Class##_Producer( \
        ClassID, \
        ZGEOMCLASSINFO( \
            #Class, \
            sizeof(Class), \
            nullptr, \
            ClassID, \
            0, \
            nullptr, \
            ZGeomValidator<Class>::SetTypeIDAndMask, \
            &Class::m_Id, \
            &Class::m_Mask \
        ) \
    ); \
    STATIC_CLASS_VAR_IMPL(Class, ZGEOMCLASSINFO*, m_OldClassInfo, OldClassInfoAddr, &Class##_Producer.m_Data)

#define REGISTER_ROUT_CLASS(Class, BaseClass, BaseClassUpperCaseName, EventName, Flags, RoutType) \
    ZROUTCLASSINFO BaseClass##_##Class##_ClassInfo( \
        #BaseClassUpperCaseName "_" #Class, \
        sizeof(Class), \
        nullptr, \
        EventName, \
        #BaseClass, \
        RoutType, \
        Flags, \
        nullptr \
    ); \
    ZFactoryProducer<Class> Class##_Producer( \
        #BaseClass "_" #Class, \
        &BaseClass##_##Class##_ClassInfo \
    ); \
    ZFactoryProducer<Class> Class##_Producer2( \
        EventName, \
        &BaseClass##_##Class##_ClassInfo \
    ) \
    STATIC_CLASS_VAR_IMPL(Class, ZGEOMCLASSINFO*, m_OldClassInfo, OldClassInfoAddr, &Class##_Producer)

