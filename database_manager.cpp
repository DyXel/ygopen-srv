#include "database_manager.hpp"
#include <sqlite3.h>
#include <cstring>

#include "enums/type.hpp"
#include "card.hpp"

namespace YGOpen
{

//static const char * errorString  = "???";
static const char* sqlSelect = "select * from datas,texts where datas.id=texts.id";

int DatabaseManager::CoreCardReader(unsigned int code, CardData* cd)
{
	const CardData* wantedCard = GetCardDataByCode(code);
	if(wantedCard == nullptr)
		std::memset((void*)cd, 0, sizeof(CardData));
	else
		std::memcpy((void*)cd, (void*)wantedCard, sizeof(CardData));
	return 0;
}

void DatabaseManager::ReadCardData(sqlite3_stmt* stmt, CardData* cd, CardDataExtra* cde)
{
	// Card Data
	cd->code = sqlite3_column_int(stmt, 0);
	// ot is 1, it is added to the extra card struct
	cd->alias = sqlite3_column_int(stmt, 2);
	cd->setcode = sqlite3_column_int64(stmt, 3);
	cd->type = sqlite3_column_int(stmt, 4);
	cd->attack = sqlite3_column_int(stmt, 5);
	cd->defense = sqlite3_column_int(stmt, 6);
	if(cd->type & TypeLink)
	{
		cd->link_marker = cd->defense;
		cd->defense = 0;
	}
	else cd->link_marker = 0;
	unsigned int level = sqlite3_column_int(stmt, 7);
	if((level & 0x80000000) != 0) // Negative levels are handled too
	{
		level = -level;
		cd->level = -(level & 0xFF);
	}
	else cd->level = level & 0xFF;
	cd->lscale = (level >> 24) & 0xFF;
	cd->rscale = (level >> 16) & 0xFF;
	cd->race = sqlite3_column_int(stmt, 8);
	cd->attribute = sqlite3_column_int(stmt, 9);

	// Card Data Extra
	cde->code = sqlite3_column_int(stmt, 0); 
	cde->ot = sqlite3_column_int(stmt, 1);
	cde->category = sqlite3_column_int(stmt, 10);
}

void DatabaseManager::ReadCardStrings(sqlite3_stmt* stmt, CardStrings* cs)
{
	// 11 is id again, skip it
	cs->name = std::string((const char*)sqlite3_column_text(stmt, 12));
	cs->desc = std::string((const char*)sqlite3_column_text(stmt, 13));
	
	for(int i = 14; i < 30; ++i)
		cs->str[i - 14] = std::string((const char*)sqlite3_column_text(stmt, i));
}

bool DatabaseManager::LoadDatabase(const char* fn)
{
	sqlite3* db;
	sqlite3_stmt* stmt;
	
	if(sqlite3_open_v2(fn, &db, SQLITE_OPEN_READONLY, 0) != SQLITE_OK)
	{
		puts(sqlite3_errmsg(db));
		return false;
	}

	if(sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, 0) != SQLITE_OK)
	{
		puts(sqlite3_errmsg(db));
		sqlite3_close(db);
		return false;
	}

	CardData cd;
	CardDataExtra cde;
	CardStrings cs;

	for(int step = SQLITE_OK; step != SQLITE_DONE; step = sqlite3_step(stmt))
	{
		if(step == SQLITE_BUSY || step == SQLITE_ERROR || step == SQLITE_MISUSE)
		{
			puts(sqlite3_errmsg(db));
			sqlite3_finalize(stmt);
			sqlite3_close(db);
			return false;
		}
		else if(step == SQLITE_ROW)
		{
			// Card Data
			ReadCardData(stmt, &cd, &cde);
			if(cardData.count(cd.code) > 0)
				cardData.erase(cd.code);
			cardData.insert(std::make_pair(cd.code, cd));
			if(cardDataExtra.count(cde.code) > 0)
				cardDataExtra.erase(cde.code);
			cardDataExtra.insert(std::make_pair(cde.code, cde));

			// Card Strings
			ReadCardStrings(stmt, &cs);
			if(cardStrings.count(cd.code) > 0)
				cardStrings.erase(cd.code);
			cardStrings.insert(std::make_pair(cd.code, cs));
		}
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);

	std::printf("Number of cards loaded in total: %lu\n", cardData.size());

	return true;
}

const CardData* DatabaseManager::GetCardDataByCode(unsigned int code) const
{
	auto search = cardData.find(code);
	if(search != cardData.end())
		return &search->second;
	return nullptr;
}

const CardDataExtra* DatabaseManager::GetCardDataExtraByCode(unsigned int code) const
{
	auto search = cardDataExtra.find(code);
	if(search != cardDataExtra.end())
		return &search->second;
	return nullptr;
}

const CardStrings* DatabaseManager::GetCardStringsByCode(unsigned int code) const
{
	auto search = cardStrings.find(code);
	if(search != cardStrings.end())
		return &search->second;
	return nullptr;
}

} // namespace YGOpen
