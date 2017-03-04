#include <iostream>
#include "..\..\Common\IFace.h"
#include <tchar.h>
#include <strsafe.h>

using namespace std;

void trace( const char* msg ) { cout << msg << endl;}


/**
 * @brief 在HKEY_CLASSES_ROOT创建键，并设置默认值
 */
BOOL SetKeyAndValue( PTCHAR pszKey, PTCHAR pszSubKey, PTCHAR pszValue )
{
	if ( !pszKey || !pszValue )
	{
		trace("invalid param pszKeyName or pszValue");
		return FALSE;
	}

	TCHAR szKeyBuf[1024] = {0};
	_tcscpy_s( szKeyBuf, _countof(szKeyBuf), pszKey );
	if ( pszSubKey )
	{
		_stprintf_s( szKeyBuf, _countof(szKeyBuf), _T("%s\\%s"), szKeyBuf, pszSubKey );
	}

	HKEY hKey = NULL;
	if ( ERROR_SUCCESS != RegCreateKeyEx( 
		HKEY_CLASSES_ROOT,
		szKeyBuf,
		NULL,
		NULL,
		0,
		KEY_WRITE,
		NULL,
		&hKey, 
		NULL ) )
	{
		trace("create key failed");
		return FALSE;
	}

	if ( ERROR_SUCCESS != RegSetValueEx( 
		hKey,
		_T(""),
		0, 
		REG_SZ,
		reinterpret_cast<const BYTE*>(pszValue),
		sizeof(TCHAR)*(_tcslen(pszValue)+1) ) )
	{
		RegCloseKey( hKey );
		trace("set key default failed");
		return FALSE;
	}

	RegCloseKey( hKey );
	return TRUE;
}

void CLSIDToStr( const CLSID& clsid, PTCHAR pszClsid, int nCountOfElement )
{
	LPOLESTR pClsidStr = NULL;
	StringFromCLSID( clsid, &pClsidStr );
	_tcscpy_s( pszClsid, nCountOfElement, pClsidStr );
	if ( pClsidStr )
	{
		CoTaskMemFree( (LPVOID)pClsidStr );
		pClsidStr = NULL;
	}
}

void RegisterServer( HMODULE hModule, PTCHAR pszFriendlyName,
					PTCHAR pszProgId, PTCHAR pszVerIndProgID )
{
	TCHAR szClsid[48] = {0};
	CLSIDToStr( CLSID_Component1, szClsid, _countof(szClsid) );

	TCHAR szDllFileName[256] = {0};
	if ( 0 == GetModuleFileName( hModule,
		szDllFileName, _countof(szDllFileName) ) )
	{
		trace( "get module file name failed" );
		return;
	}
	
	// add CLSID under the HKEY_CLASSES_ROOT
	TCHAR szClsKey[64] = {0};
	_stprintf_s( szClsKey, _countof(szClsKey), _T("CLSID\\%s"), szClsid );
	if ( !SetKeyAndValue( szClsKey, NULL, pszFriendlyName ) )
	{
		return;
	}
	if ( !SetKeyAndValue( szClsKey, _T("InprocServer32"), szDllFileName ) )
	{
		return;
	}
	if ( !SetKeyAndValue( szClsKey, _T("ProgID"), pszProgId ))
	{
		return;
	}
	// In addition to the preceding registry entry, you should add the following 
	// corresponding entry under the CLSID key:
	// \CLSID
	//	 \<CLSID>  = <human readable name>
	//	 \<VersionIndependentProgID> = human readable name  
	//	note by shanql: is this error? should VersionIndependentProgID = <VersionIndependentProgID>?
	if ( !SetKeyAndValue( szClsKey, _T("VersionIndependentProgID"), pszVerIndProgID ) )
	{
		return;
	}

	
	// add ProgId under the HKEY_CLASSES_ROOT
	// \<ProgID> = <HumanReadableNamen>
	//		\CLSID  Object's CLSID 
	//		\Insertable  Indicates that class is insertable in OLE 2 containers 
	//		\Protocol  Indicates class is insertable in OLE 1 container 
	//		\Shell  Windows 3.1 File Manager information  

	if ( !SetKeyAndValue( pszProgId, NULL, pszFriendlyName ) )
	{
		return;
	}
	if ( !SetKeyAndValue( pszProgId, _T("CLSID"), szClsid ) )
	{
		return;
	}
	



	// add version-independent ProgID under the HKEY_CLASSES_ROOT
	// HKEY_LOCAL_MACHINE\SOFTWARE\Classes
	//	\<VersionIndependentProgID> = <Vendor>.<Component>
	//	\CLSID = <CLSID>
	//	\CurVer = <ProgID>
	if ( !SetKeyAndValue( pszVerIndProgID, NULL, pszFriendlyName ) )
	{
		return;
	}
	if ( !SetKeyAndValue( pszVerIndProgID, _T("CLSID"), szClsid ) )
	{
		return;
	}
	if ( !SetKeyAndValue( pszVerIndProgID, _T("CurVer"), pszProgId ) )
	{
		return;
	}
	
	trace("register server successfully");
}

int main()
{
	RegisterServer( 
		GetModuleHandle( NULL ),
		_T("friend 友好名字 name!！！"), _T("Shanql.ComponentTest1.1"), _T("Shanql.ComponentTest1"));
	return 0;
}