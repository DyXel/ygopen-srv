#ifndef __TEAM_DUEL_OBSERVER_HPP__ 
#define __TEAM_DUEL_OBSERVER_HPP__
#include <map>
#include <memory>
#include "duel_observer.hpp"

class ServerRoomClient;

class TeamDuelObserver : public DuelObserver
{
	short team; // 0 or 1
	bool responseFlag;
	std::map<int, std::shared_ptr<ServerRoomClient>> players;
public:
	TeamDuelObserver(short team);

	void AddPlayer(int pos, std::shared_ptr<ServerRoomClient> player);
	void ClearPlayers();
	const bool IsReponseFlagSet() const;
	virtual void OnNotify(void* buffer, size_t length);
};

#endif // __TEAM_DUEL_OBSERVER_HPP__
