// MainDialog.cpp : Implementation of CMainDialog

#include "stdafx.h"
#include "MainDialog.h"
#include "RemoteHost.h"
#include "SettingsDialog.h"


// {A4879BCD-FCC1-47EF-91FA-B70C94795ABF}
static const GUID IID_IWMPEvents =
{ 0x19A6627B, 0xDA9E, 0x47c1, { 0xBB, 0x23, 0x00, 0xB5, 0xE6, 0x68, 0x23, 0x6A } };

// CMainDialog

LRESULT CMainDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

	CAxDialogImpl<CMainDialog>::OnInitDialog(uMsg, wParam, lParam, bHandled);

	///////////////////////////////////////////////////
	// Initialize WMP control
	//////////////////////////////////////////////////
	CComPtr<IAxWinHostWindow>	spHost;
	CComPtr<IObjectWithSite>    spHostObject;
	CComObject<CRemoteHost>     *pRemoteHost = NULL;
	RECT                        rectWMP = { 0, 0, 0, 0 };
	HRESULT                     hr;
	CComBSTR listText;
	
	HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_DJOG));
	SetIcon(hIcon);

	// Get an simple container to contain WMP OCX
	AtlAxWinInit();
	m_pView = new CAxWindow();
	hr = m_pView ? S_OK : E_OUTOFMEMORY;

	if (SUCCEEDED(hr))
	{
		m_pView->Create(m_hWnd, rectWMP, NULL, WS_CHILD | WS_VISIBLE);
		hr = ::IsWindow(m_pView->m_hWnd) ? S_OK : E_FAIL;
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pView->QueryHost(IID_IObjectWithSite, (void **)&spHostObject);
		hr = spHostObject.p ? hr : E_FAIL;

		if (SUCCEEDED(hr))
		{
			hr = CComObject<CRemoteHost>::CreateInstance(&pRemoteHost);
			if (pRemoteHost)
			{
				pRemoteHost->AddRef();
			}
			else
			{
				hr = E_POINTER;
			}
		}

		if (SUCCEEDED(hr))
		{
			hr = spHostObject->SetSite((IWMPRemoteMediaServices *)pRemoteHost);
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pView->QueryHost(&spHost);
		hr = spHost.p ? hr : E_FAIL;
	}

	// Create WMP control using its CLSID
	if (SUCCEEDED(hr))
	{
		hr = spHost->CreateControl(CComBSTR(L"{6BF52A52-394A-11d3-B153-00C04F79FAA6}"), m_pView->m_hWnd, 0);
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pView->QueryControl(&m_spPlayer);
		hr = m_spPlayer.p ? hr : E_FAIL;
	}

	if (SUCCEEDED(hr))
	{
		// If it is remote OCX, enable Go To Media Library button
//		VARIANT_BOOL	bIsRemote = VARIANT_FALSE;
//		m_spPlayer->get_isRemote(&bIsRemote);
//		::EnableWindow(GetDlgItem(IDC_GOTOPLAYER), bIsRemote == VARIANT_TRUE ? TRUE : FALSE);

		// Now we have the WMP OCX, we get mediaCollection and playlistCollection
		// objects for later use.
		AdjustTextSize();
		RefreshPlayList();

		// Smart pointer to IConnectionPointContainer
		CComPtr<IConnectionPointContainer>  spConnectionContainer;

		hr = m_spPlayer->QueryInterface(&spConnectionContainer);

		// Test whether the control supports the IWMPEvents interface.
		if (SUCCEEDED(hr))
		{
			hr = spConnectionContainer->FindConnectionPoint(__uuidof(IWMPEvents), &m_spConnectionPoint);
		}

		if (SUCCEEDED(hr))
		{
			hr = m_spConnectionPoint->Advise(this, &m_dwAdviseCookie);
		}

//		m_spPlayer->get_playlistCollection(&m_spPC);

		// Use tree view to show media library
//		ShowMainTree();
	}

	// Release remote host object
	if (pRemoteHost)
	{
		pRemoteHost->Release();
	}

	HMENU hMenu = GetSystemMenu(FALSE);
	AppendMenu(hMenu, MF_ENABLED | MF_STRING, IDM_SYSCOMMAND_SETTINGS, L"Settings");
	
	SetTimer(IDT_SECOND_TIMER, 1000, NULL);


	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

// Start the timer on which we reset the strings
void CMainDialog::RefreshPlayList(){
	PostMessage(WM_DRAWPLAYLIST);
}

LRESULT CMainDialog::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){
	if (wParam == IDT_SECOND_TIMER){
		CheckCountdown();
	}
	return 1;
}

