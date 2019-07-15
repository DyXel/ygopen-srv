#ifndef __STATUS_HPP__
#define __STATUS_HPP__

enum Status
{
	StatusDisabled         = 0x1,
	StatusToEnable         = 0x2,
	StatusToDisable        = 0x4,
	StatusProcComplete     = 0x8,
	StatusSetTurn          = 0x10,
	StatusNoLevel          = 0x20,
	StatusBattleResult     = 0x40,
	StatusSpSummonStep     = 0x80,
	StatusFormChanged      = 0x100,
	StatusSummoning        = 0x200,
	StatusEffectEnabled    = 0x400,
	StatusSummonTurn       = 0x800,
	StatusDestroyConfirmed = 0x1000,
	StatusLeaveConfirmed   = 0x2000,
	StatusBattleDestroyed  = 0x4000,
	StatusCopyingEffect    = 0x8000,
	StatusChaining         = 0x10000,
	StatusSummonDisabled   = 0x20000,
	StatusActivateDisabled = 0x40000,
	StatusEffectReplaced   = 0x80000,
	StatusFutureFusion     = 0x100000,
	StatusAttackCanceled   = 0x200000,
	StatusInitializing     = 0x400000,
	StatusActivated        = 0x800000,
	StatusJustPos          = 0x1000000,
	StatusContinousPos     = 0x2000000,
	StatusForbidden        = 0x4000000,
	StatusActFromHand      = 0x8000000,
	StatusOppBattle        = 0x10000000,
	StatusFlipSummonTurn   = 0x20000000,
	StatusSpSummonTurn     = 0x40000000
};

#endif // __STATUS_HPP__
