#include "query.hpp"

namespace Handler {

	bool Connect(const std::string &ipadd) {

		bool success = false;

		std::cout << "Connecting to server..." << std::endl;

		if (ipadd != "" && connsocket.getRemoteAddress() == sf::IpAddress::None) {

			sf::IpAddress ip(ipadd);
			sf::Socket::Status status = connsocket.connect(ip, 48888, sf::seconds(5));

			switch (status) {

			case sf::Socket::Status::Disconnected:
				{
					std::cout << "Disconnected." << std::endl;
				}
				break;
			case sf::Socket::Status::Error:
				{
					std::cout << "Connection Error." << std::endl;
				}
				break;
			case sf::Socket::Status::Done:
				{
					std::cout << "Connected to " + ipadd << std::endl;
					success = true;
				}
				break;
			}

		}

		return success;

	}

	bool Disconnect() {

		if (connsocket.getRemoteAddress() != sf::IpAddress::None) {
			connsocket.disconnect();
			std::cout << "Disconnected from remote address." << std::endl;
			return true;
		} else {
			return false;
		}

	}

	bool UpdateServerInfo() {

		bool success = false;

		if (connsocket.getRemoteAddress() != sf::IpAddress::None) {

			std::cout << "Getting server info..." << std::endl;

			squery::bfbc2 bfbc2 = squery::bfbc2();
			squery::bfbc2::server_info si = bfbc2.QueryServerInfo(connsocket);

			if (si.ip != sf::IpAddress::None) {

				// truncate name if too long
				std::string fullname = si.name;
				if (fullname.length() > 39) {
					fullname.erase(40, std::string::npos);
					fullname.append("...");
				}

				std::string servi = fullname + " - " + si.map + " (" + std::to_string(si.playersonline) + "/" + std::to_string(si.maxplayers) + ")";

				// populate box
				serverbox->copy_label(servi.c_str());

				// alert when server has players
				if (si.playersonline > 0) {
					serverbox->color(FL_GREEN);
				} else {
					serverbox->color(FL_RED);
				}

				success = true;

			}

		}

		return success;

	}

	bool UpdatePlayerInfo() {

		bool success = false;

		if (connsocket.getRemoteAddress() != sf::IpAddress::None) {

			squery::bfbc2 bfbc2 = squery::bfbc2();
			std::vector<squery::bfbc2::player_info> players = bfbc2.QueryPlayerInfo(connsocket);

			if (!players.empty()) {

				for (std::vector<squery::bfbc2::player_info>::iterator it = players.begin(); it != players.end(); ++it) {

					std::string tid;

					tid = (it->teamid == 1) ? "US" : "RU";

					std::string playerEntry =
						tid
						+ "\t"
						+ it->clantag 
						+ "\t" 
						+ it->name 
						+ "\t"  
						+ std::to_string(it->kills) 
						+ "/"
						+ std::to_string(it->deaths)
						+ "\t" 
						+ std::to_string(it->score)
						+ "\t"  
						+ std::to_string(it->ping);

					// populate player info box
					playerbox->add(playerEntry.c_str());

				}			

			} 

			success = true;

		}

		return success;

	}

}

