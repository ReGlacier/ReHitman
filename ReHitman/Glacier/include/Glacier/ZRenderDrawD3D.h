#pragma once

namespace Glacier {
    class IView;
    class ZRenderWintelD3D;

    enum EViewTarget {
        DEFAULT_TARGET = 1
    };

    class ZRenderDrawD3D {
    public:
        // vftable
        virtual bool IsValid();
        virtual bool ShouldFlush();
        virtual void InitAllocation();
        virtual int GetLargestFreeBlock(); // always 0
        virtual void* Alloc(int size, const char* source, unsigned int line); // source and line are unused
        virtual void Free(void* ptr, int);
        virtual void* AllocSprites(int count);
        virtual void FreeSprites(void* ptr, int);
        virtual IView* CreateView(ZRenderWintelD3D* pRender, unsigned int, EViewTarget target);
        virtual IView* FindView(unsigned int);
        virtual IView* GetViewByIndex(int);
        virtual void RemoveView(IView* pView);
        virtual void RemoveViewsUsingRender(ZRenderWintelD3D* pRender);
        virtual void Init();
        virtual void End();
        virtual void Unknown_0(); //?
        virtual void Unknown_1(); //?
        virtual void Unknown_2(); //?
        virtual void NullStub_0(); //?
        virtual void Flush();
        /// ... Unknown entities ...

        // size (0x58864)
    };
}