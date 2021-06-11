#pragma once
#include <atlwin.h>

using namespace ATL;

class CSettingsDialog :
    public CAxDialogImpl< CSettingsDialog>
{
public:
    CSettingsDialog()
        : m_countdown(10)
        , m_fadeout(0)
    {}

	CSettingsDialog(DWORD countdown, DWORD fadeout)
		: m_countdown(countdown)
		, m_fadeout(fadeout)
	{}
    enum { IDD = IDD_SETTINGS_DIALOG };

	BEGIN_MSG_MAP(CSettingsDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		CHAIN_MSG_MAP(CAxDialogImpl<CSettingsDialog>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}

    DWORD m_countdown;
    DWORD m_fadeout;
};

