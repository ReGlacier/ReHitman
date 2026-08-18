#pragma once


namespace Glacier
{
    enum EHardTypedef_TEnumID {};
    enum EHardTypedef_TPropertyID {};
    enum EHardTypedef_TIntPropertyID {};
    enum EHardTypedef_TMaterialDescID {};
    enum EHardTypedef_TBoolPropertyID {};
    enum EHardTypedef_TAudioPropertyID {};
    enum EHardTypedef_TScenePropertyID {};
    enum EHardTypedef_TFloatPropertyID {};
    enum EHardTypedef_TStringPropertyID {};

    template <typename TValue, typename TAG>
    struct ZTypedef
    {
        TValue m_Value;
    };
    // ZTypedef
}