int main(int argc, char **argv) {

	// DEBUG
	//squery::bfbc2 bfc;

	//Handler::Connect("ip here");
	//bfc.GetRawAnswer(Handler::connsocket);

	//bool run = false;
	//while (!run) {

	//}

	Handler::doc = new pugi::xml_document();

	std::string ourIP = "";

	// load/store ip for next run.
	if (Handler::doc->load_file("serv.xml") != pugi::xml_parse_status::status_file_not_found) {

		Handler::doc->append_child("ip");
		Handler::doc->save_file("serv.xml");

	} else {

		ourIP = Handler::doc->child_value("ip");

	}

	int posx = 48;
	int posy = 8;
	int windowsizex = 800;
	int windowsizey = 680;

	// WINDOW -- Main.
	Fl_Window *window = new Fl_Window(windowsizex, windowsizey, "BF:BC2 Server Query");
	window->icon((char *)LoadIcon(fl_display, "IDI_SMALL"));

	// INPUT / UPDATE BUTTON -- Ip
	Fl_Input *input_ip = new Fl_Input(posx + 80, posy, 200, 24, "IP Address: ");
	Fl_Button *input_ipsend = new Fl_Button(posx + 216 + 80, posy, 136, 24, "Update");
	input_ip->value(ourIP.c_str());
	input_ipsend->callback(CALLBACK_ipsend, input_ip);

	posy += 24 + 16;

	// BOX -- Server Details.
	Fl_Box *serverdetails  = new Fl_Box(posx, posy, windowsizex - 96, 24, "");
	serverdetails->box(FL_BORDER_BOX);
	serverdetails->labelsize(14);
	serverdetails->labelfont(FL_HELVETICA);

	posy += 24 + 16;

	// SCROLL -- For player list.
	Fl_Scroll *scroll = new Fl_Scroll(posx, posy, windowsizex - 96, windowsizey - 128);
	scroll->box(FL_DOWN_BOX);
	scroll->color((Fl_Color) FL_WHITE);
	scroll->type(FL_VERTICAL);
	scroll->begin();

	int bigwid = ((windowsizex - 96) * .4f) / 1;
	int smallwid = ((windowsizex - 96) * .6f) / 5;

	// BROWSER -- Player list.
	Fl_Browser *playerbrowser = new Fl_Browser(posx, posy, windowsizex - 96, windowsizey - 128);
	int widths[] = { smallwid, smallwid, bigwid, smallwid, smallwid, smallwid };           
	playerbrowser->column_widths(widths);
	playerbrowser->column_char('\t');
	playerbrowser->type(FL_MULTI_BROWSER);
	playerbrowser->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	playerbrowser->add("@bTEAM\t@bTAG\t@bNAME\t@bK/D\t@bSCORE\t@bPING");

	scroll->end();
	window->end();

	Handler::serverbox = serverdetails;
	Handler::playerbox = playerbrowser;

	window->show(argc, argv);

	while (Fl::wait() != 0)  {

		if (Handler::updating) {

			if (Handler::updateclock.getElapsedTime().asSeconds() > Handler::updateperiod) {

				CleanDisplay();
				RefreshData(input_ip->value());
				Handler::updateclock.restart();

			} else {

				std::string upd = "Updating in ";
				int timeleft = Handler::updateperiod - static_cast<int>(Handler::updateclock.getElapsedTime().asSeconds());
				upd.append(std::to_string(timeleft)).append("s...");
				input_ipsend->copy_label(upd.c_str());

			}

		}

		// Update at 30fps
		sf::sleep(sf::seconds(1/30.f));

	}

	// on close
	Handler::doc->save_file("serv.xml");
	delete Handler::doc;

}

void CALLBACK_ipsend (Fl_Widget *self, void *ipi) {

	Fl_Button *sendbutton = static_cast<Fl_Button *>(self);
	Fl_Input *ipinput = static_cast<Fl_Input *>(ipi);

	std::string labell = ipinput->value();

	CleanDisplay();

	if (RefreshData(labell)) {

		Handler::updating = true;
		Handler::updateclock.restart();

		Handler::doc->child("ip").text().set(labell.c_str());

	} else {

		Handler::updating = false;
		sendbutton->copy_label("Update Failed");
		Handler::serverbox->color(FL_BACKGROUND_COLOR);

		CleanDisplay();

	}


}

bool RefreshData(const sf::IpAddress &ipaddress) {

	int conncheck = 0;

	Handler::Disconnect();

	if (Handler::Connect(ipaddress.toString())) {
		conncheck++;
		if (Handler::UpdateServerInfo()) {
			conncheck++;
		}

		Handler::Disconnect();

		if (Handler::Connect(ipaddress.toString())) {
			conncheck++;
			if (Handler::UpdatePlayerInfo()) {
				conncheck++;
			}
		} 

		Handler::Disconnect();

	} 

	return (conncheck == 4) ? true : false;

}

void CleanDisplay() {

	Handler::serverbox->copy_label("");
	Handler::playerbox->clear();
	Handler::playerbox->add("@bTEAM\t@bTAG\t@bNAME\t@bK/D\t@bSCORE\t@bPING");

}
