/*

PACKET LAYOUT
=============

HEADER (12 Bytes)

int32 (4 Bytes)	- Signature.
int32 (4 Bytes)	- Size of packet.
int32 (4 Bytes)	- Number of WORDs in packet.

WORD
int32 (4 Bytes)	- Size of word excluding null terminator.
char[]			- Content of word.
char (1 Byte)	- Null terminator.


serverInfo request
==================

HEADER + 

0	WORD (Response -- "OK" == good, else = error) + 
1	WORD (Server Name) + 
2	WORD (Players Online) + 
3	WORD (Max Players Allowed) + 
4	WORD (Game Mode) + 
5	WORD (Map) + 
6	WORD (Rounds Played) + 
7	WORD (Rounds Total) + 
8	WORD (Team scores -- num entries) +
9	WORD (Team 1 score) +
10	WORD (Team 2 score) +
11	WORD (Target score) +
12	WORD (Online state) -- AcceptingPlayers, NotConnected... +
13	WORD (ranked) -- true or false +
14	WORD (punkbuster) -- true or false +
15	WORD (hasGamePassword) -- true or false +
16	WORD (serverUpTime) -- secs + 
17	WORD (roundtime) -- secs + 
18	WORD (gameMod) (usually BC2, may be other for Vietnam) + 
19	WORD (mapPack) -- integer (not sure) + 
20	WORD (externalGameIpAndPort) -- xxx.xxx.xxx.xxx:xxxxx format
21	WORD (punkBusterVersion) +
22	WORD (joinQueueEnabled) -- true or false +
23	WORD (serverRegion) -- two letter value?

==================

WORD (Team scores)
int32 (4 Bytes) Number of team scores that follow
int32 (content) scores team
...

listPlayer request
==================

HEADER + 

WORD (Response) + 
"OK" == good, else = error
WORD (Num of player categories (9)) + 
WORD (Categories) + 
Clantag
Name
Guid (zeroed out)
TeamId
SquadId
Kills
Deaths
Score
Ping
WORD (Each player and their category entries)

*/

#ifndef BFBC2_HPP
#define BFBC2_HPP

#include <SFML/Network.hpp>
#include <iostream>
#include <stdint.h>

namespace squery {

	class bfbc2 {

	public:

		// Server info block
		struct server_info {
			sf::IpAddress ip;
			std::string gamemode;
			std::string map;
			std::string name;
			int playersonline;
			int maxplayers;

			server_info() : 
				ip(sf::IpAddress::None) {}
		};

		// Player info block
		//  Clan Tag -- string
		//	Name -- string
		//	Guid -- GUID (zeroed)
		//	TeamId -- int (0 = neutral)
		//	SquadId -- int  (0 = no squad)
		//	Kills -- int
		//	Deaths -- int
		//	Score -- int
		//	Ping -- int
		struct player_info {
			std::string clantag;
			std::string name;
			std::string guid;
			int teamid;
			int squadid;
			int kills;
			int deaths;
			int score;
			int ping;
		};

	private:

		enum { PACKETSIZE = 4096, SERVERINFO_PACKETSIZE = 28, LISTPLAYER_PACKETSIZE = 37 };

		// server request packets
		char serverInfo_QueryPacket[SERVERINFO_PACKETSIZE];
		char listPlayer_QueryPacket[LISTPLAYER_PACKETSIZE];

		// parse packet WORD -- moves forward the passed index.
		std::string GetWordContent(const char (&buffer)[PACKETSIZE], int &index);

	public:

		// query server and returned parsed server_info
		server_info QueryServerInfo(sf::TcpSocket &socket);

		// query server and returned parsed player_info
		std::vector<player_info> QueryPlayerInfo(sf::TcpSocket &socket);

		// DEBUG
		//void GetRawAnswer(sf::TcpSocket &socket);

		bfbc2();

	};

}
#endif