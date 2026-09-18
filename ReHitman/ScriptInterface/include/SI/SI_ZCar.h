#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ScriptEngine/Common.h>
#include <Glacier/ZREF.h>

namespace Glacier
{
    void Zcar__Moveto(ZREF rCar, v3 target, bool useDirection);
    void Zcar__Setspeedmultiplier(ZREF rCar, float multiplier);
    void Zcar__Settargetspeedmultiplier(ZREF rCar, float minMultiplier, float maxMultiplier);
    v3 Zcar__Getpathendpoint(ZREF rCar);
    v3 Zcar__Getpathstartpoint(ZREF rCar);
    void Zcar__Setpath(ZREF rCar, ZREF rPath, bool updatePosition);
    ZREF Zcar__Getposobj(ZREF rCar, const char* name);
    void Zcar__Attachactortoseat(ZREF rCar, ZREF rActor, int seatIndex);
    void Zcar__Detachactorfromseat(ZREF rCar, ZREF rActor, int seatIndex);
    void Zcar__Stop(ZREF rCar, float blendTime);
    void Zcar__Tempstop(ZREF rCar, float blendTime);
    void Zcar__Continue_(ZREF rCar);
    void Zcar__Hithitman(ZREF rCar);
    void Zcar__Setcarposdir(ZREF rCar, v3 position, v3 direction);
}
