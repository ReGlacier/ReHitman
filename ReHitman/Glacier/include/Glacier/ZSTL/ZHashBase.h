#pragma once

#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
#pragma region "Base Class definition"
    class ZHashBase
    {
    public:
        //vftable (5 entries)
        virtual void SetSize(unsigned int size);
        virtual void Release();
        virtual void* GetArray();
        virtual size_t NodeSize();

        //data (size is 0x18)
        int m_field4;
        int m_field8;
        int m_fieldC;
        int m_field10;
        int m_field14;
    };
#pragma endregion

#pragma region "ZHash class definition"
    template <typename T1, typename T2> class ZHash;
//    007A08C8 6  ZHash<int,CMapIconDraw::SMatPos>         ZHash<int,CMapIconDraw::SMatPos>: ZHashBase;
//    00762990 6  ZHash<int,ZDecalMarkController::ZLink *> ZHash<int,ZDecalMarkController::ZLink *>: ZHashBase;
    template <> class ZHash<int, unsigned int> : ZHashBase
    {
    public:
        //vftable
        virtual int IntValue(const int&, unsigned int&, unsigned int&);
        virtual bool Equals(const int&, const int&);

        //data (total size is 0x20, base size is 0x18)
        int m_field18;
        int m_field1C;
    };

    template <> class ZHash<int, SMatPos> : ZHashBase
    {
    public:
        //vftable

        //api methods
        void Clear();
        void Put(SMatPos value, bool a2);
        [[nodiscard]] SMatPos* Find(const int& value) const;

        //data
        int m_field18;
        int m_field1C;
    };

#pragma endregion

#pragma region "ZIntHash class definition"
    template <typename T1> class ZIntHash;
//    007A0D08 6  ZIntHash<CMapIconDraw::SMatPos>         ZIntHash<CMapIconDraw::SMatPos>: ZHash<int,CMapIconDraw::SMatPos>, ZHashBase;
//    00762E20 6  ZIntHash<ZDecalMarkController::ZLink *> ZIntHash<ZDecalMarkController::ZLink *>: ZHash<int,ZDecalMarkController::ZLink *>, ZHashBase;
//    007AC994 6  ZIntHash<unsigned int>                  ZIntHash<unsigned int>: ZHash<int,unsigned int>, ZHashBase;
    template <> class ZIntHash<unsigned int> : ZHash<int, unsigned int>
    {
    public:
        //vftable (equals to parent)
        //data (same to ZHash<int, unsigned int>
    };

#pragma endregion
}