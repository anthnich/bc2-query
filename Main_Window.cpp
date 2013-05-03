#include "Main_Window.hpp"

int Main_Window::playerbrowser_widths[] = { 84, 84, 281, 84, 84, 84 };

void Main_Window::CALLBACK_ipsend(Fl_Widget *, void *data){

	Main_Window *win = static_cast<Main_Window *>(data);

	win->CleanDisplay();

	if (win->RefreshData(win->input_ip->value())) {

		win->updating = true;
		win->updateclock.restart();

	} else {

		win->updating = false;
		win->input_ipsend->copy_label("Update Failed");
		win->playerbrowser->color(FL_BACKGROUND_COLOR);

		win->CleanDisplay();

	}

}

void Main_Window::CALLBACK_menuquit(Fl_Widget *, void *data) {

	exit(0);

}

void Main_Window::CALLBACK_menuserverlist(Fl_Widget *, void *data) {

	Main_Window *win = static_cast<Main_Window *>(data);
	win->serverlist_window->Think();

}

bool Main_Window::Connect(const std::string &ipadd) {

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

bool Main_Window::Disconnect() {

	if (connsocket.getRemoteAddress() != sf::IpAddress::None) {
		connsocket.disconnect();
		std::cout << "Disconnected from remote address." << std::endl;
		return true;
	} else {
		return false;
	}

}

bool Main_Window::UpdateServerInfo() {

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
			serverdetails->copy_label(servi.c_str());

			// alert when server has players
			if (si.playersonline > 0) {
				serverdetails->color(FL_GREEN);
			} else {
				serverdetails->color(FL_RED);
			}

			success = true;

		}

	}

	return success;

}

bool Main_Window::UpdatePlayerInfo() {

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
				playerbrowser->add(playerEntry.c_str());

			}			

		} 

		success = true;

	}

	return success;

}

std::string Main_Window::ServerListFile_GetLastIP() {

	std::string ip = "";

	pugi::xml_document *doc = new pugi::xml_document();

	if (!doc->load_file("serv.xml") != pugi::xml_parse_status::status_file_not_found) {

		ip = doc->last_child().child_value("ip");

	}

	delete doc;

	return ip;

}

void Main_Window::CleanDisplay() {

	serverdetails->copy_label("");
	playerbrowser->clear();
	playerbrowser->add("@bTEAM\t@bTAG\t@bNAME\t@bK/D\t@bSCORE\t@bPING");

}

bool Main_Window::RefreshData(const sf::IpAddress &ipaddress) {

	int conncheck = 0;

	Disconnect();

	if (Connect(ipaddress.toString())) {
		conncheck++;
		if (UpdateServerInfo()) {
			conncheck++;
		}

		Disconnect();

		if (Connect(ipaddress.toString())) {
			conncheck++;
			if (UpdatePlayerInfo()) {
				conncheck++;
			}
		} 

		Disconnect();

	} 

	return (conncheck == 4) ? true : false;

}

void Main_Window::Think() {

	// check for ip selected from serverlist_window.
	if (!serverlist_window->shown() && serverlist_window->selectedIP != "") {

		if ( (serverlist_window->selectedIP != input_ip->value()) || !updating) {

			CleanDisplay();
			RefreshData(serverlist_window->selectedIP);
			updating = true;
			updateclock.restart();
			input_ip->value(serverlist_window->selectedIP.c_str());

			// wipe ip selected from serverlist_window so it doesn't override when we input directly.
			serverlist_window->selectedIP = "";

		}

	}

	if (updating) {

		if (updateclock.getElapsedTime().asSeconds() > updateperiod) {

			CleanDisplay();
			RefreshData(input_ip->value());
			updateclock.restart();

		} else {

			std::string upd = "Updating in ";
			int timeleft = updateperiod - static_cast<int>(updateclock.getElapsedTime().asSeconds());
			upd.append(std::to_string(timeleft)).append("s...");
			input_ipsend->copy_label(upd.c_str());

		}

	}


}

Main_Window::Main_Window(const int width, const int height, const char *label) :
	updating(false),
	updateperiod(120),
	Fl_Window(width, height, label) {

		std::string ourIP = ServerListFile_GetLastIP();

		windowvars.x = 64;
		windowvars.y = 64;
		windowvars.w = width;
		windowvars.h = height;

		int posx = 48;
		int posy = 40;
		int windowsizex = windowvars.w;
		int windowsizey = windowvars.h;

		// WINDOW -- Main.
		icon((char *)LoadIcon(fl_display, "IDI_SMALL"));
		color((Fl_Color) FL_LIGHT1);

		// Menu bar
		menu = new Fl_Menu_Bar(0, 0, windowsizex, 30);
		menu->box(FL_NO_BOX);	
		menu->down_box(FL_NO_BOX);
		menu->add("File/Server List", 0, CALLBACK_menuserverlist, (void*)this);
		menu->add("File/Quit", 0, CALLBACK_menuquit);

		// INPUT / UPDATE BUTTON -- Ip
		input_ip = new Fl_Input(posx + 80, posy, 200, 24, "IP Address: ");
		input_ipsend = new Fl_Button(posx + 216 + 80, posy, 136, 24, "Update");
		input_ip->value(ourIP.c_str());
		input_ipsend->callback(CALLBACK_ipsend, (void*)this);

		posy += 24 + 16;

		// BOX -- Server Details.
		serverdetails  = new Fl_Box(posx, posy, windowsizex - 96, 24, "");
		serverdetails->box(FL_BORDER_BOX);
		serverdetails->labelsize(14);
		serverdetails->labelfont(FL_HELVETICA);

		posy += 24 + 16;

		// SCROLL -- For player list.
		Fl_Scroll *scroll = new Fl_Scroll(posx, posy, windowsizex - 96, windowsizey - 160);
		scroll->box(FL_DOWN_BOX);
		scroll->color((Fl_Color) FL_WHITE);
		scroll->type(FL_VERTICAL);
		scroll->begin();

		// BROWSER -- Player list.
		playerbrowser = new Fl_Browser(posx, posy, windowsizex - 96, windowsizey - 160);
		playerbrowser->column_widths(playerbrowser_widths);
		playerbrowser->column_char('\t');
		playerbrowser->type(FL_MULTI_BROWSER);
		playerbrowser->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
		playerbrowser->add("@bTEAM\t@bTAG\t@bNAME\t@bK/D\t@bSCORE\t@bPING");

		scroll->end();
		end();

		serverlist_window = new ServerList_Window(640, 480, "Server List");

		resize(windowvars.x, windowvars.y, windowvars.w, windowvars.h);
		show();

}