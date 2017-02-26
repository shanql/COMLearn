#include <iostream>

#include "Create.h"
#include "../Common/IFace.h"

using namespace std;

void trace( const char* msg ){ cout << "Client1:\t" << msg << endl; }

int main()
{
	HRESULT hr;

	// Get the name of the component to use
	char name[40] = { 0 };
	cout << "Enter the filename of a component to use[Component?.dll]:" << endl;
	cin >> name;
	cout << endl;

	// Create component by calling CreateInstance function in the DLL
	trace( "Get an IUnknown pointer.");
	IUnknown* pIUnknown = CallCreateInstance( name );
	if ( NULL == pIUnknown )
	{
		trace("CallCreateInstance Failed");
		return 1;
	}

	trace("Get interface Ix");
	IX* pIX = NULL;
	hr = pIUnknown->QueryInterface( IID_IX, (void**)&pIX );
	if ( SUCCEEDED( hr ) )
	{
		trace("Succeeded getting IX");
		pIX->Fx();
		pIX->Release();
	}
	else
	{
		trace("Could not get interface IX");
	}

	trace("Release IUnknown interface");
	pIUnknown->Release();

	return 0;
}