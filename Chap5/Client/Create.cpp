#include <iostream>

#include "Create.h"

using namespace std;

typedef IUnknown* (*CREATEFUNCPTR)();
IUnknown* CallCreateInstance( char* name )
{
	// Load dynamic link library into process
	HINSTANCE hComponent = ::LoadLibraryA( name );
	if ( NULL == hComponent )
	{
		cout << "CallCreateInstance: Error, Cannot load component" << endl;
		return NULL;
	}

	// Get address for CreateInstance function
	CREATEFUNCPTR CreateInstance = (CREATEFUNCPTR)::GetProcAddress( hComponent, "CreateInstance");
	if( NULL == CreateInstance )
	{
		cout << "CallCreateInstance: Error, Cannot find CreateInstance function" << endl;
		return NULL;
	}

	return CreateInstance();
}



