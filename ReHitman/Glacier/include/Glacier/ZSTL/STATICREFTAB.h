#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/REFTAB.h>


namespace Glacier
{
    class STATICREFTAB : public REFTAB
    {
    public:
        // methods
        STATICREFTAB(int pPoolSize, int pUserData);
        
        // vtbl
        ~STATICREFTAB() override;

        uint32_t* Add(uint32_t) override;
        uint32_t* AddUnique(uint32_t) override;
        void Clear() override;
        void ClearThis() override;
        void DelRefPtr(uint32_t*) override;
        uint32_t* Find(uint32_t) const override;
        void Remove(uint32_t) override;
        bool RemoveIfExists(uint32_t) override;
        void RunDelRef(RefRun *) override;
        void RunInitNxtRef(RefRun *) const override;
        void RunInitNxtRef(RefRun *) override;
        void RunInitPrevRef(RefRun *) const override;
        void RunInitPrevRef(RefRun *) override;
        uint32_t RunNxtRef(RefRun *) const override;
        uint32_t RunNxtRef(RefRun *) override;
        const uint32_t* RunNxtRefPtr(RefRun *) const override;
        uint32_t* RunNxtRefPtr(RefRun *) override;
        uint32_t RunPrevRef(RefRun *) const override;
        uint32_t RunPrevRef(RefRun *) override;
        const uint32_t* RunPrevRefPtr(RefRun *) const override;
        uint32_t* RunPrevRefPtr(RefRun *) override;

        // In 2002..2006 IOI fucked up that vtable and added 3 dummy 'overloads' but with broken vtbl
        virtual void Exists_LOST(int) { return; }
        virtual uint32_t GetRefNr_LOST(int) { return 0; }
        virtual uint32_t* GetRefPtrNr_LOST(int) { return nullptr; }

        // data
        REFTAB* m_pFreeStack{nullptr};
    };
    RE_VERIFY_SIZE(STATICREFTAB, 0x20); // Verified
}