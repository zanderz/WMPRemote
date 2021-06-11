// MainDialog.h : Declaration of the CMainDialog

#pragma once

#include "resource.h"       // main symbols

#include <atlhost.h>
#include "wmp.h"

#define IDT_TIMER1	111
#define IDT_SECOND_TIMER 	112
#define TRANSITION_DELAY_SECS 10
#define FADEOUT_SECS 5				// cut off track by lowering the volume over this period
#define FORCED_TRUNCATE 180

#define WM_COUNTDOWNSTART	WM_USER+100
#define WM_COUNTDOWNEND		WM_USER+101

#define IDM_SYSCOMMAND_SETTINGS		0xE000

using namespace ATL;

// CMainDialog

class CMainDialog :
	public CAxDialogImpl<CMainDialog>,
	public IWMPEvents
{
public:
	CMainDialog()
		:m_dwAdviseCookie(0)
		, m_dwRef(0)
		, m_transitionTime(0)
		, m_volume(0)
		, m_countdownSetting(TRANSITION_DELAY_SECS)
		, m_fadeoutSetting(FORCED_TRUNCATE)
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
	MESSAGE_HANDLER(WM_COUNTDOWNSTART, OnCountdownStart)
	MESSAGE_HANDLER(WM_COUNTDOWNEND, OnCountdownEnd)
	MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
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
	LRESULT OnCountdownStart(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCountdownEnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
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
	BOOL						m_doDelay = FALSE;
	long						m_volume;
	DWORD						m_countdownSetting;
	DWORD						m_fadeoutSetting;


	void STDMETHODCALLTYPE OpenStateChange(
		/* [in] */ long NewState){
		OutputDebugString(L"OpenStateChange\n");
	}

	void STDMETHODCALLTYPE PlayStateChange(/* [in] */ long NewState);

	void STDMETHODCALLTYPE AudioLanguageChange(
		/* [in] */ long LangID){}

	void STDMETHODCALLTYPE StatusChange(void){
		OutputDebugString(L"StatusChange\n");
	}

	void STDMETHODCALLTYPE ScriptCommand(
		/* [in] */ BSTR scType,
		/* [in] */ BSTR Param){}

	void STDMETHODCALLTYPE NewStream(void){
		OutputDebugString(L"NewStream\n");
	}

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
		/* [in] */ double newPosition){
		OutputDebugString(L"PositionChange\n");
	}

	void STDMETHODCALLTYPE MarkerHit(
		/* [in] */ long MarkerNum){}

	void STDMETHODCALLTYPE DurationUnitChange(
		/* [in] */ long NewDurationUnit){}

	void STDMETHODCALLTYPE CdromMediaChange(
		/* [in] */ long CdromNum){}

	void STDMETHODCALLTYPE PlaylistChange(
		/* [in] */ IDispatch *Playlist,
		/* [in] */ WMPPlaylistChangeEventType change){
		CString out;
		out.Format(L"PlaylistChange %d\n", change);
		OutputDebugString(out);
		RefreshPlayList();
	}

	void STDMETHODCALLTYPE CurrentPlaylistChange(
		/* [in] */ WMPPlaylistChangeEventType change){
		CString out;
		out.Format(L"PlaylistChange %d\n", change);
		OutputDebugString(out);
		RefreshPlayList();
		}

	void STDMETHODCALLTYPE CurrentPlaylistItemAvailable(
		/* [in] */ BSTR bstrItemName){
		OutputDebugString(L"CurrentPlaylistItemAvailable\n");
	}

	void STDMETHODCALLTYPE MediaChange(/* [in] */ IDispatch* Item);

	void STDMETHODCALLTYPE CurrentMediaItemAvailable(
		/* [in] */ BSTR bstrItemName){
		OutputDebugString(L"CurrentMediaItemAvailable\n");
	}

	void STDMETHODCALLTYPE CurrentItemChange(
		/* [in] */ IDispatch *pdispMedia){
		OutputDebugString(L"CurrentItemChange\n");
		RefreshPlayList();
	}

	void STDMETHODCALLTYPE MediaCollectionChange(void){
		OutputDebugString(L"MediaCollectionChange\n");
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
		/* [in] */ VARIANT_BOOL NewValue){
		OutputDebugString(L"ModeChange\n");
	}

	void STDMETHODCALLTYPE MediaError(
		/* [in] */ IDispatch *pMediaObject){}

	void STDMETHODCALLTYPE OpenPlaylistSwitch(
		/* [in] */ IDispatch *pItem){
		OutputDebugString(L"OpenPlaylistSwitch\n");
	}

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
		/* [in] */ long fY){}

	void STDMETHODCALLTYPE DoubleClick(
		/* [in] */ short nButton,
		/* [in] */ short nShiftState,
		/* [in] */ long fX,
		/* [in] */ long fY){}

	void STDMETHODCALLTYPE KeyDown(
		/* [in] */ short nKeyCode,
		/* [in] */ short nShiftState){}

	void STDMETHODCALLTYPE KeyPress(
		/* [in] */ short nKeyAscii){}

	void STDMETHODCALLTYPE KeyUp(
		/* [in] */ short nKeyCode,
		/* [in] */ short nShiftState){}

	void STDMETHODCALLTYPE MouseDown(
		/* [in] */ short nButton,
		/* [in] */ short nShiftState,
		/* [in] */ long fX,
		/* [in] */ long fY){}

	void STDMETHODCALLTYPE MouseMove(
		/* [in] */ short nButton,
		/* [in] */ short nShiftState,
		/* [in] */ long fX,
		/* [in] */ long fY){}

	void STDMETHODCALLTYPE MouseUp(
		/* [in] */ short nButton,
		/* [in] */ short nShiftState,
		/* [in] */ long fX,
		/* [in] */ long fY){}
	
	void SetPlayList();
	void CheckCountdown();

	void AdjustTextSize();
protected:
	ULONG m_dwRef;

};


