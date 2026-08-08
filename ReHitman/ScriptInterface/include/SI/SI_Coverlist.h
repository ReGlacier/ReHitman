#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    void Coverlist__Releasecover(ZREF rCoverList, sCover& cover);
    void Coverlist__Checkcoverfrompos(ZREF rCoverList, ZREF rCover, v3 position, float distance, eDirection direction, sCover& cover);
    void Coverlist__Getbestcoverfrompos(ZREF rCoverList, ZREF rCover, v3 position, float distance, eDirection direction, sCover& cover);
    void Coverlist__Getbestcover(ZREF rCoverList, ZREF rCover, ZREF rPosition, float distance, eDirection direction, sCover& cover);
    int Coverlist__Getcoverindexesaroundpos(ZREF rCoverList, v3 position, float distance, int maxCount, int& coverIndex);
    void Coverlist__Getpisscover(ZREF rCoverList, ZREF rCover, float distance, sCover& cover);
    void Coverlist__Getpissposdir(ZREF rCoverList, ZREF rCover, sCover& cover);
    void Coverlist__Getcoverposdir(ZREF rCoverList, sCover& cover);
    void Coverlist__Getcoverposdir_Pos(ZREF rCoverList, sCover& cover, v3 position);
    void Coverlist__Getcoverposition(ZREF rCoverList, int coverIndex, ZREF rPosition, v3& position);
    void Coverlist__Getshootposdir(ZREF rCoverList, sCover& cover, bool alternate);
    void Coverlist__Getshootposdir_Pos(ZREF rCoverList, sCover& cover, v3 position, bool alternate);
    void Coverlist__Getintermediateposition(ZREF rCoverList, sCover cover, v3 position, sCover target, float distance, v3& result);
}
