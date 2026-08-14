#include <Glacier/Render/Material/ZRenderMaterialBinderParser.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderList.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderMaterialBinderParser::ZRenderMaterialBinderParser()
        : m_pTranslatorMapper(g_TranslatorMapper)
    {
    }

    ZRenderMaterialBinderParser::~ZRenderMaterialBinderParser() = default;

    void ZRenderMaterialBinderParser::CreateBinders(ZRenderMaterialBinderList* pMaterialBinderList, ZRenderMaterialInstance* pMaterialInstance)
    {
        ZASSERT(pMaterialBinderList->m_BindersMaterial.m_pBinders == nullptr);
        ZASSERT(pMaterialBinderList->m_BindersObject.m_pBinders == nullptr);

        ZRenderBinder* aMaterialBindersStack[256];
        ZRenderBinder* aObjectBindersStack[256];

        // Attach to stack allocated stuff
        pMaterialBinderList->m_BindersMaterial.m_pBinders = aMaterialBindersStack;
        pMaterialBinderList->m_BindersObject.m_pBinders   = aObjectBindersStack;

        // Run parsers
        CreatePropertyBinders(pMaterialBinderList, pMaterialInstance);
        CreateBuiltInBinders(pMaterialBinderList, pMaterialInstance);

        // Alloc&copy materials
        const uint32_t lNumMaterialBinders = pMaterialBinderList->m_BindersMaterial.m_lNumBinders;
        if (lNumMaterialBinders > 0)
        {
            ZRenderBinder** ppBindersHeap = static_cast<ZRenderBinder**>(ZUniMemory::Allocate(sizeof(ZRenderBinder*) * lNumMaterialBinders));

            for (uint32_t i = 0; i < lNumMaterialBinders; ++i)
            {
                ZASSERT(i < pMaterialBinderList->m_BindersMaterial.m_lNumBinders);
                ppBindersHeap[i] = pMaterialBinderList->m_BindersMaterial.m_pBinders[i];
            }

            pMaterialBinderList->m_BindersMaterial.m_pBinders = ppBindersHeap;
        }
        else
        {
            pMaterialBinderList->m_BindersMaterial.m_pBinders = nullptr;
        }

        // Allocate&copy objects
        const uint32_t lNumObjectBinders = pMaterialBinderList->m_BindersObject.m_lNumBinders;
        if (lNumObjectBinders > 0)
        {
            ZRenderBinder** ppBindersHeap = static_cast<ZRenderBinder**>(ZUniMemory::Allocate(sizeof(ZRenderBinder*) * lNumObjectBinders));

            for (uint32_t j = 0; j < lNumObjectBinders; ++j)
            {
                ZASSERT(j < pMaterialBinderList->m_BindersObject.m_lNumBinders);
                ppBindersHeap[j] = pMaterialBinderList->m_BindersObject.m_pBinders[j];
            }

            pMaterialBinderList->m_BindersObject.m_pBinders = ppBindersHeap;
        }
        else
        {
            pMaterialBinderList->m_BindersObject.m_pBinders = nullptr;
        }
    }

    void ZRenderMaterialBinderParser::CreatePropertyBinders(ZRenderMaterialBinderList* pMaterialBinderList, ZRenderMaterialInstance* pMaterialInstance)
    {
        // TODO: Finish me
    }

    void ZRenderMaterialBinderParser::CreateBuiltInBinders(ZRenderMaterialBinderList* pMaterialBinderList, ZRenderMaterialInstance* pMaterialInstance)
    {
        if (!m_pTranslatorMapper)
            return;

        for (const SMapper* pMapper = m_pTranslatorMapper; pMapper->Type != RTT_NONE; ++pMapper)
        {
            if (pMapper->Type == RTT_MATERIAL)
            {
                CreatePropertyBinderContext(
                    &pMaterialBinderList->m_BindersMaterial,
                    pMapper->pszName, 
                    static_cast<uint32_t>(pMapper->Name)
                );
            }
            else if (pMapper->Type == RTT_OBJECT)
            {
                CreatePropertyBinderContext(
                    &pMaterialBinderList->m_BindersObject, 
                    pMapper->pszName, 
                    static_cast<uint32_t>(pMapper->Name)
                );
            }
        }
    }

    void ZRenderMaterialBinderParser::VerifyBinders(int lLayer, ZRenderMaterialBinderList* pMaterialBinderList, ZRenderMaterialInstance* pMaterialInstance)
    {
        // Do nothing or lost debug stuff
    }
}