void CMainDialog::SetPlayList()
{
	long lCount = 0;
	long lDisplayed = 0;
	CString songList;

	CComPtr<IWMPPlaylist> pPl;
	CComPtr<IWMPCore> pCore;
	CComPtr<IWMPControls> pControl;
	CComPtr<IWMPMedia> pCurrentItem;

	if (SUCCEEDED(m_spPlayer->QueryInterface(&pCore)) &&
		SUCCEEDED(pCore->get_controls(&pControl))) {
			pControl->get_currentItem(&pCurrentItem);
	}
	else {
		return;
	}

	if (SUCCEEDED(m_spPlayer->get_currentPlaylist(&pPl)))
		pPl->get_count(&lCount);

	for (long i = 0; i < lCount && lDisplayed < 3; i++)
	{
		CComPtr<IWMPMedia> pItem;
		if (SUCCEEDED(pPl->get_item(i, &pItem)))
		{
			// If we couldn't get the current item, set it to the first
			if (pCurrentItem == NULL && i == 0)
				pCurrentItem = pItem;

			long lAttributeCount = 0;
			CComBSTR attributeName, attributeVal;

			// Skip the items in the playlist till we get to ours
			if (lDisplayed == 0) {
				VARIANT_BOOL bSame = VARIANT_FALSE;
				pItem->get_isIdentical(pCurrentItem, &bSame);
				if(bSame != VARIANT_TRUE)
					continue;
			}

			if (FAILED(pItem->get_attributeCount(&lAttributeCount)))
				continue;

			if (FAILED(pItem->getItemInfo(CComBSTR(L"WM/Genre"), &attributeVal)) || attributeVal.Length() == 0)
			{
				pItem->getItemInfo(L"Title", &attributeVal);
			}
			if (attributeVal.Length() == 0)
				pItem->get_name(&attributeVal);
			if (attributeVal.Length() == 0)
				attributeVal = L"???";
			songList += attributeVal;
			if (lDisplayed < 2)
				songList += L"\r\n";
			lDisplayed++;
			CString out;
			out.Format(L"SetPlayList: item %d\n", i);
			OutputDebugString(out);
		}
	}
	songList.MakeUpper();
	SetDlgItemText(IDC_EDIT1, songList);
	GetDlgItem(IDC_EDIT1).HideCaret();
}

void CMainDialog::AdjustTextSize() {
	
	RECT r;
	HWND h = GetDlgItem(IDC_EDIT1);
	::GetWindowRect(h, &r); //get window rect of control relative to screen

	LONG HeightInPixels = (r.bottom - r.top) * 3 / 10;

	int HeightInDialogUnits = HeightInPixels;

	HFONT hFont = CreateFont(
		HeightInDialogUnits,	//_In_ int     nHeight,
		0,						//_In_ int     nWidth,
		0,						//_In_ int     nEscapement,
		0,						//_In_ int     nOrientation,
		FW_DONTCARE,			//_In_ int     fnWeight,
		FALSE,					//_In_ DWORD   fdwItalic,
		FALSE,					//_In_ DWORD   fdwUnderline,
		FALSE,					//_In_ DWORD   fdwStrikeOut,
		ANSI_CHARSET,			//_In_ DWORD   fdwCharSet,
		OUT_DEFAULT_PRECIS,		//_In_ DWORD   fdwOutputPrecision,
		CLIP_DEFAULT_PRECIS,	//_In_ DWORD   fdwClipPrecision,
		DEFAULT_QUALITY,		//_In_ DWORD   fdwQuality,
		DEFAULT_PITCH | FF_SWISS,	//_In_ DWORD   fdwPitchAndFamily,
		L"Arial"				//_In_ LPCTSTR lpszFace
		);
	HFONT hOldFont = (HFONT) SendMessage(h, WM_GETFONT, 0, 0);
	SendMessage(h, WM_SETFONT, WPARAM(hFont), TRUE);
	::HideCaret(h);
	DeleteObject(hOldFont);

	h = GetDlgItem(IDC_COUNTDOWN);
	hOldFont = (HFONT)SendMessage(h, WM_GETFONT, 0, 0);
	SendMessage(h, WM_SETFONT, WPARAM(hFont), TRUE);
	::HideCaret(h);
	DeleteObject(hOldFont);

	CString timerText;
	UINT textLen = GetDlgItemText(IDC_COUNTDOWN, timerText.GetBufferSetLength(40), 40);
	GetClientRect(&r);
	int lineHeight = (r.bottom - r.top) / 3;
	r.bottom = r.top + lineHeight;
	// Getting the real width of the countdown text never really worked out,
	// so we just hack it here
	r.left = r.right - (lineHeight * 4) / 3;
	if (textLen > 2)
		r.left -= lineHeight;
	GetDlgItem(IDC_COUNTDOWN).MoveWindow(&r, false);
}

