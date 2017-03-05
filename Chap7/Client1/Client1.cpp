#include <iostream>

#define _WIN32_WINNT 0x0400

#include "..\..\Common\IFace.h"
#include "..\..\Common\Utils.h"



using namespace std;


int main()
{
	HRESULT hr;
	hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
	if ( FAILED( hr ) )
	{
		trace("initialize COM library failed");
		return -1;
	}

	IX* pX = NULL;
	hr = CoCreateInstance( CLSID_Component1, NULL, CLSCTX_INPROC_SERVER, IID_IX, (LPVOID*)&pX );
	if ( SUCCEEDED(hr) )
	{
		pX->Fx();
		pX->Release();

		CoUninitialize();
		return 0;
	}
	else
	{
		trace("could not get IX interface");
		return -1;
	}
}