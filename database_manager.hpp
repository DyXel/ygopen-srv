#ifndef __DATABASE_MANAGER__
#define __DATABASE_MANAGER__
#include <string>
#include <unordered_map>
#include <functional>
#include "card.hpp"

struct sqlite3_stmt;

namespace YGOpen
{

class DatabaseManager
{
public:
	bool LoadDatabase(const char* filepath);

	const CardData* GetCardDataByCode(unsigned int code) const;
	const CardDataExtra* GetCardDataExtraByCode(unsigned int code) const;
	const CardStrings* GetCardStringsByCode(unsigned int code) const;
private:
	std::unordered_map<unsigned int, CardData> cardData;
	std::unordered_map<unsigned int, CardDataExtra> cardDataExtra;
	std::unordered_map<unsigned int, CardStrings> cardStrings;
	
	int CoreCardReader(unsigned int code, CardData* cd);
	
	void ReadCardData(sqlite3_stmt* stmt, CardData* cd, CardDataExtra* cde);
	void ReadCardStrings(sqlite3_stmt* stmt, CardStrings* cs) ;
};

} // namespace YGOpen

#endif // __DATABASE_MANAGER__