LRESULT CMainDialog::OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){

	if ((HWND)lParam == (HWND)GetDlgItem(IDC_EDIT1))
	{
		SetTextColor((HDC)wParam, RGB(0, 0, 0));
		SetBkColor((HDC)wParam, RGB(255, 255, 255));
		::HideCaret((HWND)lParam);
	}
	else if ((HWND)lParam == (HWND)GetDlgItem(IDC_COUNTDOWN))
	{
		if (m_transitionTime > 0)
			SetTextColor((HDC)wParam, RGB(0, 0, 255));
		else
			SetTextColor((HDC)wParam, RGB(255, 0, 0));
		SetBkColor((HDC)wParam, RGB(255, 255, 255));
		::HideCaret((HWND)lParam);
	}
	return (INT_PTR)((HBRUSH)GetStockObject(WHITE_BRUSH));
}

LRESULT CMainDialog::OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	RECT r;
	GetClientRect(&r);
	GetDlgItem(IDC_EDIT1).MoveWindow(&r, false);
	AdjustTextSize();
	return 1;
}

LRESULT CMainDialog::OnCountdownStart(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CComPtr<IWMPCore> pCore;
	CComPtr<IWMPControls> pControl;

	if (SUCCEEDED(m_spPlayer->QueryInterface(&pCore))) {
		if (SUCCEEDED(pCore->get_controls(&pControl))) {
			pControl->stop();
		}
	}
	// In case we stopped it by cutting off the track
	SetTimer(IDT_SECOND_TIMER, 1000, NULL);
	return 1;
}

LRESULT CMainDialog::OnCountdownEnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CComPtr<IWMPCore> pCore;
	CComPtr<IWMPSettings> pSettings;
	// This is all to reset the volume after
	// the last track in the playlist gets cut off
	if (m_volume > 0 
		SUCCEEDED(m_spPlayer->QueryInterface(&pCore)) &&
		SUCCEEDED(pCore->get_settings(&pSettings))) {
		OutputDebugString(L"Resetting volume\n");
		pSettings->put_volume(m_volume);
		m_volume = 0;
	}
	return 1;
}

LRESULT CMainDialog::OnDrawPlaylist(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	SetPlayList();
	bHandled = TRUE;
	return 1;
}

LRESULT CMainDialog::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (wParam == IDM_SYSCOMMAND_SETTINGS) {
		bHandled = TRUE;
		CSettingsDialog dlg(m_countdownSetting, m_fadeoutSetting);
		dlg.DoModal();
		if (dlg.m_countdown != m_countdownSetting || dlg.m_fadeout != m_fadeoutSetting) {
			m_countdownSetting = dlg.m_countdown;
			m_fadeoutSetting = dlg.m_fadeout;
			// store in registry here
		}
		return 0;
	}
	bHandled = FALSE;
	return 1;
}

