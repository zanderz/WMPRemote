// WMPRemote.cpp : Implementation of WinMain


#include "stdafx.h"
#include "resource.h"
#include "WMPRemote_i.h"
#include "MainDialog.h"

using namespace ATL;


class CWMPRemoteModule : public ATL::CAtlExeModuleT< CWMPRemoteModule >
{
public :
	DECLARE_LIBID(LIBID_WMPRemoteLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_WMPREMOTE, "{7237493E-7182-4589-B4BF-B6C1FB46CD34}")


	HRESULT PreMessageLoop(int nCmdShow)
	{
		HRESULT hr;
		hr = CAtlExeModuleT< CWMPRemoteModule >::PreMessageLoop(nCmdShow);
		if (FAILED(hr))
		{
			return hr;
		}

		CMainDialog dlg;
		dlg.DoModal();

		return S_FALSE;
	}


	};

CWMPRemoteModule _AtlModule;



//
extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, 
								LPTSTR /*lpCmdLine*/, int nShowCmd)
{
	return _AtlModule.WinMain(nShowCmd);
}

