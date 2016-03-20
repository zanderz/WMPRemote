// MainDialog.cpp : Implementation of CMainDialog

#include "stdafx.h"
#include "MainDialog.h"
#include "RemoteHost.h"


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

	SetTimer(IDT_SECOND_TIMER, 1000, NULL);


	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

// Start the timer on which we reset the strings
void CMainDialog::RefreshPlayList(){
	SetTimer(IDT_TIMER1, 500, NULL);
}

LRESULT CMainDialog::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){
	if (wParam == IDT_TIMER1){
		SetPlayList();
		KillTimer(IDT_TIMER1);
		return 0;
	}
	else if (wParam == IDT_SECOND_TIMER){
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

	if (SUCCEEDED(m_spPlayer->QueryInterface(&pCore))){
		if (SUCCEEDED(pCore->get_controls(&pControl)))
			pControl->get_currentItem(&pCurrentItem);
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
			//for (int j = 0; j < lAttributeCount; j++)
			//{
			//	if (FAILED(pItem->getAttributeName(j, &attributeName)))
			//		continue;
			//	pItem->getItemInfo(attributeName, &attributeVal);
			//	int i = 0;
			//}
			//attributeVal.Empty();
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
	int lineHeight = (r.bottom - r.top) / 3;
	r.bottom = r.top + lineHeight;
	r.left = r.right - (lineHeight * 4) / 3;
	GetDlgItem(IDC_COUNTDOWN).MoveWindow(&r, false);
	AdjustTextSize();
	return 1;
}

void CMainDialog::CheckCountdown() {
	CString text;
	CComPtr<IWMPCore> pCore;
	CComPtr<IWMPControls> pControl;
	CComPtr<IWMPMedia> pCurrentItem;

	if (SUCCEEDED(m_spPlayer->QueryInterface(&pCore))){
		if (SUCCEEDED(pCore->get_controls(&pControl))){
			if (m_transitionTime == TRANSITION_DELAY_SECS){
				// Start of transition delay
				pControl->stop();
			}
			if (m_transitionTime > 0){
				if (m_transitionTime == 1){
					// End of transition delay
					pControl->play();
				}
				else {
					text.Format(L"%d", (int)m_transitionTime);
				}
				m_transitionTime--;
			}
			else if (SUCCEEDED(pControl->get_currentItem(&pCurrentItem))) {
				double currentPosition = 0;
				double duration = 0;
				double timeLeft = 0;
				if (pControl && pCurrentItem &&
					SUCCEEDED(pControl->get_currentPosition(&currentPosition)) &&
					SUCCEEDED(pCurrentItem->get_duration(&duration))) {
					timeLeft = duration - currentPosition;
					if (timeLeft <= 50)
						text.Format(L"%d", (int)timeLeft + TRANSITION_DELAY_SECS);
				}
			}
		}
	}
	if (text.GetLength() == 0)
		GetDlgItem(IDC_COUNTDOWN).ShowWindow(SW_HIDE);
	else {
		SetDlgItemText(IDC_COUNTDOWN, text);
		GetDlgItem(IDC_COUNTDOWN).ShowWindow(SW_SHOW);
	}
}