#pragma once

#include <Glacier/ZSTL/SimpleXML.h>
#include <Glacier/LoaderSequence/ZLoader_Sequence_Info.h>

namespace Glacier
{
    enum eREAD_MODE : uint32_t
    {
        eNOTHING = 0,
        eGATHER_INFO = 1,
        eFILL_STRUCTS = 2,
    };

    // Parses a Loader_Sequence XML script into a ZLoader_Sequence_Info in two
    // passes: eGATHER_INFO counts key frames / collects picture names, then
    // eFILL_STRUCTS fills the key-frame and picture tables.
    // Original source: engine\enginedata\loader_sequence_script.cpp (XBOX_KL1 PDB).
    struct ZLoader_Sequence_Script_Reader : public Glacier::SimpleXML
    {
        // methods
        ZLoader_Sequence_Script_Reader();           // rev @ 0x4679F0

        void Clear();                               // rev @ 0x466C20
        void Load_Script(ZLoader_Sequence_Info* pLoader_Sequence_Info, const char* pScript_Text, uint32_t iScript_Text_Size); // rev @ 0x467A80

        uint32_t Nr_Key_Frames() const              { return m_iNr_Key_Frames; }
        uint32_t Nr_Pictures() const                { return m_iPicture_Name_Count; }

        // vtbl
        void startElement(const char* name, const char** attrs) override; // rev @ 0x467C70
        void endElement(const char* name) override; // empty body (PC vtable slot 1 -> nullsub_4)

    private:
        void Add_Picture_Name(const char* psName);  // rev @ 0x4672F0
        uint32_t Get_Picture_Nr(const char* psName); // rev @ 0x467500
        void Fill_Keys_Not_Entered();               // rev @ 0x466CB0
        void Get_Cds(float& fX, float& fY, const char* psValue); // rev @ 0x467560; splits "X,Y" into two floats

    public:
        // data
        float m_fLast_KeyFrame_Time;                // +0x1AC maximum parsed key-frame time
        float m_fScreen_Size_X;                     // +0x1B0 default 640.0
        float m_fScreen_Size_Y;                     // +0x1B4 default 400.0
        float m_fFull_Progress_Time;                // +0x1B8 default -1.0
        eREAD_MODE m_eRead_Mode;                    // +0x1BC
        uint32_t m_iNr_Key_Frames;                  // +0x1C0
        uint32_t m_iPicture_Name_Count;             // +0x1C4
        char* m_pPicture_Name_Buffer;               // +0x1C8 scratch name pool, grows x2 from 1024 bytes
        uint32_t m_iPicture_Name_Buffer_Size_Total; // +0x1CC
        uint32_t m_iPicture_Name_Buffer_Size_Used;  // +0x1D0
        uint32_t* m_pPicture_Name_Offsets_Buffer;   // +0x1D4 scratch offsets table, grows x2 from 128 entries
        uint32_t m_iPicture_Name_Offsets_Buffer_Size; // +0x1D8
        float m_fCurrent_Time;                      // +0x1DC current key-frame time while parsing (-10.0 = invalid)
        uint32_t m_iCurrent_Key_Frame_Nr;           // +0x1E0
        ZLoader_Sequence_Info* m_pLoader_Sequence_Info; // +0x1E4 target of the FILL pass
    };
    RE_VERIFY_SIZE(ZLoader_Sequence_Script_Reader, 0x1E8); // XBOX_KL1 PDB; PC field offsets confirmed in startElement @ 0x467C70
    RE_VERIFY_OFFSET(ZLoader_Sequence_Script_Reader, m_eRead_Mode, 0x1BC);
    RE_VERIFY_OFFSET(ZLoader_Sequence_Script_Reader, m_pLoader_Sequence_Info, 0x1E4);
}
