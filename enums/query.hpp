#ifndef __QUERY_HPP__
#define __QUERY_HPP__

enum Query
{
	QueryCode        = 0x1,
	QueryPosition    = 0x2,
	QueryAlias       = 0x4,
	QueryType        = 0x8,
	QueryLevel       = 0x10,
	QueryRank        = 0x20,
	QueryAttribute   = 0x40,
	QueryRace        = 0x80,
	QueryAttack      = 0x100,
	QueryDefense     = 0x200,
	QueryBaseAttack  = 0x400,
	QueryBaseDefense = 0x800,
	QueryReason      = 0x1000,
	QueryReasonCard  = 0x2000,
	QueryEquipCard   = 0x4000,
	QueryTargetCard  = 0x8000,
	QueryOverlayCard = 0x10000,
	QueryCounters    = 0x20000,
	QueryOwner       = 0x40000,
	QueryStatus		 = 0x80000,
	QueryIsPublic    = 0x100000,
	QueryLScale      = 0x200000,
	QueryRSCale      = 0x400000,
	QueryLink        = 0x800000
};

constexpr int basicDefQueryFlag =
QueryCode + QueryPosition + QueryAlias + QueryType +
QueryLevel + QueryRank + QueryAttribute + QueryRace +
QueryAttack + QueryDefense + QueryBaseAttack + QueryBaseDefense +
QueryReason +
QueryStatus; // 0x81fff

constexpr int mZoneDefQueryFlag = basicDefQueryFlag + QueryLink; // 0x881fff

constexpr int sZoneDefQueryFlag = basicDefQueryFlag + QueryLScale + QueryRSCale; // 0x681fff

constexpr int handDefQueryFlag = basicDefQueryFlag + QueryIsPublic + QueryLScale + QueryRSCale; // 0x781fff

constexpr int graveDefQueryFlag = basicDefQueryFlag;

constexpr int extraDefQueryFlag = basicDefQueryFlag;

constexpr int singleDefQueryFlag = basicDefQueryFlag + QueryIsPublic + QueryLScale + QueryRSCale + QueryLink; // 0xf81fff

constexpr int deckDefQueryFlag = basicDefQueryFlag + QueryIsPublic; // 0x181fff

#endif // __QUERY_HPP__
