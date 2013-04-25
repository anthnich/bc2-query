#include "bfbc2.hpp"

namespace squery {

	std::string bfbc2::GetWordContent(const char (&buffer)[PACKETSIZE], int &index) {

		// skip first 4 Bytes, ie size of word, then just ride out the content until we hit the null terminator.
		index += 4;

		std::string content;

		// go until we hit the null terminator for the content.
		while (buffer[index] != '\0') {
			content.push_back(buffer[index]);
			index++;
		}

		// skip the null terminator
		index++;

		return content;

	}

	bfbc2::server_info bfbc2::QueryServerInfo(sf::TcpSocket &socket) {

		server_info serverInfo;

		std::cout << "Sending serverInfo query..." << std::endl;

		if (socket.send(serverInfo_QueryPacket, sizeof(serverInfo_QueryPacket)) == sf::Socket::Status::Done) {

			std::cout << "Success! Waiting for response..." << std::endl;

			char buffer[PACKETSIZE];
			std::size_t received = 0;

			// receive response
			if (socket.receive(buffer, sizeof(buffer), received) == sf::Socket::Status::Done) {				

				if (received > 0) {

					std::cout << "Success! Processing..." << std::endl;

					// skip header.
					int index = 8;

					// get number of WORDs in packet, then store them for parsing.
					uint32_t numofwords = (static_cast<uint32_t>(buffer[index]) ) +
						(static_cast<uint32_t>(buffer[index + 1]) << 8) +
						(static_cast<uint32_t>(buffer[index + 2]) << 16)  +
						(static_cast<uint32_t>(buffer[index + 3]) << 24);

					// skip numwords
					index += 4;

					// extract the content of each word.
					std::vector<std::string> content;
					for (uint32_t i = 0; i < numofwords; i++) {
						content.push_back(GetWordContent(buffer, index));
					}

					// checks to see if we have the correct results
					if (!content.empty() && content.size() == 24) {

						//
						//	CHECKING FOR OK RESPONSE (.at(0)) & & IF ACCEPTINGPLAYERS )
						//
						if (content.at(0) == "OK") {

							if (content.at(12) == "AcceptingPlayers") {

								serverInfo.ip = socket.getRemoteAddress();
								serverInfo.name = content.at(1);
								serverInfo.playersonline = atoi(content.at(2).c_str());
								serverInfo.maxplayers = atoi(content.at(3).c_str());
								serverInfo.gamemode = content.at(4);
								std::string mapstring = content.at(5);

								// change map string to lowercase, as the map filename is stored in the server .cfg,
								// and the server admin can make it upper or lower case.
								std::transform(mapstring.begin(), mapstring.end(), mapstring.begin(), ::tolower);

								std::string map;

								if (mapstring.find("001") != std::string::npos) { map = "Panama Canal"; }
								else if (mapstring.find("mp_002") != std::string::npos) { map = "Valparaiso"; }
								else if (mapstring.find("003") != std::string::npos) { map = "Laguna Alta"; }
								else if (mapstring.find("004") != std::string::npos) { map = "Isla Inocentes"; }
								else if (mapstring.find("mp_005") != std::string::npos) { map = "Atacama Desert"; }
								else if (mapstring.find("006") != std::string::npos) { map = "Arica Harbor"; }
								else if (mapstring.find("007") != std::string::npos) { map = "White Pass"; }
								else if (mapstring.find("008") != std::string::npos) { map = "Nelson Bay"; }
								else if (mapstring.find("009") != std::string::npos) { map = "Laguna Presa"; }
								else if (mapstring.find("012") != std::string::npos) { map = "Port Valdez"; }
								else if (mapstring.find("oasis") != std::string::npos) { map = "Oasis"; }
								else if (mapstring.find("sp_005") != std::string::npos) { map = "Heavy Metal"; }
								else if (mapstring.find("sp_002") != std::string::npos) { map = "Cold War"; }
								else if (mapstring.find("harvest") != std::string::npos) { map = "Harvest Day"; }

								serverInfo.map = map;

							}

						}

					}

				}

			} // socket.receive

		} // socket.send

		// if send, receive, or content check fails anywhere, server_info.ip will return sf::IpAddress::None
		return serverInfo;

	}

