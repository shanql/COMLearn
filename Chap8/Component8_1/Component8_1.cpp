#include <iostream>

#include "../../Common/IFace.h"
#include "../../Common/Utils.h"

using namespace std;


void CMPT1Trace( const char* msg ) { cout << "Component8_1\t\t"; trace(msg); }

//
// Component8_1
//

//////////////////////////////////////////////////////////////////////////
// 
// Global variables
//

static HMODULE g_hModule = NULL;
static long g_cComponents = 0;
static long g_cServerLocks = 0;

const TCHAR g_szFriendlyName[] = _T("Inside COM By Shanql, Chapter 8 Example, Component1");
const TCHAR g_szVerIndProgID[] = _T("InsideComByShanql.Chap8.Component1");
const TCHAR g_szProgID[] = _T("InsideComByShanql.Chap8.Component1.1");


//////////////////////////////////////////////////////////////////////////

class CA: public IX
{
public:
	// IUnknown
	virtual HRESULT __stdcall QueryInterface( const IID& iid, void** ppv );
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	// Interface IX
	virtual void __stdcall Fx() { cout << "Fx" << endl; }

	CA();

	~CA();

	// Initialization function called by the class factory
	HRESULT __stdcall Init();

private:
	long m_cRef;

	// Pointer to the aggregated component's IY interface
	// (We do not have to retain an IY pointer, However, we
	// can use it in QueryInterface)
	IY* m_pIY;

	// Pointer to inner component's IUnknown
	IUnknown* m_pUnknownInner;
};

CA::CA()
:m_cRef( 1 ), m_pUnknownInner( NULL ), m_pIY( NULL )
{
	InterlockedIncrement( &g_cComponents );
}

CA::~CA()
{
	InterlockedDecrement( &g_cComponents );
	CMPT1Trace("CA::Destroy self");

	// Prevent recursive destruction on next AddRef/Release pair
	m_cRef = 1;

	// Counter the pUnknownOuter->Release in the Init method
	IUnknown* pUnknownOuter = this;
	pUnknownOuter->AddRef();

	// Properly release the pointer; there may be per-interface reference counts
	//m_pIY->Release();
	if ( NULL != m_pIY )
	{
		m_pIY->Release();
	}

	// Release contained component
	if ( m_pUnknownInner )
	{
		m_pUnknownInner->Release();
	}
}

// Initialize the Component by creating the contained component
HRESULT __stdcall CA::Init()
{
	// Get the pointer to the outer unknown
	// Since this component is not aggregated, the outer unknown
	// is the same as the this pointer
	IUnknown* pUnknownOuter = this;
	CMPT1Trace("Create inner component");
	HRESULT hr = CoCreateInstance( CLSID_Component8_2,
		pUnknownOuter, //Outer component's IUnknown
		CLSCTX_INPROC_SERVER,
		IID_IUnknown, // IUnknown when aggregating
		(void**)&m_pUnknownInner );
	if ( FAILED(hr) )
	{
		CMPT1Trace( "Could not create contained component");
		return E_FAIL;
	}

	// This call will increment the reference count on the outer component
	CMPT1Trace( "Get the IY interface from the inner component");
	hr = m_pUnknownInner->QueryInterface( IID_IY, (void**)&m_pIY );
	if ( FAILED(hr) )
	{
		CMPT1Trace("Inner component does not support interface IY");
		m_pUnknownInner->Release();
		m_pUnknownInner = NULL; //important assign, crash if not
		return E_FAIL;
	}

	// We need to release the reference count added to the 
	// outer component in the above call. So call Release
	// on the pointer you passed to CoCreateInstance
	pUnknownOuter->Release();

	return S_OK;
}

