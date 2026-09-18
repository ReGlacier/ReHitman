#pragma once

#include <Glacier/ScriptEngine/Common.h>
#include <Glacier/ZREF.h>

namespace Glacier
{
    void Dialog__Startdialog(ZREF rActor, ZREF rPartner, const char* speechGroup, const char* speechGroup2, const char* speechGroup3);
    void Dialog__Startonelinerdialog(ZREF rActor, ZREF rPartner, const char* speechGroup);
    void Dialog__Startonelinerdialognoturn(ZREF rActor, ZREF rPartner, const char* speechGroup);
    void Dialog__Pushdialogcamera(ZREF rActor, ZREF rPartner);
    void Dialog__Popdialogcamera();
    void Dialog__Startdialogex(ZREF rActor, ZREF rPartner, const char* speechGroup, const char* speechGroup2, const char* speechGroup3, int unknown, char unknown2, char unknown3, char unknown4, char unknown5);
    bool Dialog__Isplaying();
    void Dialog__Setnotifytarget(ZREF rTarget, ZSC_EVENT event);
    void Dialog__Stopdialog();
    bool Dialog__Actorplaynextdialog(ZREF rActor, const char* speechGroupPrefix, int dialogIndex, const char* speechGroupSuffix);
    bool Dialog__Setdialogskipable(char skipable);
    void Dialog__Sethmposbeforedialog(v3 position);
}
