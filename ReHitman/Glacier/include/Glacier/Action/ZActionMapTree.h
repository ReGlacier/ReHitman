#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Serializer/ZTokenStream.h>
#include <cstdint>


namespace Glacier
{
    class ZActionMapTree
    {
    public:
        // types
        enum EType 
        {
            eUNDEFINED = 0,
            eGET = 1,
            eCONSTANT = 2,
            eHOLD = 3,
            eTAP = 4,
            eRELEASE = 5,
            eDOWNEDGE = 6,
            eFASTTAP = 7,
            eDOUBLECLICK = 8,
            eHOLDDOWN = 9,
            eCLICKHOLD = 10,
            ePRESS = 11,
            eAND = 12,
            eOR = 13,
            eGT = 14,
            eLT = 15,
            eSEQUENCE = 16,
            eANALOG = 17,
            eRELATIVE = 18,
            ePLUS = 19,
            eMINUS = 20,
            eMULT = 21,
        };

        enum EModifiers {
            eNONE = 0,
            eNEG = 1,
            eDISABLED = 2,
            eOVERRIDDEN = 4,
            eSTATIC = 8,
            eDEBUGKEYS = 16,
            eALWAYS = 32,
        };


        // vtbl
        virtual ~ZActionMapTree();

        // methods
        ZActionMapTree();

        void Init();
        void Init(float fConstant);
        void Init(ZTokenStream::EToken eToken);
        void Init(ZTokenStream::EToken eToken, int iDeviceId, int iControlId);
        void Init(ZActionMapTree* gettree);
        void SetName(const char* psName);
        const char* GetName() const;
        void SetDevice(int iDevice);
        void RevertToDefault();
        void RemoveFromTree();
        void Display(int,char *);
        class ZActionMapTree* Parent() const;
        const char* GetKeyName() const;
        const char* GetSystemKeyName(bool firstKey) const;
        int GetDeviceId() const;
        int GetControlId() const;
        void ClearInputKeys();
        bool Digital();
        float Analog();
        int ActivatedBy() const;
        int Override(int* pDeviceId, int* pControlId);
        int AddChild(ZActionMapTree* pChild);
        void Neg();
        void Not();
        void Minus();
        ZActionMapTree* FirstChild() const;
        ZActionMapTree* Next() const;
        bool Disabled() const;

        // members
        class ZActionMapTree *m_pkParent;
        class ZActionMapTree *m_pkFirstChild;
        class ZActionMapTree *m_pkNext;
        uint8_t m_eType;
        uint8_t m_eMods;
        uint16_t m_iLastDevice;
        uint16_t m_iDeviceId;
        uint16_t m_iControlId;
        uint16_t m_iOrgDeviceId;
        uint16_t m_iOrgControlId;
        char m_acName[24];
        float m_fConstant;
        float m_fLastFired;
        class ZActionMapTree *m_pkGetTree;
    };
    RE_VERIFY_SIZE(ZActionMapTree, 0x40); // Verified
}