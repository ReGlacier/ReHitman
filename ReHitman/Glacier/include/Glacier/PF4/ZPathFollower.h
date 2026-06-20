#pragma once

#include <Glacier/PF4/EPathStatus.h>
#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier::PF4
{
	struct ZMetaNode;

	struct ZPathFollower
	{
        virtual ZMetaNode* GetReservationNode() = 0;
        virtual void SetDestination(const ZLocation* destination, float speed, float unk1, bool unk2, float* forwardDir) = 0;
        virtual float* GetEndDir() = 0;
        virtual void GetDestination(ZLocation* outLocation) = 0;
        virtual void Advance(float deltaTime) = 0;
        virtual void GetPosition(float* outPos) = 0;
        virtual void GetDirection(float* outDir) = 0;
        virtual float GetMaxSpeed() = 0;
        virtual bool StopPath(float unk, float* outVec) = 0;
        virtual void GetSpeed(float* outSpeed) = 0;
        virtual bool DoLocomotion() = 0;
        virtual bool PushFailed() = 0;
        virtual void GetLean(float* outLean) = 0;
        virtual void SetSpeed(float* speed) = 0;
        virtual bool Notify() = 0;
        virtual bool PathDone() = 0;
        virtual void GetLocation(ZLocation* outLocation) = 0;
        virtual void GetInsideLocation(ZLocation* outLocation, bool unk) = 0;
        virtual int GetType() = 0;
        virtual int GetId() = 0;
        virtual const void* GetPath() = 0; // Тип ZPath*
        virtual bool PathValid() = 0;
        virtual void Teleport(float* pos) = 0;
        virtual void Teleport(const ZLocation* location) = 0;
        virtual void SetPushAble(bool bPushable) = 0;
        virtual bool PushAble() = 0;
        virtual void TeleportAndKeepDest(float* pos) = 0;
        virtual void TeleportAndKeepDest(const ZLocation* location) = 0;
        virtual bool DestinationSet() = 0;
        virtual EPathStatus PathFailed() = 0;
        virtual bool UpcommingReservations() = 0;
        virtual bool AtDestination() = 0;
        virtual bool OnSpecialLink() = 0;
        virtual int GetSpecialLinkAction() = 0;
        virtual void GetTurn180Pos(float* unk1, float* unk2) = 0;
        virtual float GetTurn180Dist() = 0;
        virtual void ClearTurn180(float unk) = 0;
        virtual void GetSpecialLinkPositions(float* start, float* end) = 0;
        virtual int GetNextSpecialLinkAction() = 0;
        virtual void* GetSpecialLinkUserData() = 0;
        virtual void* GetNextSpecialLinkUserData() = 0;
        virtual void GetNextSpecialLinkPositions(float* start, float* end) = 0;
        virtual void SetSpecialLinkProgress(float progress) = 0;
        virtual void SetSpecialLinkEndPos(float* pos) = 0;
        virtual float GetSpecialLinkProgress() = 0;
        virtual void GetNextEvent(float* unk1, int* unk2) = 0;
        virtual void SkipToLink() = 0;
        virtual float TotalRemaining() = 0;
        virtual bool Stop() = 0;
        virtual void SetVisualPriority(int priority) = 0;
        virtual void SetDesiredSpeed(float speed) = 0;
        virtual void Deactivate() = 0;
        virtual void Activate() = 0;
        virtual bool IsActive() = 0;
        virtual void SetKeyMask(uint16_t mask) = 0;
        virtual uint16_t GetKeyMask() = 0;

        // Data 
        /* 0x04 */ int16_t m_Type;
	};

    RE_VERIFY_SIZE(ZPathFollower, 0x8);
}