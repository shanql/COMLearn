#include <iostream>
#include <ObjBase.h>

using namespace std;

#include "../Common/IFace.h"

void trace( const char* msg ) { cout << "Component1:\t" << msg << endl;}

class CA: public IX
{
	virtual HRESULT __stdcall QueryInterface( const IID& iid, void** ppv );
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	virtual void __stdcall Fx() { cout << "Fx" << endl; }

public:
	CA():m_cRef(0) {}
	~CA(){ trace("Destroy self"); }

private:
	long m_cRef;
};


HRESULT __stdcall CA::QueryInterface( const IID& iid, void** ppv )
{
	if ( IID_IUnknown == iid )
	{
		trace( "QueryInterface: Return pointer to IUnknown");
		*ppv = static_cast<IX*>( this );
	}
	else if ( IID_IX == iid )
	{
		trace("QueryInterface: Return pointer to IX");
		*ppv = static_cast<IX*>(this);
	}
	else
	{
		trace("QueryInterface: Interface not supported");
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}

ULONG __stdcall CA::AddRef()
{
	cout << "Component1:\tCA: AddRef = " << m_cRef + 1 << endl;
	return InterlockedIncrement( &m_cRef );
}

ULONG __stdcall CA::Release()
{
	cout << "Component1:\tCA: Release = " << m_cRef - 1 << endl;
	if ( InterlockedDecrement( &m_cRef ) == 0 )
	{
		delete this;
		return 0;
	}
	return m_cRef;
}

// Creation function
extern "C" __declspec(dllexport) IUnknown* CreateInstance()
{
	IUnknown* pI = static_cast<IX*>(new CA);
	pI->AddRef();
	return pI;
}
