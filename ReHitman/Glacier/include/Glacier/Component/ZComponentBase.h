#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/NotCopyable.h>


namespace Glacier
{
    struct ZComponentBase : public NotCopyable
    {
        // vtbl
        virtual ~ZComponentBase();
        virtual void TestSetUp();
        virtual void TestTearDown();
        virtual void InitializeComponent() = 0;

        // members
        ZComponentBase();

        // data
        struct ZComponentBase* m_pNext { nullptr };
        struct ZComponentBase* m_pPrev { nullptr };
        const char* m_ComponentId { nullptr };
    };
    RE_VERIFY_SIZE(ZComponentBase, 0x10);
}