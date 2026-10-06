#include <Glacier/Render/Material/ZRenderMaterialInstanceD3D.h>
#include <Glacier/Render/Material/ZRenderMaterialSubClass.h>
#include <Glacier/Render/Object/ZRenderObject.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderMaterialInstanceD3D::ZRenderMaterialInstanceD3D(const char* pszName, ZRenderMaterialSubClass* pSubClass, SRMaterialProperties* pMatProperties, uint32_t lMaterialId)
        : ZRenderMaterialInstance(pszName, pSubClass, pMatProperties, lMaterialId)
    {
    }

    ZRenderMaterialInstanceD3D::~ZRenderMaterialInstanceD3D() = default;

    void ZRenderMaterialInstanceD3D::Begin(ZRenderContext* pRenderContext)
    {
        ZASSERT(m_pMaterialSubClass);
        m_pMaterialSubClass->BeginInstance(this, pRenderContext);
    }

    void ZRenderMaterialInstanceD3D::Draw(ZRenderObjectInstance** ppInstances, uint32_t lCount, ZRenderContext* pRenderContext)
    {
        ZASSERT(m_pMaterialSubClass);
        m_pMaterialSubClass->Draw(this, ppInstances, lCount, pRenderContext);
    }

    void ZRenderMaterialInstanceD3D::End()
    {
        ZASSERT(m_pMaterialSubClass);
        m_pMaterialSubClass->EndInstance();
    }

    ZRenderObject* ZRenderMaterialInstanceD3D::CreateRenderObject(const ZPrimHandle& hPrim)
    {
        ZASSERT(m_pMaterialSubClass);
        return m_pMaterialSubClass->CreateRenderObject(hPrim, this);
    }
    
    void ZRenderMaterialInstanceD3D::FreeRenderObject(ZRenderObject* pObject)
    {
        ZASSERT(m_pMaterialSubClass);
        
        if (pObject)
        {
            ZUniMemory::Delete(pObject);
        }
    }

    bool ZRenderMaterialInstanceD3D::Update(ZRMaterialObject* pObject)
    {
        return false;
    }
}