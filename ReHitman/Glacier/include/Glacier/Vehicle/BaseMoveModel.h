#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>


namespace Glacier
{
    class ZVehicle;

    class BaseMoveModel
    {
    public:
        // vtbl
        virtual void Init();
        virtual void LoadSave(ISerializerStream&, bool);
        virtual void InitRootTM(const float*, const float*);
        virtual void FrameUpdate();
        virtual void OnCameraEnter();
        virtual void OnCameraLeave();
        virtual void LocalBoneUpdate();
        virtual void GlobalBoneUpdate();
        virtual void InitDirPos(const float*, const float*);
        virtual void OnPathFinished();
        
        // data
        ZVehicle* m_pVehicle;
        bool m_bMoveWithPath;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(BaseMoveModel, 0xC); // Verified
}