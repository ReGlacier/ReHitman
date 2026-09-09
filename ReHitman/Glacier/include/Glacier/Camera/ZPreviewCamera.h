#pragma once


#include <Glacier/ReGlacier.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZCAMERA.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class ZPreviewCamera : public CBaseEvent<ZCAMERA>
    {
    public:
        // RTTI
        DECLARE_ROUT_CLASS(ZPreviewCamera, ZCAMERA, PreviewCamera, 272, 0);

        // vtbl
        ~ZPreviewCamera() override;
        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;
        // ZEventBase
        void Init() override;
        void FrameUpdate() override;

        // methods
        ZPreviewCamera();
    };
    RE_VERIFY_SIZE(ZPreviewCamera, 0x30);
}
