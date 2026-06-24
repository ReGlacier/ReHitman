#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>


namespace Glacier
{
	struct ZWINDOWS;

	class ZWINGROUP : public ZGROUP
	{
	public:
		// vtbl
		virtual ZWINDOWS* GetSystem();
		virtual bool WndMessage(struct ZWMEVENT* event);
		virtual void GetMouseColi(struct SMouseColi* coli, float* x, float* y);
		virtual void RecalcMaxMin();
		virtual void CalcRealCenSize(float* x, float* y);
		virtual void SetText(const char* text, int unk1, int unk2, int unk3, int* unk4);
		virtual void SetAlpha(int alpha);
		virtual void SetLineSpacing(int);
		virtual void ForceNoLineBreak(bool force);
		
		// members
		ZWINDOWS* m_pSystem; //+0x4C
		bool m_bNoLineBreak; //+0x50
		RE_ADD_PADDING(3); //+0x51
	}; // total size is 0x54
	RE_VERIFY_SIZE(ZWINGROUP, 0x54); // verified

	RE_VERIFY_OFFSET(ZWINGROUP, m_pSystem, 0x4C);
	RE_VERIFY_OFFSET(ZWINGROUP, m_bNoLineBreak, 0x50);
}
