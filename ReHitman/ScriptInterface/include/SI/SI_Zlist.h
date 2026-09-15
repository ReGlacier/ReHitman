#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    ZREF Zlist__Getrndref(ZREF rList);
    int Zlist__Getcount(ZREF rList);
    ZREF Zlist__Getref(ZREF rList, int index);
    int Zlist__Getindex(ZREF rList, ZREF rItem);
    bool Zlist__Contains(ZREF rList, ZREF rItem);
    void Zlist__Add(ZREF rList, ZREF rItem);
    void Zlist__Remove(ZREF rList, ZREF rItem);
    void Zlist__Clear(ZREF rList);
    void Zlist__Sendeventtolist(ZREF rList, ZSC_EVENT event);
}
