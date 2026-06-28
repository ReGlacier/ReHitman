#pragma once

#include <Glacier/ZSysInput.h>

namespace Glacier {
	class ZSysInputWintel : public ZSysInput {
	public:
		// vftable (no changes)
		int m_field98;
		int m_field9C;
		int m_fieldA0;
		int m_fieldA4;
		int m_fieldA8;
	}; // total size is 0xAC (confirmed by allocation)
}