//
// IUnknown implementation
//
HRESULT __stdcall CA::QueryInterface( const IID& iid, void** ppv )
{
	if ( iid == IID_IUnknown )
	{
		*ppv = static_cast<IUnknown*>(this);
	}
	else if ( iid == IID_IX )
	{
		*ppv = static_cast<IX*>(this);
	}
	else if ( iid == IID_IY )
	{
		CMPT1Trace("Return inner component's IY interface");
#if 1
		// You can query for the interface
		return m_pUnknownInner->QueryInterface( iid, ppv );
#else	
		// Or you can return a cached pointer
		*ppv = m_pIY;
#endif
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	static_cast<IUnknown*>(*ppv)->AddRef();
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
// class Factory

class CFactory: public IClassFactory
{
public:
	// IUnknown
	virtual HRESULT __stdcall QueryInterface( const IID& iid, void** ppv );
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	// Interface IClassFactory
	virtual HRESULT __stdcall CreateInstance(IUnknown *pUnkOuter, const IID& riid, void **ppv );
	virtual HRESULT __stdcall LockServer( BOOL bLock );

	CFactory():m_cRef(1){}
	~CFactory(){ CMPT1Trace("CFactory destroy"); }

private:
	long m_cRef;
};

HRESULT __stdcall CFactory::QueryInterface( const IID& iid, void** ppv )
{
	IUnknown* pI = NULL;
	if ( (iid == IID_IUnknown ) || ( iid == IID_IClassFactory ) )
	{
		pI = static_cast<IClassFactory*>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	pI->AddRef();
	*ppv = pI;
	return S_OK;
}

ULONG __stdcall CFactory::AddRef()
{
	return InterlockedIncrement( &m_cRef );
}

ULONG __stdcall CFactory::Release()
{
	if ( 0 == InterlockedDecrement( &m_cRef ) )
	{
		delete this;
		return 0;
	}
	return m_cRef;
}

HRESULT __stdcall CFactory::CreateInstance( IUnknown* pUnknownOuter, const IID& iid, void** ppv )
{
	HRESULT hr = E_FAIL;
	// Cannot aggregate
	if ( pUnknownOuter != NULL )
	{
		return CLASS_E_NOAGGREGATION;
	}

	// Create component
	CA* pA = new CA;
	if ( !pA )
	{
		return E_OUTOFMEMORY;
	}

	// Initialize the component
	hr = pA->Init();
	if ( FAILED(hr) )
	{
		// Initialization failed, delete component
		pA->Release();
		return hr;
	}

	// Get the requested interface
	hr = pA->QueryInterface( iid, ppv );
	pA->Release();
	return hr;
}

HRESULT __stdcall CFactory::LockServer( BOOL bLock )
{
	if ( bLock  )
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
// Exported functions
//

STDAPI DllCanUnloadNow()
{
	if ( (g_cComponents == 0 ) && ( g_cServerLocks == 0 ) )
	{
		return S_OK;
	}
	else
	{
		return S_FALSE;
	}
}

// 
// Get class factory
//
STDAPI DllGetClassObject( const CLSID& clsid, const IID& iid, void** ppv )
{
	// Can we create this component?
	if ( clsid != CLSID_Component8_1 )
	{
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	// Create class factory
	CFactory* pFactory = new CFactory;
	if ( !pFactory )
	{
		return E_OUTOFMEMORY;
	}

	// Get requested interface
	HRESULT hr = pFactory->QueryInterface( iid, ppv );
	pFactory->Release();
	return hr;
}

//
// Server registration
//
STDAPI DllRegisterServer()
{
	return RegisterServer( 
		g_hModule, 
		CLSID_Component8_1, 
		g_szFriendlyName,
		g_szProgID,
		g_szVerIndProgID );
}

STDAPI DllUnregisterServer()
{
	return UnRegisterServer( 
		CLSID_Component8_1, 
		g_szProgID, 
		g_szVerIndProgID );
}


// 
// Dll module information
//
BOOL APIENTRY DllMain(HANDLE _HDllHandle, DWORD _Reason, LPVOID _Reserved)
{
	if ( _Reason == DLL_PROCESS_ATTACH )
	{
		g_hModule = (HMODULE)_HDllHandle;
	}
	return TRUE;
}