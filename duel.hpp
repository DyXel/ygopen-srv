#ifndef __DUEL_HPP__
#define __DUEL_HPP__
#include <string>
#include <vector>
#include <utility>

#include "util/buffer_manipulator.hpp"

#define DUEL_BUFFER_SIZE 4096 // size took from core/duel.cpp

#include "enums/core_message.hpp"

namespace YGOpen
{

enum class DuelMessage : int
{
	Continue = 0,
	NeedResponse = 1,
	EndOfDuel = 2,
};

class CoreInterface;
class DuelObserver;

class Duel
{
	CoreInterface& core;
	std::vector<uint8_t> buffer;
	std::vector<uint8_t> queryBuffer;
	uintptr_t pduel;

	std::vector<DuelObserver*> observers;

	DuelMessage Analyze(unsigned int bufferLen);
	DuelMessage HandleCoreMessage(CoreMessage msgType, BufferManipulator* bm);

	void Message(void* buff, size_t length);
public:
	Duel(CoreInterface& core, unsigned int seed);
	~Duel();

	void AddObserver(DuelObserver* duelObs);

	void Start(int options);

	void PreloadScript(const std::string& file);

	void SetPlayerInfo(int playerID, int startLP, int startHand, int drawCount);

	void Process();

	void NewCard(int code, int owner, int playerID, int location, int sequence, int position, int duelist);

	// Any call to the following functions invalidates the result of the last function called
	std::pair<void*, size_t> QueryCard(int playerID, int location, int sequence, int queryFlag, bool useCache = false);
	int QueryFieldCount(int playerID, int location);
	std::pair<void*, size_t> QueryFieldCard(int playerID, int location, int queryFlag, bool useCache = false);
	std::pair<void*, size_t> QueryFieldInfo();
	
	void SetResponseInteger(int val);
	void SetResponseBuffer(void* buff, size_t length);

	//void SetDeck(int player, std::vector<unsigned int>& deck);
	//void SetExtraDeck(int player, std::vector<unsigned int>& deck);
};

} // namespace YGOpen

#endif // __DUEL_HPP__
