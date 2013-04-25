//#pragma comment( linker, "/subsystem:windows" )
//#pragma comment( linker, "/subsystem:console" )

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Browser.H>

#include "pugixml.hpp"
#include "bfbc2.hpp"

namespace Handler {

	sf::TcpSocket connsocket;

	// for saving ip
	pugi::xml_document *doc = NULL;

	// widgets that are populated with info
	Fl_Box *serverbox = NULL;
	Fl_Browser *playerbox = NULL;

	// periodic updates from server
	bool updating = false;
	int updateperiod = 120;
	sf::Clock updateclock;

	// setup connection
	bool Connect(const std::string &ipadd);
	bool Disconnect();

	// true on successful update
	bool UpdateServerInfo();
	bool UpdatePlayerInfo();

}

// update button callback
void CALLBACK_ipsend (Fl_Widget *self, void *ipi);

bool RefreshData(const sf::IpAddress &ipaddress);

void CleanDisplay();