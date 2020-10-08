#pragma once

#include <Glacier/Glacier.h>

namespace Glacier
{
    class ZGameData
    {
    public:
        virtual void Release(bool);
        virtual void LoadSave(ISerializerStream &,bool);
        virtual void Init();
        virtual void Init2();
        virtual void RemapRefs(uint*, uint);
        virtual void OnLevelChangeBegin();
        virtual void OnLevelChangeLoadDone();
        virtual void OnLevelChangeFinish();
        virtual void PreFrameUpdate();
        virtual void PostFrameUpdate();
        virtual void AddGeomToCheckPoint(ZGEOM*);
        virtual void AddEventToCheckPoint(ZEventBase*);
        virtual void AddItemOnGround(ZItem*);
        virtual void RemoveItemOnGround(ZItem*);
        virtual void CheckPointSave(ZCheckPointBuffer &);
        virtual void CheckPointLoad(ZCheckPointBuffer &);
        virtual void __PURE_UNKNOWN_FUNC__();
        virtual bool IsFrontEnd();
        virtual ZGameStats* GetStats();
        virtual void InitExcludedAnimNames(uint *);
        virtual void* GetAnimExclude();
        virtual void InitWeaponHandles(uint *);
        virtual int32_t GetTotalWeaponPrimsCount();
        virtual int32_t GetTotalWeaponPrims();
        virtual int32_t GetUsedWeaponPrims();
        virtual int32_t GetBigWeaponPrimsCount();
        virtual void* GetBigWeaponPrims();
        virtual void OnDecalRemoval(uint);
        virtual void GetSuitcaseWeaponPrims(int&, uint*);
        virtual bool IsInCutsequence();
        virtual void LoadGameFailureScene();
        virtual void AddIconToText(const zstring& str, const char*, bool);
    };
}