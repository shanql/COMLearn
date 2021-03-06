#include <iostream>
#include "../../Common/Utils.h"
#include "../../Common/IFace.h"

using namespace std;

//void trace( const char* msg ) { cout << msg << endl;}


//////////////////////////////////////////////////////////////////////////
// 
// Global variables
//
static HMODULE g_hModule = NULL;		//DLL module handles
static long g_cComponents = 0;			//Count of active components
static long g_cServerLocks = 0;			//Count of locks

// Friendly name of component
const TCHAR g_szFriendlyName[] = _T("Inside COM, Chapter 7 Example");

// Version-independent ProgID
const TCHAR g_szVerIndProgID[] = _T("InsideCOM.Chap07");

// ProgID
const TCHAR g_szProgID[] = _T("InsideCOM.Chap07.1");


//////////////////////////////////////////////////////////////////////////
//
// Component
//
class CA: public IX, public IY
{
public:
	// IUnknown
	virtual HRESULT __stdcall QueryInterface( const IID& iid, void** ppv );
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	// Interface IX
	virtual void __stdcall Fx() { cout << "Fx" << endl; }
	
	// Interface IY
	virtual void __stdcall Fy() { cout << "Fy" << endl; }

	// Constructor
	CA();

	// Destructor
	~CA();

private:
	long m_cRef;
};

CA::CA():m_cRef(1)
{
	InterlockedIncrement( &g_cComponents );
}

CA::~CA()
{
	InterlockedDecrement( &g_cComponents );
	trace("Component1:\t\tDestroy self");
}

HRESULT __stdcall CA::QueryInterface( const IID& iid, void** ppv )
{
	if ( iid == IID_IUnknown )
	{
		*ppv = static_cast<IX*>(this);
	}
	else if ( iid == IID_IX )
	{
		*ppv = static_cast<IX*>(this);
		trace("Component1:\t\tReturn pointer to IX");
	}
	else if ( iid == IID_IX )
	{
		*ppv = static_cast<IY*>(this);
		trace("Component1:\t\tReturn pointer to IY");
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	(static_cast<IUnknown*>(*ppv))->AddRef();
	return S_OK;
}

ULONG __stdcall CA::AddRef()
{
	return InterlockedIncrement( &m_cRef );
}

ULONG __stdcall CA::Release()
{
	if ( InterlockedDecrement( &m_cRef ) == 0 )
	{
		delete this;
		return 0;
	}
	return m_cRef;
}


//////////////////////////////////////////////////////////////////////////
//
// Class factory
//
class CFactory: public IClassFactory
{
public:
	// IUnknown
	virtual HRESULT __stdcall QueryInterface( const IID& iid, void** ppv );
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	// Interface IClassFactory
	virtual HRESULT __stdcall CreateInstance(IUnknown *pUnkOuter, const IID& riid, void **ppv );
	virtual HRESULT __stdcall LockServer(BOOL fLock );

	CFactory():m_cRef(1){}
	~CFactory();

private:
	long m_cRef;
};

CFactory::~CFactory()
{
	trace("Class factory:\t\tDestroy self");
}

HRESULT __stdcall CFactory::QueryInterface( const IID& iid, void** ppv )
{
	if ( ( iid == IID_IUnknown ) || ( iid == IID_IClassFactory ) )
	{
		*ppv = static_cast<IClassFactory*>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	static_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}

ULONG __stdcall CFactory::AddRef()
{
	return InterlockedIncrement( &m_cRef );
}

ULONG __stdcall CFactory::Release()
{
	if ( InterlockedDecrement( &m_cRef ) == 0 )
	{
		delete this;
		return 0;
	}
	return m_cRef;
}

HRESULT __stdcall CFactory::CreateInstance(IUnknown *pUnkOuter, const IID& riid, void **ppv )
{
	trace("Class factory:\t\tCreate component");
	//Cannot aggregate
	if ( pUnkOuter != NULL )
	{
		return CLASS_E_NOAGGREGATION;
	}

	// Create component
	CA* pA = new CA();
	if ( !pA )
	{
		return E_OUTOFMEMORY;
	}

	// Get the requested interface
	HRESULT hr = pA->QueryInterface( riid, ppv );
	
	// Release the IUnknown pointer
	// (If QueryInterface failed, component will delete itself)
	pA->Release();
	return hr;
}

HRESULT __stdcall CFactory::LockServer( BOOL bLock )
{
	if ( bLock )
	{
		InterlockedIncrement( &g_cServerLocks );
	}
	else
	{
		InterlockedDecrement( &g_cServerLocks );
	}
	return S_OK;
}



//////////////////////////////////////////////////////////////////////////
// exported functions
//

STDAPI DllCanUnloadNow(void)
{
	if ( ( g_cServerLocks == 0 ) && ( g_cComponents == 0 ) )
	{
		return S_OK;
	}
	else
	{
		return S_FALSE;
	}
}

STDAPI  DllGetClassObject(
	const CLSID& clsid, const IID& iid, void** ppv)
{
	trace("DllGetClassObject:\tCreate class factory");
	// Can we create this component
	if ( clsid != CLSID_Component1 )
	{
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	// Create class factory
	CFactory* pFactory = new CFactory();
	if ( NULL == pFactory )
	{
		return E_OUTOFMEMORY;
	}

	// Get requested interface
	HRESULT hr = pFactory->QueryInterface( iid, ppv );
	pFactory->Release();

	return hr;
}

STDAPI DllRegisterServer(void)
{
	if ( !RegisterServer( g_hModule, CLSID_Component1,
		g_szFriendlyName, g_szProgID, g_szVerIndProgID ))
	{
		trace("DllRegisterServer failed");
		return S_FALSE;
	}

	trace("DllRegisterServer successfully");
	return S_OK;
}

STDAPI DllUnregisterServer(void)
{
	if ( !UnRegisterServer( CLSID_Component1, g_szProgID, g_szVerIndProgID ))
	{
		trace("DllUnregisterServer failed");
		return S_FALSE;
	}
	trace("DllUnregisterServer successfully");
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
//
// Dll module information
//
BOOL APIENTRY DllMain( HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved )
{
	if ( dwReason == DLL_PROCESS_ATTACH )
	{
		g_hModule = (HMODULE)hDllHandle;
	}
	return TRUE;
}