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
        // methods
        ZTypedef() = default;
        ZTypedef(TValue val) : m_Value(val) {}

        // members
        TValue m_Value {};
    };

    // aliases
    using TMaterialDescID = ZTypedef<int,EHardTypedef_TMaterialDescID>;
    using TEnumID = ZTypedef<int, EHardTypedef_TEnumID>;
    using TBoolPropertyID = ZTypedef<int,EHardTypedef_TBoolPropertyID>;
    using TIntPropertyID = ZTypedef<int,EHardTypedef_TIntPropertyID>;
    using TFloatPropertyID = ZTypedef<int,EHardTypedef_TFloatPropertyID>;
    using TStringPropertyID = ZTypedef<int,EHardTypedef_TStringPropertyID>;
    using TScenePropertyID = ZTypedef<int,EHardTypedef_TScenePropertyID>;
    using TAudioPropertyID = ZTypedef<int,EHardTypedef_TAudioPropertyID>;
}
