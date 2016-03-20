// MainDialog.h : Declaration of the CMainDialog

#pragma once

#include "resource.h"       // main symbols

#include <atlhost.h>
#include "wmp.h"

#define IDT_TIMER1	111
#define IDT_SECOND_TIMER 	112
#define TRANSITION_DELAY_SECS 10
#define INPUT_TICKS_THRESHOLD 3000

using namespace ATL;

// CMainDialog

class CMainDialog :
	public CAxDialogImpl<CMainDialog>,
	public IWMPEvents
{
public:
	CMainDialog() :m_dwAdviseCookie(0), m_dwRef(0), m_transitionTime(0), m_dwLastInputTicks(0)
	{
	}

	~CMainDialog()
	{
		// Stop listening to events
		if (m_spConnectionPoint)
		{
			if (0 != m_dwAdviseCookie)
				m_spConnectionPoint->Unadvise(m_dwAdviseCookie);
			m_spConnectionPoint.Release();
		}

	}

	enum { IDD = IDD_MAINDIALOG };

	HRESULT STDMETHODCALLTYPE QueryInterface(const IID &iid, void **ppunk) throw(){
		if (iid == IID_IWMPEvents){
			*ppunk = (IWMPEvents*) this;
			return S_OK;
		}
		return E_NOTIMPL;
	}
	ULONG STDMETHODCALLTYPE AddRef(void) throw(){ return 1; }
	ULONG STDMETHODCALLTYPE Release(void) throw(){ return 1; }

BEGIN_MSG_MAP(CMainDialog)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_TIMER, OnTimer)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
	CHAIN_MSG_MAP(CAxDialogImpl<CMainDialog>)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	void RefreshPlayList();
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}
	CAxWindow                       *m_pView;
	CComPtr<IWMPPlayer4>            m_spPlayer;             // Player
	CComPtr<IConnectionPoint>   m_spConnectionPoint;
	DWORD                       m_dwAdviseCookie;
	int							m_transitionTime;
	DWORD						m_dwLastInputTicks;


	void STDMETHODCALLTYPE OpenStateChange(
		/* [in] */ long NewState){}

	void STDMETHODCALLTYPE PlayStateChange(
		/* [in] */ long NewState){
		if (NewState == wmppsTransitioning && CheckNoRecentInput()){
			m_transitionTime = TRANSITION_DELAY_SECS;
			PostMessage(WM_TIMER, IDT_SECOND_TIMER);
		}
	}

	void STDMETHODCALLTYPE AudioLanguageChange(
		/* [in] */ long LangID){}

	void STDMETHODCALLTYPE StatusChange(void){}

	void STDMETHODCALLTYPE ScriptCommand(
		/* [in] */ BSTR scType,
		/* [in] */ BSTR Param){}

	void STDMETHODCALLTYPE NewStream(void){}

	void STDMETHODCALLTYPE Disconnect(
		/* [in] */ long Result){}

	void STDMETHODCALLTYPE Buffering(
		/* [in] */ VARIANT_BOOL Start){}

	void STDMETHODCALLTYPE Error(void){}

	void STDMETHODCALLTYPE Warning(
		/* [in] */ long WarningType,
		/* [in] */ long Param,
		/* [in] */ BSTR Description){}

	void STDMETHODCALLTYPE EndOfStream(
		/* [in] */ long Result){}

	void STDMETHODCALLTYPE PositionChange(
		/* [in] */ double oldPosition,
		/* [in] */ double newPosition){}

	void STDMETHODCALLTYPE MarkerHit(
		/* [in] */ long MarkerNum){}

	void STDMETHODCALLTYPE DurationUnitChange(
		/* [in] */ long NewDurationUnit){}

	void STDMETHODCALLTYPE CdromMediaChange(
		/* [in] */ long CdromNum){}

	void STDMETHODCALLTYPE PlaylistChange(
		/* [in] */ IDispatch *Playlist,
		/* [in] */ WMPPlaylistChangeEventType change){
		RefreshPlayList();
	}

	void STDMETHODCALLTYPE CurrentPlaylistChange(
		/* [in] */ WMPPlaylistChangeEventType change){
		RefreshPlayList();
		}

	void STDMETHODCALLTYPE CurrentPlaylistItemAvailable(
		/* [in] */ BSTR bstrItemName){}

	void STDMETHODCALLTYPE MediaChange(
		/* [in] */ IDispatch *Item){
		int i = 0;
	}

	void STDMETHODCALLTYPE CurrentMediaItemAvailable(
		/* [in] */ BSTR bstrItemName){}

	void STDMETHODCALLTYPE CurrentItemChange(
		/* [in] */ IDispatch *pdispMedia){
		RefreshPlayList();
	}

	void STDMETHODCALLTYPE MediaCollectionChange(void){
		int i = 0;
	}

	void STDMETHODCALLTYPE MediaCollectionAttributeStringAdded(
		/* [in] */ BSTR bstrAttribName,
		/* [in] */ BSTR bstrAttribVal){}

	void STDMETHODCALLTYPE MediaCollectionAttributeStringRemoved(
		/* [in] */ BSTR bstrAttribName,
		/* [in] */ BSTR bstrAttribVal){}

	void STDMETHODCALLTYPE MediaCollectionAttributeStringChanged(
		/* [in] */ BSTR bstrAttribName,
		/* [in] */ BSTR bstrOldAttribVal,
		/* [in] */ BSTR bstrNewAttribVal){}

	void STDMETHODCALLTYPE PlaylistCollectionChange(void){}

	void STDMETHODCALLTYPE PlaylistCollectionPlaylistAdded(
		/* [in] */ BSTR bstrPlaylistName){}

	void STDMETHODCALLTYPE PlaylistCollectionPlaylistRemoved(
		/* [in] */ BSTR bstrPlaylistName){}

	void STDMETHODCALLTYPE PlaylistCollectionPlaylistSetAsDeleted(
		/* [in] */ BSTR bstrPlaylistName,
		/* [in] */ VARIANT_BOOL varfIsDeleted){}

	void STDMETHODCALLTYPE ModeChange(
		/* [in] */ BSTR ModeName,
		/* [in] */ VARIANT_BOOL NewValue){}

	void STDMETHODCALLTYPE MediaError(
		/* [in] */ IDispatch *pMediaObject){}

	void STDMETHODCALLTYPE OpenPlaylistSwitch(
		/* [in] */ IDispatch *pItem){}

	void STDMETHODCALLTYPE DomainChange(
		/* [in] */ BSTR strDomain){}

	void STDMETHODCALLTYPE SwitchedToPlayerApplication(void){}

	void STDMETHODCALLTYPE SwitchedToControl(void){}

	void STDMETHODCALLTYPE PlayerDockedStateChange(void){}

	void STDMETHODCALLTYPE PlayerReconnect(void){}

	void STDMETHODCALLTYPE Click(
		/* [in] */ short nButton,
		/* [in] */ short nShiftState,
		/* [in] */ long fX,
		/* [in] */ long fY){
		m_dwLastInputTicks = GetTickCount();
	}

	void STDMETHODCALLTYPE DoubleClick(
		/* [in] */ short nButton,
		/* [in] */ short nShiftState,
		/* [in] */ long fX,
		/* [in] */ long fY){
		m_dwLastInputTicks = GetTickCount();
	}

	void STDMETHODCALLTYPE KeyDown(
		/* [in] */ short nKeyCode,
		/* [in] */ short nShiftState){}

	void STDMETHODCALLTYPE KeyPress(
		/* [in] */ short nKeyAscii){
		m_dwLastInputTicks = GetTickCount();
	}

	void STDMETHODCALLTYPE KeyUp(
		/* [in] */ short nKeyCode,
		/* [in] */ short nShiftState){}

	void STDMETHODCALLTYPE MouseDown(
		/* [in] */ short nButton,
		/* [in] */ short nShiftState,
		/* [in] */ long fX,
		/* [in] */ long fY){
		m_dwLastInputTicks = GetTickCount();
	}

	void STDMETHODCALLTYPE MouseMove(
		/* [in] */ short nButton,
		/* [in] */ short nShiftState,
		/* [in] */ long fX,
		/* [in] */ long fY){}

	void STDMETHODCALLTYPE MouseUp(
		/* [in] */ short nButton,
		/* [in] */ short nShiftState,
		/* [in] */ long fX,
		/* [in] */ long fY){
		m_dwLastInputTicks = GetTickCount();
	}
	
	void SetPlayList();
	void CheckCountdown();

	void AdjustTextSize();
	bool CheckNoRecentInput(){
		return (GetTickCount() - m_dwLastInputTicks) > INPUT_TICKS_THRESHOLD;
	}
protected:
	ULONG m_dwRef;

};


