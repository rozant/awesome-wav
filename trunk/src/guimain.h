#ifndef __main_h
#define __main_h
#include <wx-2.8/wx/wx.h>
#include <wx-2.8/wx/notebook.h>

// unique identifiers for menu options
enum {
    ID_Quit = 1,
    ID_About,
	ID_CHECKBOX,
	ID_RADIOBOX,
	ID_USEENCRYPTION
};

// Hey Look!, its my program!
class AwesomeApp: public wxApp {
    virtual bool OnInit();
};

// the main window
class AwesomeFrame: public wxFrame {
private:
	wxMenu *menuFile;
	wxNotebook *nb;
public:
    AwesomeFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	~AwesomeFrame();

    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    DECLARE_EVENT_TABLE()
};

#endif
