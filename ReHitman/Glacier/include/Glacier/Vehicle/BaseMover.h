#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/PF4/ZDynamicObstacle.h>


namespace Glacier
{
    class ZSHAPE;
    class PathFinderIF;
    class ZVehicle;
    class ZSoundObject;

    struct ZBaseMoverAudio 
    {
        ZVehicle* m_pVehicle;
        bool m_pAudio_IsRunnin;
        ZSoundObject* m_pAudio_Running;
        ZSoundObject* m_pAudio_Skid;
    };
    RE_VERIFY_SIZE(ZBaseMoverAudio, 0x10);

    struct BaseMover
    {
        // vtbl
        virtual ~BaseMover();
        virtual void Init();
        virtual void LoadSave(ISerializerStream&, bool);
        virtual void InitPos(const ZVector3* pPos);
        virtual void SetSpline(ZSHAPE*);
        virtual void FrameUpdate();
        virtual void MoveTo(const ZVector3* pPos, bool);
        virtual void SetMoveParams(float, float, float, float);
        virtual void Stop(float);
        virtual const float SteerAngle();
        virtual float* Pos();
        virtual float* Dir();
        virtual const float Spd();
        virtual const float Acc();
        virtual const float DirFactor();
        virtual const bool IsHumanMover();
        virtual PathFinderIF* PathFinder() const;
        virtual void PathFinder(PathFinderIF*);

        // data
        ZVehicle* m_pVehicle;
        float m_fWarnLookAheadTim;
        ZStackArray<2,PF4::ZDynamicObstacle> m_DynObstacles;
        ZVector3 m_vWarningBoxSize;
        ZBaseMoverAudio m_Audio;
    };
    RE_VERIFY_SIZE(BaseMover, 0x324);
}