	std::vector<bfbc2::player_info> bfbc2::QueryPlayerInfo(sf::TcpSocket &socket) {

		std::cout << "Sending playerInfo query..." << std::endl;

		std::vector<player_info> players;

		if (socket.send(listPlayer_QueryPacket, sizeof(listPlayer_QueryPacket)) == sf::Socket::Status::Done) {

			std::cout << "Success! Waiting for response..." << std::endl;

			char buffer[PACKETSIZE];
			std::size_t received = 0;

			// receive response
			if (socket.receive(buffer, sizeof(buffer), received) == sf::Socket::Status::Done) {

				if (received > 0) {

					std::cout << "Success! Processing..." << std::endl;

					// skip header
					int index = 8;

					uint32_t numofwords = (static_cast<uint32_t>(buffer[index]) ) +
						(static_cast<uint32_t>(buffer[index + 1]) << 8) +
						(static_cast<uint32_t>(buffer[index + 2]) << 16)  +
						(static_cast<uint32_t>(buffer[index + 3]) << 24);

					// skip numofwords
					index += 4;

					// extract the content of each word.
					std::vector<std::string> content;
					for (uint32_t i = 0; i < numofwords; i++) {
						content.push_back(GetWordContent(buffer, index));
					}

					// hIndex is used for parsing.
					int hIndex = 0;

					// check for content. we need at least 9 entries for the 
					// response and the categories, so if it didn't at least fill those, 
					// i'll know there was a kind of error.
					if (content.at(hIndex) == "OK" && content.size() >= 9) {

						// skip forward +2, 1 for response, other for numofcategories
						// i know the number, so no need to pull.
						hIndex += 2;

						// next is the actual categories, 9 of them.
						// skipping & not parsing the category names, already know them.
						hIndex += 9;

						// next is the player count on the server, read then skip
						int playerc = atoi(content.at(hIndex).c_str());
						hIndex++;

						// parse players and their category info
						for (int pc = 0; pc < playerc; pc++) {
							player_info p;
							p.clantag = content.at(hIndex++);
							p.name = content.at(hIndex++);
							p.guid = content.at(hIndex++);
							p.teamid = atoi(content.at(hIndex++).c_str());
							p.squadid = atoi(content.at(hIndex++).c_str());
							p.kills = atoi(content.at(hIndex++).c_str());
							p.deaths = atoi(content.at(hIndex++).c_str());
							p.score = atoi(content.at(hIndex++).c_str());
							p.ping = atoi(content.at(hIndex++).c_str());
							players.push_back(p);
						}

					}

				}

			} // socket.receive

		}  // socket.send

		return players;

	}

	// DEBUG
	//void bfbc2::GetRawAnswer(sf::TcpSocket &socket) {

	//	if (socket.send(serverInfo_QueryPacket, sizeof(serverInfo_QueryPacket)) == sf::Socket::Status::Done) {
	//		//if (socket.send(listPlayer_QueryPacket, sizeof(listPlayer_QueryPacket)) == sf::Socket::Status::Done) {

	//		//std::cout << "Query Received." << std::endl;

	//		// Receive response
	//		char buffer[PACKETSIZE];
	//		std::size_t received = 0;

	//		if (socket.receive(buffer, sizeof(buffer), received) == sf::Socket::Status::Done) {

	//			std::string answer(buffer, PACKETSIZE);
	//			std::cout << "Response: " << answer << std::endl;

	//		}

	//	}

	//}

	bfbc2::bfbc2() {

		memcpy(serverInfo_QueryPacket, "\x00\x00\x00\x00\x1b\x00\x00\x00\x01\x00\x00\x00\x0a\x00\x00\x00serverInfo\x00", sizeof(serverInfo_QueryPacket));
		memcpy(listPlayer_QueryPacket, "\x00\x00\x00\x00\x24\x00\x00\x00\x02\x00\x00\x00\x0b\x00\x00\x00listPlayers\x00\x03\x00\x00\x00\x61ll\x00", sizeof(listPlayer_QueryPacket));

	}

}