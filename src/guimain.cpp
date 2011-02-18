#include <wx-2.8/wx/wx.h>
#include <wx-2.8/wx/notebook.h>
#include "guimain.h"

bool AwesomeApp::OnInit() {
    AwesomeFrame *frame = new AwesomeFrame( _("Awesome-Wav"), wxPoint(50, 50), wxSize(450, 340) );
    frame->Show(true);
    SetTopWindow(frame);
    return true;
}

AwesomeFrame::AwesomeFrame(const wxString& title, const wxPoint& pos, const wxSize& size): wxFrame(NULL, -1, title, pos, size) {
	wxBoxSizer *fwvSizer = new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(this, -1);
	fwvSizer->Add(panel,1,wxEXPAND);
	wxButton *btnExecute = new wxButton(this, wxID_ANY, wxT("Execute"));
	fwvSizer->Add(btnExecute,0,wxALIGN_RIGHT|wxALL,3);
	this->SetSizer(fwvSizer);

	// Controls
	wxBoxSizer *vSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hSizer = new wxBoxSizer(wxHORIZONTAL);
	vSizer->Add(hSizer,0,wxEXPAND);
	wxString options[2] = {wxT("Encode"),wxT("Decode")};
	wxRadioBox *ecBox = new wxRadioBox(panel,ID_RADIOBOX,wxT("Action"),wxDefaultPosition,wxDefaultSize,2,options,wxRA_SPECIFY_COLS);
	hSizer->Add(ecBox,0,wxEXPAND|wxALL,3);
	// Input File
	wxBoxSizer *ifSizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *ifLabel =  new wxStaticText(panel, wxID_ANY, wxT("Input File: "));
	wxTextCtrl *infile = new wxTextCtrl(panel, wxID_ANY);
	ifSizer->Add(ifLabel,0,wxEXPAND|wxTOP,3);
	ifSizer->Add(infile,1,wxEXPAND);
	vSizer->Add(ifSizer,0,wxEXPAND|wxALL,3);
	// output file
	wxBoxSizer *ofSizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *ofLabel =  new wxStaticText(panel, wxID_ANY, wxT("Output File: "));
	wxTextCtrl *outfile = new wxTextCtrl(panel, wxID_ANY);
	ofSizer->Add(ofLabel,0,wxEXPAND|wxTOP,3);
	ofSizer->Add(outfile,1,wxEXPAND);
	vSizer->Add(ofSizer,0,wxEXPAND|wxALL,3);
	// Encryption
	wxCheckBox *Encrypt = new wxCheckBox(panel,ID_USEENCRYPTION,wxT("Use Encryption"));
	vSizer->Add(Encrypt,0,wxEXPAND|wxLEFT|wxRIGHT|wxTOP,3);
	
	wxBoxSizer *kSizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *kLabel =  new wxStaticText(panel, wxID_ANY, wxT("Key: "));
	wxTextCtrl *key = new wxTextCtrl(panel, wxID_ANY);
	kSizer->Add(kLabel,0,wxEXPAND|wxTOP,3);
	kSizer->Add(key,1,wxEXPAND);
	vSizer->Add(kSizer,0,wxEXPAND|wxALL,3);
	// Compression
	wxCheckBox *Compress = new wxCheckBox(panel,ID_USEENCRYPTION,wxT("Use Compression"));
	vSizer->Add(Compress,0,wxEXPAND|wxLEFT|wxRIGHT|wxTOP,3);
	
	panel->SetSizer(vSizer);
	
    //CreateStatusBar();
    SetStatusText( _("") );
}

AwesomeFrame::~AwesomeFrame() {
	wxDELETE(menuFile);
	wxDELETE(nb);
}

void AwesomeFrame::OnQuit(wxCommandEvent& WXUNUSED(event)) {
    Close(true);
}

void AwesomeFrame::OnAbout(wxCommandEvent& WXUNUSED(event)) {
    wxMessageBox( _("The awesome-wav project is a project designed to encode any data file into a PCM or IEEE float WAV audio file and be virtually undetectable."),
                  _("About Awesome-Wav"), 
                  wxOK | wxICON_INFORMATION, this );
}

BEGIN_EVENT_TABLE(AwesomeFrame, wxFrame)
	EVT_MENU(ID_Quit,  AwesomeFrame::OnQuit)
	EVT_MENU(ID_About, AwesomeFrame::OnAbout)
END_EVENT_TABLE()

IMPLEMENT_APP(AwesomeApp)
