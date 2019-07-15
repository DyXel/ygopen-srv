#ifndef __LOCATION_HPP__
#define __LOCATION_HPP__

enum Location
{
	LocationMainDeck     = 0x1,
	LocationHand         = 0x2,
	LocationMonsterZone  = 0x4,
	LocationSpellZone    = 0x8,
	LocationGraveyard    = 0x10,
	LocationBanished     = 0x20,
	LocationExtraDeck    = 0x40,
	LocationOverlay      = 0x80,
	LocationOnField      = 0x0C,
	LocationFieldZone    = 0x100,
	LocationPendulumZone = 0x200
};

#endif // __LOCATION_HPP__
