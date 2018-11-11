#ifndef __TEAM_DUEL_OBSERVER_HPP__ 
#define __TEAM_DUEL_OBSERVER_HPP__
#include <map>
#include <memory>
#include <string>
#include "duel.hpp"
#include "duel_observer.hpp"

#include "enums/query.hpp"

namespace YGOpen
{
namespace Legacy
{

class ServerRoomClient;

class TeamDuelObserver : public DuelObserver
{
	const short team; // 0 or 1 (2 for spectators)
	bool responseFlag;
	std::weak_ptr<Duel> duel;
	std::map<int, std::shared_ptr<ServerRoomClient>> players;

	// Knowledge handling helper functions
	const bool IsCardPublic(const uint8_t location, const uint32_t position) const;
	const bool IsMsgForThisTeam(void* buffer, size_t length);
	const bool StripMessageKnowledge(void* buffer, size_t length, std::string& newMsg);
	const bool StripQueryKnowledge(void* buffer, size_t length, std::string& newMsg);

	// These functions handle queries sending. Queries are optional
	// information that can be retrieved from the core to be sent to the
	// client to obtain more information about the board or invididual cards
	void QueryLocation(int location, int flag, bool useCache = true);
	void QueryMonsterZones(int flag = mZoneDefQueryFlag, bool useCache = true);
	void QuerySpellZones(int flag = sZoneDefQueryFlag, bool useCache = true);
	void QueryHand(int playerPos, int flag = handDefQueryFlag, bool useCache = true);
	void QueryGrave(int playerPos, int flag = graveDefQueryFlag, bool useCache = true);
	void QueryExtra(int playerPos, int flag = extraDefQueryFlag, bool useCache = true);
	void QuerySingle(int playerPos, int location, int sequence, int flag = singleDefQueryFlag);
	void QueryDeckPseudo(int playerPos, int flag = deckDefQueryFlag);
	
	void HandleBeforeMsgQueries(void* buffer, size_t length);
	void HandleAfterMsgQueries(void* buffer, size_t length);
public:
	TeamDuelObserver(short team);

	virtual void OnNotify(void* buffer, size_t length);

	void AddPlayer(int pos, std::shared_ptr<ServerRoomClient> player);
	void SetDuel(std::weak_ptr<Duel> weakDuel);
	
	// Used to refresh extra deck atm but could be used for other things
	void Start();

	void Deinitialize();

	const bool IsReponseFlagSet() const;
};

} // namespace Legacy
} // namespace YGOpen

#endif // __TEAM_DUEL_OBSERVER_HPP__
