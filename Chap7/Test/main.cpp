#include <iostream>
#include "..\..\Common\IFace.h"
#include "..\..\Common\Utils.h"



int main()
{
	RegisterServer( 
		GetModuleHandle( NULL ),
 		CLSID_Component1,
 		_T("friend ÓÑºÃÃû×Ö name!£¡£¡"),
 		_T("Shanql.ComponentTest1.1"),
 		_T("Shanql.ComponentTest1"));

	UnRegisterServer(
		CLSID_Component1,
		_T("Shanql.ComponentTest1.1"),
		_T("Shanql.ComponentTest1") );

	return 0;
}