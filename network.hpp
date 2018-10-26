#ifndef NETWORK_H
#define NETWORK_H
#include <cstdint>

namespace YGOpen
{
namespace Legacy
{

struct HostInfo {
	uint32_t lflist;
	uint8_t rule;
	uint8_t mode;
	uint8_t duel_rule;
	bool no_check_deck;
	bool no_shuffle_deck;
	uint32_t start_lp;
	uint8_t start_hand;
	uint8_t draw_count;
	uint16_t time_limit;
	uint8_t check;
	uint32_t duel_flag;
	int32_t forbiddentypes;
	uint16_t extra_rules;
};

struct HostPacket {
	uint16_t identifier;
	uint16_t version;
	uint16_t port;
	uint32_t ipaddr;
	uint16_t name[20];
	HostInfo host;
};

struct HostRequest {
	uint16_t identifier;
};

namespace StoC
{

enum class Msg : uint8_t
{
	GameMsg = 0x1,
	ErrorMsg = 0x2,
	SelectHand = 0x3,
	SelectTp = 0x4,
	HandResult = 0x5,
	TpResult = 0x6,
	ChangeSide = 0x7,
	WaitingSide = 0x8,
	CreateGame = 0x11,
	JoinGame = 0x12,
	TypeChange = 0x13,
	LeaveGame = 0x14,
	DuelStart = 0x15,
	DuelEnd = 0x16,
	Replay = 0x17,
	TimeLimit = 0x18,
	Chat = 0x19,
	HsPlayerEnter = 0x20,
	HsPlayerChange = 0x21,
	HsWatchChange = 0x22,
	NewReplay = 0x30
};

struct CreateGame {
	uint32_t gameid;
};
struct JoinGame {
	HostInfo info;
};
struct TypeChange {
	unsigned char type;
};
struct ExitGame {
	unsigned char pos;
};
struct TimeLimit {
	unsigned char player;
	uint16_t left_time;
};
struct Chat {
	uint16_t player;
	uint16_t msg[256];
};
struct HS_PlayerEnter {
	uint16_t name[20];
	unsigned char pos;
};
struct HS_PlayerChange {
	//pos<<4 | state
	unsigned char status;
};
struct HS_WatchChange {
	uint16_t watch_count;
};

}

namespace CtoS
{

enum class Msg : uint8_t
{
	Response = 0x1,
	UpdateDeck = 0x2,
	HandResult = 0x3,
	TpResult = 0x4,
	PlayerInfo = 0x10,
	CreateGame = 0x11,
	JoinGame = 0x12,
	LeaveGame = 0x13,
	Surrender = 0x14,
	TimeConfirm = 0x15,
	Chat = 0x16,
	HsToDuelist = 0x20,
	HsToObserver = 0x21,
	HsReady = 0x22,
	HsNotReady = 0x23,
	HsKick = 0x24,
	HsStart = 0x25
};

}
}
}


// Too tedious to change these to enums
// NOTE: Dont use outside of Legacy server
#define PLAYERCHANGE_OBSERVE	0x8
#define PLAYERCHANGE_READY		0x9
#define PLAYERCHANGE_NOTREADY	0xa
#define PLAYERCHANGE_LEAVE		0xb

#define ERRMSG_JOINERROR	0x1
#define ERRMSG_DECKERROR	0x2
#define ERRMSG_SIDEERROR	0x3
#define ERRMSG_VERERROR		0x4

#define DECKERROR_LFLIST		0x1
#define DECKERROR_OCGONLY		0x2
#define DECKERROR_TCGONLY		0x3
#define DECKERROR_UNKNOWNCARD	0x4
#define DECKERROR_CARDCOUNT		0x5
#define DECKERROR_MAINCOUNT		0x6
#define DECKERROR_EXTRACOUNT	0x7
#define DECKERROR_SIDECOUNT		0x8
#define DECKERROR_FORBTYPE		0x9

#define NETPLAYER_TYPE_PLAYER1		0
#define NETPLAYER_TYPE_PLAYER2		1
#define NETPLAYER_TYPE_PLAYER3		2
#define NETPLAYER_TYPE_PLAYER4		3
#define NETPLAYER_TYPE_PLAYER5		4
#define NETPLAYER_TYPE_PLAYER6		5
#define NETPLAYER_TYPE_OBSERVER		7

/*
namespace ygo {
struct CTOS_HandResult {
	unsigned char res;
};
struct CTOS_TPResult {
	unsigned char res;
};
struct CTOS_PlayerInfo {
	uint16_t name[20];
};
struct CTOS_CreateGame {
	HostInfo info;
	uint16_t name[20];
	uint16_t pass[20];
};
struct CTOS_JoinGame {
	uint16_t version;
	uint32_t gameid;
	uint16_t pass[20];
};
struct CTOS_Kick {
	unsigned char pos;
};


}

#define NETWORK_SERVER_ID	0x7428
#define NETWORK_CLIENT_ID	0xdef6

#define CTOS_RESPONSE		0x1
#define CTOS_UPDATE_DECK	0x2
#define CTOS_HAND_RESULT	0x3
#define CTOS_TP_RESULT		0x4
#define CTOS_PLAYER_INFO	0x10
#define CTOS_CREATE_GAME	0x11
#define CTOS_JOIN_GAME		0x12
#define CTOS_LEAVE_GAME		0x13
#define CTOS_SURRENDER		0x14
#define CTOS_TIME_CONFIRM	0x15
#define CTOS_CHAT			0x16
#define CTOS_HS_TODUELIST	0x20
#define CTOS_HS_TOOBSERVER	0x21
#define CTOS_HS_READY		0x22
#define CTOS_HS_NOTREADY	0x23
#define CTOS_HS_KICK		0x24
#define CTOS_HS_START		0x25



#define MODE_SINGLE		0x0
#define MODE_MATCH		0x1
#define MODE_TAG		0x2
#define MODE_RELAY		0x3
*/

#endif //NETWORK_H
