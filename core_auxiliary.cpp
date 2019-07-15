#include <cstring>
#include "core_auxiliary.hpp"

#include "core_interface.hpp"
#include "database_manager.hpp"

namespace YGOpen
{

CoreInterface* CoreAuxiliary::ci = nullptr;
DatabaseManager* CoreAuxiliary::dbm = nullptr;

void CoreAuxiliary::SetCore(CoreInterface* core)
{
	ci = core;
}

void CoreAuxiliary::SetDatabaseManager(DatabaseManager* dbManager)
{
	dbm = dbManager;
}

unsigned int CoreAuxiliary::CoreCardReader(unsigned int code, CardData* cd)
{
	if(dbm == nullptr)
	{
		puts("Warning: no DatabaseManager set on CoreAuxiliary");
		std::memset((void*)cd, 0, sizeof(CardData));
		return 0;
	}
		
	const CardData* wantedCard = dbm->GetCardDataByCode(code);
	if(wantedCard == nullptr)
		std::memset((void*)cd, 0, sizeof(CardData));
	else
		std::memcpy((void*)cd, (void*)wantedCard, sizeof(CardData));
	return 0;
}

unsigned int CoreAuxiliary::CoreMessageHandler(void* pduel, unsigned int msgType)
{
	char buffer[256];
	ci->get_log_message((long)pduel, (unsigned char*)buffer);
	printf("Core Message (%d): %s\n", msgType, buffer);
	return 0;
}

} // namespace YGOpen
