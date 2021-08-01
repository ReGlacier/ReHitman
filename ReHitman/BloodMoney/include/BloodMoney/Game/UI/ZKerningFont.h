#include <BloodMoney/Game/UI/ZTTFONT.h>

namespace Hitman::BloodMoney {
    class ZKerningFont : public ZTTFONT {
    public:
        // vftable
        // custom api
        static ZKerningFont* Create();

        // data (size is 0x9C, base size is 0x98)
        int m_field98;
    };
}