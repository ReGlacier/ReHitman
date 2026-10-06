#include <Glacier/Items/ZItemWatch.h>

#include <Glacier/ScriptEngine/ScriptEngine.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    namespace
    {
        constexpr ZMSGID kScriptSendItemWatch = 0x823;
        constexpr ZMSGID kScriptSendItemWatchPutdown = 2083;

        void SendItemWatch(ZItemWatch* watch, ZMSGID message, uint32_t state)
        {
            if (watch == nullptr || watch->m_pBaseGeom == nullptr)
                return;

            const uint32_t itemRef = watch->m_pBaseGeom->GetRef();
            RefRun run;
            watch->m_Receivers.RunInitNxtRef(&run);
            for (uint32_t receiverRef = watch->m_Receivers.RunNxtRef(&run);
                 run._RunPtr != nullptr;
                 receiverRef = watch->m_Receivers.RunNxtRef(&run))
            {
                ZGEOM* receiver = ZGEOM::RefToPtr(receiverRef);
                if (receiver == nullptr)
                    continue;

                uint32_t data[2] = {itemRef, state};
                ScriptSendCommand(receiver, message, data);
            }
        }
    }

    ZItemWatch::ZItemWatch()
        : CBaseEvent<ZItem>(),
          m_Receivers(1, 0)
    {
    }

    const RTP::ZPropertyInfo& ZItemWatch::GetProperties() const
    {
        return ZItemWatch::Info;
    }

    void ZItemWatch::OnPickup()
    {
        SendItemWatch(this, kScriptSendItemWatch, 0);
    }

    void ZItemWatch::OnPutdown()
    {
        SendItemWatch(this, kScriptSendItemWatchPutdown, 1);
    }

    DEFINE_ROUT_CLASS(ZItemWatch, ZItem, ItemWatch, 0, 0, 0x00813EB0, nullptr, ZEventBase);
}
