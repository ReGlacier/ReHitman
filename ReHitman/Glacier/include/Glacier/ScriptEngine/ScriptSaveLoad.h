#pragma once

#include <Glacier/ScriptEngine/Fwd.h>
#include <cstdint>


namespace Glacier
{
    void PrepareSave(_ScriptState* pScriptState);
    void PrepareScriptState(_ScriptState* pScriptState);
    void PrepareScriptVariables(_ScriptState* pScript);
    void PrepareStateVariables(_ScriptState* pScriptState);
    void PrepareData(uint16_t* , int8_t*);
    void PrepareAsyncCall(_AsyncCall_Struct* pAsyncCall);
    void PrepareMessageCue(_MessageCue* pQue);
    void PrepareVariables(_LocalVarEntry* pVar);
}