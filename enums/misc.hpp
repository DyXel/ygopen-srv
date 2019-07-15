#ifndef __MISC_HPP__
#define __MISC_HPP__

enum class Assume
{
	AssumeCode       = 1,
	AssumeType       = 2,
	AssumeLevel      = 3,
	AssumeRank       = 4,
	AssumeAttribute  = 5,
	AssumeRace       = 6,
	AssumeAttack     = 7,
	AssumeDefense    = 8,
	AssumeLink       = 9,
	AssumeLinkMarker = 10
};

enum LinkMarker
{
	LinkMarkerBottomLeft  = 0x1,
	LinkMarkerBottom      = 0x2,
	LinkMarkerBottomRight = 0x4,
	LinkMarkerLeft        = 0x8,
	LinkMarkerRight       = 0x20,
	LinkMarkerTopLeft     = 0x40,
	LinkMarkerTop         = 0x80,
	LinkMarkerTopRight    = 0x100
};

// Flip effect flags
#define NO_FLIP_EFFECT     0x10000
#define FLIP_SET_AVAILABLE 0x20000

// Counter
#define COUNTER_WITHOUT_PERMIT 0x1000
#define COUNTER_NEED_ENABLE    0x2000

#endif // __MISC_HPP__
