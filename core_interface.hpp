#ifndef __CORE_INTERFACE__
#define __CORE_INTERFACE__
#include <string>

#include "card.hpp"

namespace YGOpen
{

typedef unsigned char* (*script_reader)(const char*, int*);
typedef unsigned int (*card_reader)(unsigned int, CardData*);
typedef unsigned int (*message_handler)(void*, unsigned int);

class CoreInterface
{
public:
	// Core loading
	CoreInterface(bool Loadlibrary);
	bool LoadCore(const char* path);
	bool LoadCore();
	bool ReloadCore();

	bool IsLibraryLoaded();

	// Core functions
	void (*set_script_reader)(script_reader);
	void (*set_card_reader)(card_reader);
	void (*set_message_handler)(message_handler);

	uintptr_t(*create_duel)(unsigned int);
	void (*start_duel)(uintptr_t, int);
	void (*end_duel)(uintptr_t);
	void (*set_player_info)(uintptr_t, int, int, int, int);
	void (*get_log_message)(uintptr_t, unsigned char*);
	int (*get_message)(uintptr_t, unsigned char*);
	int (*process)(uintptr_t);
	void (*new_card)(uintptr_t, unsigned int, unsigned char, unsigned char, unsigned char, unsigned char, unsigned char, int);
	int (*get_cached_query)(uintptr_t pduel, unsigned char* buf);
	int (*query_card)(uintptr_t, unsigned char, unsigned char, unsigned char, int, unsigned char*, int, int);
	int (*query_field_count)(uintptr_t, unsigned char, unsigned char);
	int (*query_field_card)(uintptr_t, unsigned char, unsigned char, int, unsigned char*, int, int);
	int (*query_field_info)(uintptr_t, unsigned char*);
	void (*set_responsei)(uintptr_t, int);
	void (*set_responseb)(uintptr_t, unsigned char*, size_t);
	int (*preload_script)(uintptr_t, char*, int, int, char*);

	// Core extension functions
	int(*get_api_version)(int*);

	// Core unloading
	~CoreInterface();
	void UnloadCore();
private:
	std::string activeCorePath;
	void* handle;
	
	template<typename T>
	T LoadFunction(void* handle, T* func, const char* name, bool unload);
};

} // namespace YGOpen

#endif // __CORE_INTERFACE__
