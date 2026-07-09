#pragma once

#include <Glacier/ReGlacier.h>


namespace Glacier
{
    struct ZComponentBase
    {
        // NotCopyable
        ZComponentBase() = default;
        ZComponentBase(const ZComponentBase&) = delete;
        ZComponentBase& operator=(const ZComponentBase&) = delete;

        // vtbl
        virtual ~ZComponentBase();
        virtual void TestSetUp();
        virtual void TestTearDown();
        virtual void InitializeComponent();

        // data
        struct ZComponentBase* m_pNext;
        struct ZComponentBase* m_pPrev;
        const char* m_ComponentId;
    };
    RE_VERIFY_SIZE(ZComponentBase, 0x10);
}