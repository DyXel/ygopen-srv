#ifndef __TYPE_HPP__
#define __TYPE_HPP__

enum Type
{
	TypeMonster       = 0x1,
	TypeSpell         = 0x2,
	TypeTrap          = 0x4,
	TypeNormal        = 0x10,
	TypeEffect        = 0x20,
	TypeFusion        = 0x40,
	TypeRitual        = 0x80,
	TypeTrapMonster   = 0x100,
	TypeSpirit        = 0x200,
	TypeUnion         = 0x400,
	TypeDual          = 0x800, // Gemini
	TypeTuner         = 0x1000,
	TypeSynchro       = 0x2000,
	TypeToken         = 0x4000,
	TypeQuickPlay     = 0x10000,
	TypeContinous     = 0x20000,
	TypeEquip         = 0x40000,
	TypeField         = 0x80000,
	TypeCounter       = 0x100000,
	TypeFlip          = 0x200000,
	TypeToon          = 0x400000,
	TypeXyz           = 0x800000,
	TypePendulum      = 0x1000000,
	TypeSpecialSummon = 0x2000000,
	TypeLink          = 0x4000000
};

#endif // __TYPE_HPP__
