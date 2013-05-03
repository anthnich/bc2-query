#ifndef	SERVERLIST_WINDOW_HPP
#define SERVERLIST_WINDOW_HPP

#include <FL/x.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Browser.H>

#include <vector>
#include "pugixml.hpp"
#include "bfbc2.hpp"

class ServerList_Window : public Fl_Window {

public:

	// for passing ip to other windows.
	std::string selectedIP;

private:

	static int serverbrowser_widths[];

	Fl_Input *input_ip;
	Fl_Button *input_ipsave;
	Fl_Button *input_ipremove;
	Fl_Input *input_name;
	Fl_Button *input_done;
	Fl_Scroll *scroll;
	Fl_Browser *serverbrowser;

	struct serverfileentry {
		std::string ip;
		std::string name;
	};

	std::vector<serverfileentry> serverbrowserentries;

	// an fltk bug forces me to save the width/height of a window
	// and resize it on each show.
	struct winvars {
		int x;
		int y;
		int w;
		int h;
	} windowvars;

	void ServerListFile_LoadEntries();
	void ServerListFile_SaveEntries();
	void ServerList_FillBrowser();

	// callbacks.
	static void CALLBACK_done(Fl_Widget *, void *data);
	static void CALLBACK_save(Fl_Widget *, void *data);
	static void CALLBACK_remove(Fl_Widget *, void *data);

public:

	void Think();

	ServerList_Window(const int width, const int height, const char *label = 0);

};
#endif
