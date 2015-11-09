// RemoteHost.h : Declaration of the CRemoteHost

#pragma once
#include "resource.h"       // main symbols



#include "WMPRemote_i.h"



#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

#include "wmp.h"

using namespace ATL;


// CRemoteHost

class ATL_NO_VTABLE CRemoteHost :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CRemoteHost, &CLSID_RemoteHost>,
	public IDispatchImpl<IRemoteHost, &IID_IRemoteHost, &LIBID_WMPRemoteLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IServiceProvider,
	public IWMPRemoteMediaServices
{
public:
	CRemoteHost()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_REMOTEHOST)

DECLARE_NOT_AGGREGATABLE(CRemoteHost)

BEGIN_COM_MAP(CRemoteHost)
	COM_INTERFACE_ENTRY(IRemoteHost)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IServiceProvider)
	COM_INTERFACE_ENTRY(IWMPRemoteMediaServices)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:

	// IServiceProvider
	STDMETHOD(QueryService)(REFGUID guidService, REFIID riid, void **ppv)
	{
		return QueryInterface(riid, ppv);
	}
	// IWMPRemoteMediaServices
	STDMETHOD(GetServiceType)(BSTR *pbstrType)
	{
		*pbstrType = ::SysAllocString(L"Remote");
		return S_OK;
	}
	STDMETHOD(GetApplicationName)(BSTR *pbstrName)
	{
		*pbstrName = ::SysAllocString(L"MEA");
		return S_OK;
	}
	STDMETHOD(GetScriptableObject)(BSTR * pbstrName, IDispatch ** ppDispatch)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(GetCustomUIMode)(BSTR * pbstrFile)
	{
		return E_NOTIMPL;
	}

};

OBJECT_ENTRY_AUTO(__uuidof(RemoteHost), CRemoteHost)
