#pragma once

#include <Glacier/ScriptEngine/Common.h>
#include <Glacier/ZREF.h>

namespace Glacier
{
    void Actorcommunication__Registerradiouser(ZREF rActor, int iChannel);
    void Actorcommunication__Sendradiomessage(ZREF rActor, int iChannel, ZSC_EVENT event);
    void Actorcommunication__Sendradiomessagetoclosestn(ZREF rActor, int iChannel, int n, ZSC_EVENT event);
    void Actorcommunication__Sendradiomessagetoneedeforce(ZREF rActor, int iChannel, float force, ZSC_EVENT event);
    void Actorcommunication__Sendrangedmessage(ZREF rActor, float range, ZSC_EVENT event);
    void Actorcommunication__Sendglobalevent(ZREF rActor, ZSC_EVENT event);
    void Actorcommunication__Sendeventtoactorsinbox(ZREF rActor, ZSC_EVENT event);
    void Actorcommunication__Sendeventtoactorsinbox2(ZREF rActor, ZSC_EVENT event);
}
