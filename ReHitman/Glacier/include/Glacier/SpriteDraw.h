#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    struct SDrawArray
    {
    };

    struct SSpriteClipPlanes
    {
        unsigned int lNrClipPlanes;
        float ClipPlanes[24];
    };
    RE_VERIFY_SIZE(SSpriteClipPlanes, 0x64);

    struct SSpriteArray : public SDrawArray
    {
        int lNumSprites;
        unsigned int lDrawMask;

        union {
            struct SSpriteArrayElement* pSprites;
            struct SSpriteArrayElementUV* pSpritesUV;
            struct SSpriteArrayElementRaw* pSpritesRaw;
            struct SSpriteArrayElementParticle* pSpritesParticle;
        };
    };
    RE_VERIFY_SIZE(SSpriteArray, 0xC);

    struct SSpriteBox
    {
        float vCornerBot0[3];
        float vCornerBot1[3];
        float vCornerTop0[3];
        float vCornerTop1[3];
        float vDirectionBot0[3];
        float vDirectionBot1[3];
        float vDirectionTop0[3];
        float vDirectionTop1[3];
        unsigned int lMask;
        unsigned int Pad[3];
    };
    RE_VERIFY_SIZE(SSpriteBox, 0x70);

    struct SSpriteArrayBox : public SSpriteArray 
    {
        float fNear;
        float fFar;
        SSpriteClipPlanes* pSpriteClipPlanes;
        uint32_t lColor;
        float fDirectionScale;
        uint32_t lNrBoxes;
        SSpriteBox* pBoxes[32]; //[32]
    };
    RE_VERIFY_SIZE(SSpriteArrayBox, 0xA4);

    struct SSpriteArrayElement
    {
        ZVector3 p;
        float fScale;
        float fAngle;
        uint32_t lColor;
    };
    RE_VERIFY_SIZE(SSpriteArrayElement, 0x18);

    struct SSpriteArrayElementBox 
    {
        ZVector3 p;          // +0x00
        float    fScale;        // +0x0c
        float    fULeft;        // +0x10
        float    fURight;       // +0x14
        uint32_t lMask1;       // +0x18
        uint32_t lMask2;       // +0x1c
    };
    RE_VERIFY_SIZE(SSpriteArrayElementBox, 0x20);

    struct SSpriteArrayElementBoxDirection : public SSpriteArrayElementBox
    {
        ZVector3 vDirection;
        float lUnused;
    };
    RE_VERIFY_SIZE(SSpriteArrayElementBoxDirection, 0x30);

    struct SSpriteArrayElementUV
    {
        ZVector3 p;
        ZVector2 u;
        ZVector2 v;
        ZVector2 fScale;
        uint32_t lColor;
    };
    RE_VERIFY_SIZE(SSpriteArrayElementUV, 0x28);

    struct SSpriteArrayElementRaw
    {
        float p[3];
        float uv[8];
        float vertex[4][3];
        float fScale[2];
        uint32_t lColor[4];
    };
    RE_VERIFY_SIZE(SSpriteArrayElementRaw, 0x74);

    struct SSpriteArrayElementParticle
    {
        float vC1[3];
        float vC2[3];
        float fStartTime;
        float fEndTime;
        float fStartAngle;
        float fStartScale;
        uint8_t u8Flags;
        uint8_t u8Phase;
        uint8_t u8Extra1;
        uint8_t u8Extra2;
    };
    RE_VERIFY_SIZE(SSpriteArrayElementParticle, 0x2C);

    struct SSpriteArrayParticle : public SSpriteArray {
        float fMaxAge;
        float fScale;
        float fScaleVel;
        float fScaleAcc;
        float fAngleSpeed;
        float fAngleSpeedVel;
        float fAngleSpeedAcc;
        float fFriction;
        float fMotionStretch;
        bool bAlignWithDir;
        int32_t lColorRepeat;
        ZVector3 vScaledGravity;
        const uint32_t* piUnusedColorTable;
        const float* pfColorTable;
    };
    RE_VERIFY_SIZE(SSpriteArrayParticle, 0x4C);
}