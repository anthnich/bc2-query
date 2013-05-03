#include "ServerList_Window.hpp"

int ServerList_Window::serverbrowser_widths[] = { 272, 272 };

void ServerList_Window::ServerListFile_LoadEntries() {

	serverbrowserentries.clear();

	pugi::xml_document *doc = new pugi::xml_document();

	if (!doc->load_file("serv.xml") != pugi::xml_parse_status::status_file_not_found) {

		for (pugi::xml_node xmlserver = doc->child("server"); xmlserver; xmlserver = xmlserver.next_sibling("server")) {

			serverfileentry sfe;
			sfe.ip = xmlserver.child_value("ip");
			sfe.name = xmlserver.child_value("name");

			serverbrowserentries.push_back(sfe);

		}

	}

	delete doc;

}

void ServerList_Window::ServerListFile_SaveEntries() {

	pugi::xml_document *doc = new pugi::xml_document();

	if (!serverbrowserentries.empty()) {

		for (int i = 0; i < serverbrowserentries.size(); i++) {

			pugi::xml_node server = doc->append_child("server");
			server.append_child("ip").text().set(serverbrowserentries.at(i).ip.c_str());
			server.append_child("name").text().set(serverbrowserentries.at(i).name.c_str());

		}

	}

	doc->save_file("serv.xml");

	delete doc;

}

void ServerList_Window::ServerList_FillBrowser() {

	if (!serverbrowserentries.empty()) {

		serverbrowser->clear();

		for (int i = 0; i < serverbrowserentries.size(); i++) {

			std::string ent = serverbrowserentries.at(i).name + "\t" + serverbrowserentries.at(i).ip;
			serverbrowser->add(ent.c_str());

		}

	}

}

void ServerList_Window::CALLBACK_done(Fl_Widget *, void *data) {

	ServerList_Window *win = static_cast<ServerList_Window *>(data);

	if (!win->serverbrowserentries.empty()) {
		// if selected. 0 means nothing has been selected.
		if (win->serverbrowser->value() > 0) 
			win->selectedIP = win->serverbrowserentries.at(win->serverbrowser->value() - 1).ip;
	}

	win->hide();

}

void ServerList_Window::CALLBACK_save(Fl_Widget *, void *data) {

	ServerList_Window *win = static_cast<ServerList_Window *>(data);
	serverfileentry sfe;

	sfe.ip = win->input_ip->value();
	sfe.name = win->input_name->value();

	if (sfe.ip != "") {
		win->serverbrowserentries.push_back(sfe);
		win->ServerListFile_SaveEntries();
		win->ServerList_FillBrowser();
	}

}

void ServerList_Window::CALLBACK_remove(Fl_Widget *, void *data) {

	ServerList_Window *win = static_cast<ServerList_Window *>(data);

	// remove from vector.
	if (win->serverbrowser->value() > 0)
		win->serverbrowserentries.erase(win->serverbrowserentries.begin() + (win->serverbrowser->value() - 1));

	// remove from server browser.
	win->serverbrowser->remove(win->serverbrowser->value());

	// save xml.
	win->ServerListFile_SaveEntries();

}

void ServerList_Window::Think() {

	// an fltk bug forces me to save the width/height of a window
	// and resize it on each show.
	resize(windowvars.x, windowvars.y, windowvars.w, windowvars.h);
	show();

}

ServerList_Window::ServerList_Window(const int width, const int height, const char *label) :
	Fl_Window(width, height, label) {

		windowvars.x = 64;
		windowvars.y = 64;
		windowvars.w = width;
		windowvars.h = height;

		int posx = 48;
		int posy = 8 + 24;

		// window.
		icon((char *)LoadIcon(fl_display, "IDI_SMALL"));
		color((Fl_Color) FL_LIGHT1);

		// INPUT / SAVE IP BUTTON 
		input_ip = new Fl_Input(posx + 48, posy, 148, 24, "IP:");

		posy += 32;

		input_name = new Fl_Input(posx + 48, posy, 148, 24, "Name:");

		input_ipsave = new Fl_Button(posx + 216, posy, 96, 24, "Save");
		input_ipsave->callback(CALLBACK_save, (void*)this);

		input_ipremove = new Fl_Button(posx + 216 + 112, posy, 96, 24, "Remove");
		input_ipremove->callback(CALLBACK_remove, (void*)this);

		input_done = new Fl_Button(width - 96, height - 48, 80, 32, "Select");
		input_done->callback(CALLBACK_done, (void*)this);

		posy += 24 + 16;

		// SCROLL -- For player list.
		scroll = new Fl_Scroll(posx, posy, width - 96, height - 160);
		scroll->box(FL_DOWN_BOX);
		scroll->color((Fl_Color) FL_WHITE);
		scroll->type(FL_VERTICAL);
		scroll->begin();

		// BROWSER -- Player list.
		serverbrowser = new Fl_Browser(posx, posy, width - 96, height - 160);         
		serverbrowser->column_widths(serverbrowser_widths);
		serverbrowser->column_char('\t');
		serverbrowser->type(FL_MULTI_BROWSER);
		serverbrowser->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

		ServerListFile_LoadEntries();
		ServerList_FillBrowser();

		scroll->end();

		end();

		set_modal();

}