#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/GUI/SMouseColi.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    // fwds
	struct ZWINDOWS;

	class ZWINGROUP : public ZGROUP
	{
	public:
	    // RTTI
		DECLARE_GEOM_CLASS(ZWINGROUP, 0x10002Eu);

		// vtbl
		~ZWINGROUP() override;

		// RTP::cBase
		const RTP::ZPropertyInfo& GetProperties() const override;

		// ZGEOM
		uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void SetPos(const Glacier::ZVector3& vPos) override;
        void ClassInit() override;
        int32_t ClassCommand(ZMSGID Msg, void* pData) override;
        void CopyData(const ZGEOM* Source) override;

		// ZWINGROUP
		virtual ZWINDOWS* GetSystem();
		virtual bool WndMessage(struct ZWMEVENT* event);
		virtual void GetMouseColi(SMouseColi& coli, const ZVector3& vPos, const ZMat3x3& mMat);
		virtual void RecalcMaxMin();
		virtual void CalcRealCenSize(ZVector3& vCen, ZVector3& vSize) const;
		virtual void SetText(const char* text);
		virtual void SetAlpha(int lAlpha);
		virtual void SetLineSpacing(int lSpacing);
		virtual void ForceNoLineBreak(bool bLineNoBreak);

		// methods
		ZWINGROUP(const char* psName, ZBaseGeom* pBaseGeom);

		void SetPos(float x, float y, float z);

		// members
		ZWINDOWS* m_pSystem; //+0x4C
		bool m_bNoLineBreak; //+0x50
		RE_ADD_PADDING(3); //+0x51
	}; // total size is 0x54
	RE_VERIFY_SIZE(ZWINGROUP, 0x54); // Verified PC alloc

	RE_VERIFY_OFFSET(ZWINGROUP, m_pSystem, 0x4C);
	RE_VERIFY_OFFSET(ZWINGROUP, m_bNoLineBreak, 0x50);
}
