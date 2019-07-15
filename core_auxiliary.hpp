#ifndef __CORE_AUXILIARY_HPP__
#define __CORE_AUXILIARY_HPP__
#include "card.hpp"

namespace YGOpen
{

class CoreInterface;
class DatabaseManager;

class CoreAuxiliary
{
	static CoreInterface* ci;
	static DatabaseManager* dbm;
public:
	static void SetCore(CoreInterface* core);
	static void SetDatabaseManager(DatabaseManager* dbManager);

	static unsigned char* CoreScriptReader(const char* scriptName, int*);
	static unsigned int CoreCardReader(unsigned int code, CardData* cd);
	static unsigned int CoreMessageHandler(void* pduel, unsigned int msgType);
};

} // namespace YGOpen

#endif // __CORE_AUXILIARY_HPP__