void STDMETHODCALLTYPE CMainDialog::PlayStateChange(
	/* [in] */ long NewState) {
	CString out;
	out.Format(L"PlayStateChange %d\n", NewState);
	OutputDebugString(out);
	switch (NewState) {
	case wmppsMediaEnded:
		m_doDelay = TRUE;
		break;
	case wmppsPlaying:
		if (m_doDelay) {
			m_doDelay = FALSE;
			if (m_countdownSetting > 0) {
				PostMessage(WM_COUNTDOWNSTART);
				m_transitionTime = m_countdownSetting;
				//PostMessage(WM_TIMER, IDT_SECOND_TIMER);
			}
		}
		break;
	case wmppsReady:
		// assume the playlist ended
		PostMessage(WM_COUNTDOWNEND);
		break;
	}
}

void STDMETHODCALLTYPE CMainDialog::MediaChange(
	/* [in] */ IDispatch* Item) {
	OutputDebugString(L"MediaChange\n");
}

void CMainDialog::CheckCountdown() {
	CString text;
	CComPtr<IWMPCore> pCore;
	CComPtr<IWMPControls> pControl;
	CComPtr<IWMPSettings> pSettings;
	CComPtr<IWMPMedia> pCurrentItem;

	if (SUCCEEDED(m_spPlayer->QueryInterface(&pCore))){
		if (SUCCEEDED(pCore->get_controls(&pControl)) &&
			SUCCEEDED(pCore->get_settings(&pSettings))){
			if (m_transitionTime == m_countdownSetting && m_countdownSetting > 0){
				// Start of transition delay
				OutputDebugString(L"CheckCountdown: Start of transition delay\n");
			}
			if (m_transitionTime > 0){
				if (m_transitionTime == 1){
					// End of transition delay
					OutputDebugString(L"CheckCountdown: End of transition delay\n");
					if (m_volume > 0)
						pSettings->put_volume(m_volume);
					pControl->play();
				}
				else {
					CString out;
					out.Format(L"CheckCountdown: transitioning %d\n", m_transitionTime);
					OutputDebugString(out);
					text.Format(L"%d", (int)m_transitionTime);

				}
				m_transitionTime--;
			}
			else if (SUCCEEDED(pControl->get_currentItem(&pCurrentItem))) {
				double currentPosition = 0;
				double trackLength = 0;
				if (pControl && pCurrentItem &&
					SUCCEEDED(pControl->get_currentPosition(&currentPosition)) &&
					SUCCEEDED(pCurrentItem->get_duration(&trackLength))) {
					double duration = trackLength;
					if (m_fadeoutSetting > 0 && duration > m_fadeoutSetting)
						duration = m_fadeoutSetting;

					CString minutes;
					int timeLeft = int(duration - currentPosition);
					if (timeLeft >= 60)
						minutes.Format(L"%d:", timeLeft / 60);
					text.Format(L"%s%02d", minutes.GetBuffer(), timeLeft % 60);
					OutputDebugString(CString(L"CheckCountdown: ") + text + L"\n");

					if (m_fadeoutSetting > 0 && timeLeft == FADEOUT_SECS) {
						if (!SUCCEEDED(pSettings->get_volume(&m_volume)))
							m_volume = 0;
					}
					else if (m_fadeoutSetting > 0 && timeLeft < FADEOUT_SECS) {
						if (m_volume > 0) {
							OutputDebugString(L"Lowering volume\n");
							pSettings->put_volume(m_volume * timeLeft / FADEOUT_SECS);
						}
						// Here's where we cut off the end
						if (timeLeft <= 0 && m_fadeoutSetting > 0 && trackLength > m_fadeoutSetting) {
							CString out;
							out.Format(L"Cutting off %d length track\n", int(trackLength));
							OutputDebugString(out);
							KillTimer(IDT_SECOND_TIMER);
							pControl->put_currentPosition(trackLength);
						}
					}
				}
			}
		}
	}
	if (text.GetLength() == 0)
		GetDlgItem(IDC_COUNTDOWN).ShowWindow(SW_HIDE);
	else {
		CString timerText;
		UINT textLen = GetDlgItemText(IDC_COUNTDOWN, timerText.GetBufferSetLength(40), 40);
		SetDlgItemText(IDC_COUNTDOWN, text);
		if (text.GetLength() != textLen)
			AdjustTextSize();
		GetDlgItem(IDC_COUNTDOWN).ShowWindow(SW_SHOW);
	}
}