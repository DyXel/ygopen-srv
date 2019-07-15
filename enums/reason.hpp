#ifndef __REASON_HPP__
#define __REASON_HPP__

enum Reason
{
	ReasonDestroy    = 0x1,
	ReasonRelease    = 0x2,
	ReasonTemporary  = 0x4,
	ReasonMaterial   = 0x8,
	ReasonSummon     = 0x10,
	ReasonBattle     = 0x20,
	ReasonEffect     = 0x40,
	ReasonCost       = 0x80,
	ReasonAdjust     = 0x100,
	ReasonLostTarget = 0x200,
	ReasonRule       = 0x400,
	ReasonSpSummon   = 0x800,
	ReasonDisSummon  = 0x1000,
	ReasonFlip       = 0x2000,
	ReasonDiscard    = 0x4000,
	ReasonRDamage    = 0x8000,
	ReasonRRecover   = 0x10000,
	ReasonReturn     = 0x20000,
	ReasonFusion     = 0x40000,
	ReasonSynchro    = 0x80000,
	ReasonRitual     = 0x100000,
	ReasonXyz        = 0x200000,
	ReasonReplace    = 0x1000000,
	ReasonDraw       = 0x2000000,
	ReasonRedirect   = 0x4000000,
	// ReasonReveal  = 0x8000000,
	ReasonLink       = 0x10000000
};

#endif // __REASAON_HPP__
