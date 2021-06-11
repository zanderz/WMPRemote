#include "stdafx.h"
#include "SettingsDialog.h"

LRESULT CSettingsDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CAxDialogImpl<CSettingsDialog>::OnInitDialog(uMsg, wParam, lParam, bHandled);

	SetDlgItemInt(IDC_COUNTDOWN, m_countdown);
	SetDlgItemInt(IDC_FORCEFADE, m_fadeout);
	return 1;
}

LRESULT CSettingsDialog::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	BOOL translated;
	UINT val = GetDlgItemInt(IDC_COUNTDOWN, &translated);
	if (translated)
		m_countdown = val;
	val = GetDlgItemInt(IDC_FORCEFADE, &translated);
	if (translated)
		m_fadeout = val;
	EndDialog(wID);
	return 0;
}
