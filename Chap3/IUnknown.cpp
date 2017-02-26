#include <iostream>
#include <ObjBase.h>

using namespace std;
 
void trace( const char* msg ) { cout << msg << endl;}

// Interface
interface IX: public IUnknown
{
	virtual void __stdcall Fx() = 0;
};

interface IY: public IUnknown
{
	virtual void __stdcall Fy() = 0;
};

interface IZ: public IUnknown
{
	virtual void __stdcall Fz() = 0;
};

// Forward references for GUIDs
extern const IID IID_IX;
extern const IID IID_IY;
extern const IID IID_IZ;


// Component
class CA: public IX, public IY
{
public:
	// IUnknown implementation
	virtual HRESULT __stdcall QueryInterface( const IID& iid, void** ppv );
	virtual ULONG __stdcall AddRef() { return 0; }
	virtual ULONG __stdcall Release() { return 0; }

	// Interface IX implementation
	virtual void __stdcall Fx() { cout << "Fx" << endl; }

	// Interface IY implementation 
	virtual void __stdcall Fy() { cout << "Fy" << endl; }
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
	else if ( IID_IY == iid )
	{
		trace( "QueryInterface: Return pointer to IY");
		*ppv = static_cast<IY*>( this );
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

// Creation function
IUnknown* CreateInstance()
{
	IUnknown* pI = static_cast<IX*>(new CA);
	pI->AddRef();
	return pI;
}


// IIDs
// {2931BFDC-CC73-402a-82A4-980107E2EDA7}
static const IID IID_IX = 
{ 0x2931bfdc, 0xcc73, 0x402a, { 0x82, 0xa4, 0x98, 0x1, 0x7, 0xe2, 0xed, 0xa7 } };

// {47B4F184-BE28-47a9-AA1A-F2BA50B0E6A6}
static const IID IID_IY = 
{ 0x47b4f184, 0xbe28, 0x47a9, { 0xaa, 0x1a, 0xf2, 0xba, 0x50, 0xb0, 0xe6, 0xa6 } };

// {30C0B9EB-40B2-4a2d-8D2F-CBA192011113}
static const IID IID_IZ = 
{ 0x30c0b9eb, 0x40b2, 0x4a2d, { 0x8d, 0x2f, 0xcb, 0xa1, 0x92, 0x1, 0x11, 0x13 } };


// Client
int main()
{
	HRESULT hr;
	trace("Client:Get an IUnknown pointer");
	IUnknown* pIUnknown = CreateInstance();

	trace("Client:Get interface IX");
	IX* pIX = NULL;
	hr = pIUnknown->QueryInterface( IID_IX, (void**)&pIX );
	if ( SUCCEEDED( hr ) )
	{
		trace("Client:Succeeded getting IX");
		pIX->Fx();
	}

	trace("Client:Get interface IY");
	IY* pIY = NULL;
	hr = pIUnknown->QueryInterface( IID_IY, (void**)&pIY );
	if ( SUCCEEDED( hr ) )
	{
		trace("Client:Succeeded getting IY");
		pIY->Fy();
	}

	trace("Client:Ask for an unsupported interface");
	IZ* pIZ = NULL;
	hr = pIUnknown->QueryInterface( IID_IZ, (void**)&pIZ );
	if ( SUCCEEDED( hr ) )
	{
		trace("Client:Succeeded getting IZ");
		pIZ->Fz();
	}
	else
	{
		trace("Client:Could not get interface IZ");
	}

	trace("Client:Get interface IY from interface IX");
	IY* pIYfromIX = NULL;
	hr = pIX->QueryInterface( IID_IY, (void**)&pIYfromIX );
	if ( SUCCEEDED( hr ) )
	{
		trace("Client:Succeeded getting IY");
		pIYfromIX->Fy();
	}

	trace("Client:Get interface IUnknown from IY");
	IUnknown* pIUnknownFromIY = NULL;
	hr = pIY->QueryInterface( IID_IUnknown, (void**)&pIUnknownFromIY );
	if ( SUCCEEDED( hr ) )
	{
		 trace("Are the IUnknown pointers equal?");
		 if ( pIUnknownFromIY == pIUnknown )
		 {
			 trace("equal");
		 }
		 else
		 {
			 trace("not equal");
		 }
	}

	// Delete the component
	delete pIUnknown;

	return 0;
}


