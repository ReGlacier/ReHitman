#pragma once

namespace Glacier {
    class ZOffsetAlloc {
    public:
        // virtual methods
        virtual ~ZOffsetAlloc();

        // data
        int m_field0;
        void* m_pLocation;
        int m_field8;
        int m_iChunkSize; //RLY?
        bool m_bIsPlacementAllocated;
        bool m_field11;
        bool m_field12;
        bool m_field13;
    };
}