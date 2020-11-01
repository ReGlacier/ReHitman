#pragma once

namespace Glacier
{
    enum ZTextureType : unsigned int {
        BITMAP_PAL      = 0x50414C4E,
        BITMAP_PAL_OPAC = 0x50414C4F,
        BITMAP_32       = 0x52474241,
        BITMAP_U8V8     = 0x55385638,
        BITMAP_DXT1     = 0x44585431

        //Declared but values not recognized yet
        //BITMAP_DXT3,
        //BITMAP_I8,

    };


    /**
     * @note Conditions for DX3 and I8 formats in loader code

      v16 = type - 0x44585431;
          if ( !v16 )
          {
            v21 = ZSysMem::allocate(0x30, "engine\\drawing\\_wintel\\renderd3d\\renderwinteld3dtex.cpp", 778);
            LOBYTE(v54) = 12;
            if ( !v21 )
              goto LABEL_46;
            v19 = ZBitmapDXT1::Ctor((int)v21);
            goto LABEL_47;
          }
          v17 = v16 - 2;
          if ( !v17 )
          {
            v20 = ZSysMem::allocate(0x30, "engine\\drawing\\_wintel\\renderd3d\\renderwinteld3dtex.cpp", 781);
            LOBYTE(v54) = 13;
            if ( !v20 )
              goto LABEL_46;
            v19 = ZBitmapDXT3::Ctor((int)v20);
            goto LABEL_47;
          }
          if ( v17 == 0x4DFCBED )
          {
            v18 = ZSysMem::allocate(0x30, "engine\\drawing\\_wintel\\renderd3d\\renderwinteld3dtex.cpp", 769);
            LOBYTE(v54) = 9;
            if ( !v18 )
              goto LABEL_46;
            v19 = ZBitmapI8::Ctor((int)v18);
LABEL_47:
            LOBYTE(v54) = 6;
            v45[v12] = v19;
            goto LABEL_48;
          }
     */
}