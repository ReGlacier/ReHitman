#pragma once

#include <Glacier/ZSysInput.h>

namespace Glacier {
    class ZSysInputWintel : public ZSysInput {
    public:
    	// vftable (no changes)
    	// data (total size is 0xAC)
	    int m_unk4 { 0 };
	    int m_unk8 { 0 };
	    int m_unkC { 0 };
	    ZInputDevice* m_devices[32];
	    int m_attachedDevicesCount { 0 }; // 0x90
	    bool m_isSuspended { false }; // field94
	    bool m_field95;
	    bool m_field96;
	    bool m_field97;
	    int m_field98;
	    int m_field9C;
	    int m_fieldA0;
	    int m_fieldA4;
	    int m_fieldA8;
    };


  static_assert(offsetof(ZSysInputWintel, m_isSuspended) == 0x94, "ZSysInput bad offset!");
}