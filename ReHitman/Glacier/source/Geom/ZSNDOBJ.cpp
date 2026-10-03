#include <Glacier/Geom/ZSNDOBJ.h>
#include <Glacier/Audio/ZSoundObject.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>


namespace Glacier
{
    STATIC_CLASS_VAR_IMPL(ZSNDOBJ, uint32_t, m_Id, 0, 0);
    STATIC_CLASS_VAR_IMPL(ZSNDOBJ, uint32_t, m_Mask, 0, 0);

    bool ZSNDOBJ::AttachToDefaultRoom(bool bOverride)
    {
        // TODO: Finish me
        return false;
    }

    ZSoundObject* ZSNDOBJ::GetSoundObject()
    {
        return g_pEngineData->SRefToPtr(m_rZSoundRef);
    }
}
