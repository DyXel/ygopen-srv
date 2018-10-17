#ifndef __TEAM_DUEL_OBSERVER_HPP__ 
#define __TEAM_DUEL_OBSERVER_HPP__
#include <map>
#include <memory>
#include <string>
#include "duel.hpp"
#include "duel_observer.hpp"

#include "enums/query.hpp"

class ServerRoomClient;

class TeamDuelObserver : public DuelObserver
{
	short team; // 0 or 1 (2 for spectators)
	bool responseFlag;
	std::weak_ptr<Duel> duel;
	std::map<int, std::shared_ptr<ServerRoomClient>> players;

	bool IsMsgForThisTeam(void* buffer, size_t length);
	bool StripKnowledge(void* buffer, size_t length, std::string& newMsg);

	// These functions handle queries sending, the queries are _optional_
	// information that can be retrieved from the core to be send to the
	// client to obtain more information about the board or invididual cards
	void QueryMonsterZone(int playerPos, int flag = mZoneDefQueryFlag, bool useCache = true);
	void QuerySpellZone(int playerPos, int flag = sZoneDefQueryFlag, bool useCache = true);
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

	void Deinitialize();

	const bool IsReponseFlagSet() const;
};

#endif // __TEAM_DUEL_OBSERVER_HPP__
