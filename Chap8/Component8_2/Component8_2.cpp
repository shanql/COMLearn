#include <iostream>

#include "../../Common/IFace.h"
#include "../../Common/Utils.h"

using namespace std;

void CMPT2Trace( const char* msg ) { cout << "Component8_2\t\t"; trace(msg); }

//
// Component8_2
//

//////////////////////////////////////////////////////////////////////////
// 
// Global variables
//

static HMODULE g_hModule = NULL;
static long g_cComponents = 0;
static long g_cServerLocks = 0;

const TCHAR g_szFriendlyName[] = _T("Inside COM By Shanql, Chapter 8 Example, Component2");
const TCHAR g_szVerIndProgID[] = _T("InsideComByShanql.Chap8.Component2");
const TCHAR g_szProgID[] = _T("InsideComByShanql.Chap8.Component2.1");


//////////////////////////////////////////////////////////////////////////
// Nondelegating IUnknown interface
//
struct INondelegatingUnknown
{
	virtual HRESULT __stdcall NondelegatingQueryInterface( const IID& iid, void** ppv ) = 0;
	virtual ULONG __stdcall NondelegatingAddRef() = 0;
	virtual ULONG __stdcall NondelegatingRelease() = 0;
};

class CB: public IY, public INondelegatingUnknown
{
public:
	// Delegating IUnknown
	virtual HRESULT __stdcall QueryInterface( const IID& iid, void** ppv )
	{
		CMPT2Trace("CompoDelegate QueryInterface");
		return m_pUnknownOuter->QueryInterface( iid, ppv );
	}
	virtual ULONG __stdcall AddRef()
	{
		CMPT2Trace("Delegate AddRef");
		return m_pUnknownOuter->AddRef();
	}
	virtual ULONG __stdcall Release()
	{
		CMPT2Trace("Delegate Release");
		return m_pUnknownOuter->Release();
	}

	// Nondelegating IUnknown
	virtual HRESULT __stdcall NondelegatingQueryInterface( const IID& iid, void** ppv );
	virtual ULONG __stdcall NondelegatingAddRef();
	virtual ULONG __stdcall NondelegatingRelease();

	// Interface IY
	virtual void __stdcall Fy() { cout << "Fy" << endl; }

	CB( IUnknown* pUnknownOuter );

	~CB();

private:
	long m_cRef;
	IUnknown* m_pUnknownOuter;
};

CB::CB( IUnknown* pUnknownOuter )
:m_cRef( 1 )
{
	InterlockedIncrement( &g_cComponents );
	if ( pUnknownOuter == NULL )
	{
		CMPT2Trace("Not aggregating;delegate to nondelegating IUnknown.");
		m_pUnknownOuter = reinterpret_cast<IUnknown*>( static_cast<INondelegatingUnknown*>(this) );
	}
	else
	{
		CMPT2Trace("Aggregating,delegate to outer IUnknown");
		m_pUnknownOuter = pUnknownOuter;
	}
}

CB::~CB()
{
	InterlockedDecrement( &g_cComponents );
	CMPT2Trace("Destroy self");
}



//
// IUnknown implementation
//
HRESULT __stdcall CB::NondelegatingQueryInterface( const IID& iid, void** ppv )
{
	if ( iid == IID_IUnknown )
	{
		*ppv = static_cast<INondelegatingUnknown*>(this);
	}
	else if ( iid == IID_IY )
	{
		*ppv = static_cast<IY*>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	static_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}

ULONG __stdcall CB::NondelegatingAddRef()
{
	return InterlockedIncrement( &m_cRef );
}

ULONG __stdcall CB::NondelegatingRelease()
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
	~CFactory(){ CMPT2Trace("CFactory destroy"); }

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
	// Aggregate only if the requested iid is IID_IUnknown
	if ( pUnknownOuter != NULL && ( iid != IID_IUnknown ) )
	{
		return CLASS_E_NOAGGREGATION;
	}

	// Create component
	CB* pB = new CB( pUnknownOuter );
	if ( !pB )
	{
		return E_OUTOFMEMORY;
	}


	// Get the requested interface
	hr = pB->NondelegatingQueryInterface( iid, ppv );
	pB->NondelegatingRelease();
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
	if ( clsid != CLSID_Component8_2 )
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
		CLSID_Component8_2, 
		g_szFriendlyName,
		g_szProgID,
		g_szVerIndProgID );
}

STDAPI DllUnregisterServer()
{
	return UnRegisterServer( 
		CLSID_Component8_2, 
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