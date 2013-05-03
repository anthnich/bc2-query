#ifndef	MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <FL/x.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Browser.H>

#include "pugixml.hpp"
#include "bfbc2.hpp"

#include "ServerList_Window.hpp"

class Main_Window : public Fl_Window {
	
	static int playerbrowser_widths[];

	// an fltk bug forces me to save the width/height of a window
	// and resize it on each show.
	struct winvars {
		int x;
		int y;
		int w;
		int h;
	} windowvars;

	// widgets.
	Fl_Menu_Bar *menu;
	Fl_Input *input_ip;
	Fl_Button *input_ipsend;
	Fl_Box *serverdetails;
	Fl_Browser *playerbrowser;

	// sub windows.
	ServerList_Window *serverlist_window;

	bool updating;
	int updateperiod;
	sf::Clock updateclock;

	sf::TcpSocket connsocket;

	// callbacks.
	static void CALLBACK_ipsend(Fl_Widget *, void *data);
	static void CALLBACK_menuquit(Fl_Widget *, void *data);
	static void CALLBACK_menuserverlist(Fl_Widget *, void *data);

	// setup connection.
	bool Connect(const std::string &ipadd);
	bool Disconnect();

	// true on successful update.
	bool UpdateServerInfo();
	bool UpdatePlayerInfo();

	// get last saved ip from serv.xml.
	std::string ServerListFile_GetLastIP();

	// for displaying results.
	void CleanDisplay();
	bool RefreshData(const sf::IpAddress &ipaddress);

public:

	void Think();

	Main_Window(const int width, const int height, const char *label = 0);

};
#endif
