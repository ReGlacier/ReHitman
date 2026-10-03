#pragma once


namespace Glacier
{
    /**
     * From BMEdit project
     * Ref: https://github.com/ReGlacier/BMEdit/blob/codex/add-text-rendering-with-sdf-and-mesh-support/BMEdit/GameLib/Include/GameLib/PRM/PRMEntries.h#L218
     */
    enum EPrimType
    {
        PTSTRIP = 0,
		PTSTRIPBONES = 1,
		PTSPRITES = 2,
		// #3 MISSING
		PTSPLINES = 4,
		PTSTRIPBONESV = 5,
		PTDOT3STRIP = 6,
		PTOBJECTHEADER = 7,
		PTMESH = 8,
		// #9 MISSING
		PTOBJECTSCATTER = 10,
		PTWATERPATCH = 11,
		PTLIGHT = 12,
    };
}
