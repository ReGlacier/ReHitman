#pragma once

#include <Glacier/Runtime/ZGEOMCLASSINFO.h>
#include <Glacier/Runtime/ZROUTCLASSINFO.h>
#include <Glacier/Runtime/ZFactory.h>
#include <Glacier/Geom/ZGeomValidator.h>
#include <Glacier/ZUniMemory.h>


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

#define DECLARE_ID_AND_MASK(cls) \
    STATIC_CLASS_VAR(cls, uint32_t, m_Id); \
    STATIC_CLASS_VAR(cls, uint32_t, m_Mask);

#define DECLARE_ID_AND_MASK_IMPL(cls, id_addr, mask_addr) \
    STATIC_CLASS_VAR_IMPL(cls, uint32_t, m_Id, id_addr, 0u); \
    STATIC_CLASS_VAR_IMPL(cls, uint32_t, m_Mask, mask_addr, 0u);


#define DECLARE_GEOM_CLASS(ClassName, ClassTypeId)                \
    static constexpr uint32_t m_TypeId = ClassTypeId;             \
    STATIC_CLASS_VAR(ClassName, const char*, FactoryName);        \
    STATIC_CLASS_VAR(ClassName, RTP::ZPropertyInfo, Info);        \
    STATIC_CLASS_VAR(ClassName, ZGEOMCLASSINFO*, m_OldClassInfo); \
    DECLARE_ID_AND_MASK(ClassName);

#define DECLARE_GEOM_CLASS_IMPL(ClassName, BaseClass, OldClassInfoAddr, FactroryName, FactoryAddr, FirstProperty, PropertiesAddr, IDAddr, MaskAddr) \
    STATIC_CLASS_VAR_IMPL(ClassName, const char*, FactoryName, FactoryAddr, FactroryName);                                                          \
    STATIC_CLASS_VAR_IMPL(ClassName, RTP::ZPropertyInfo, Info, PropertiesAddr, (RTP::ZPropertyInfo {                                                \
        .First = FirstProperty,                                                                                                                     \
        .Super = &BaseClass::Info,                                                                                                                  \
        .Name = ClassName::FactoryName                                                                                                              \
    }));                                                                                                                                            \
    DECLARE_ID_AND_MASK_IMPL(ClassName, IDAddr, MaskAddr);                                                                                          \
    REGISTER_GLACIER_GEOM_CLASS(ClassName, BaseClass, ClassName::m_TypeId, OldClassInfoAddr);
