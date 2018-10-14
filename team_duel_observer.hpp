#ifndef __TEAM_DUEL_OBSERVER_HPP__ 
#define __TEAM_DUEL_OBSERVER_HPP__
#include <map>
#include <memory>
#include <string>
#include "duel_observer.hpp"

class ServerRoomClient;

class TeamDuelObserver : public DuelObserver
{
	short team; // 0 or 1 (2 for spectators)
	bool responseFlag;
	std::map<int, std::shared_ptr<ServerRoomClient>> players;
	
	bool IsMsgForThisTeam(void* buffer, size_t length);
	bool StripKnowledge(void* buffer, size_t length, std::string& newMsg);
public:
	TeamDuelObserver(short team);

	virtual void OnNotify(void* buffer, size_t length);
	void AddPlayer(int pos, std::shared_ptr<ServerRoomClient> player);
	void ClearPlayers();
	const bool IsReponseFlagSet() const;
};

#endif // __TEAM_DUEL_OBSERVER_HPP__
