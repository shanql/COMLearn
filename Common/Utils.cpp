#include <iostream>

#include "Utils.h"

using namespace std;


void trace( const char* msg ) { cout << msg << endl;}


/**
 * @brief 在HKEY_CLASSES_ROOT创建键，并设置默认值
 */
BOOL SetKeyAndValue( LPCTSTR pszKey, LPCTSTR pszSubKey, LPCTSTR pszValue )
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


void CLSIDToStr( const CLSID& clsid, LPTSTR pszClsid, int nCountOfElement )
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

BOOL RegisterServer( HMODULE hModule, const CLSID& clsid, LPCTSTR pszFriendlyName,
					LPCTSTR pszProgId, LPCTSTR pszVerIndProgID )
{
	TCHAR szDllFileName[256] = {0};
	if ( 0 == GetModuleFileName( hModule,
		szDllFileName, _countof(szDllFileName) ) )
	{
		trace( "get module file name failed" );
		return FALSE;
	}
	
	// add CLSID under the HKEY_CLASSES_ROOT
	TCHAR szClsid[48] = {0};
	CLSIDToStr( clsid, szClsid, _countof(szClsid) );
	TCHAR szClsKey[64] = {0};
	_stprintf_s( szClsKey, _countof(szClsKey), _T("CLSID\\%s"), szClsid );
	if ( !SetKeyAndValue( szClsKey, NULL, pszFriendlyName ) )
	{
		return FALSE;
	}
	if ( !SetKeyAndValue( szClsKey, _T("InprocServer32"), szDllFileName ) )
	{
		return FALSE;
	}
	if ( !SetKeyAndValue( szClsKey, _T("ProgID"), pszProgId ))
	{
		return FALSE;
	}
	// In addition to the preceding registry entry, you should add the following 
	// corresponding entry under the CLSID key:
	// \CLSID
	//	 \<CLSID>  = <human readable name>
	//	 \<VersionIndependentProgID> = human readable name  
	//	note by shanql: is this error? should VersionIndependentProgID = <VersionIndependentProgID>?
	if ( !SetKeyAndValue( szClsKey, _T("VersionIndependentProgID"), pszVerIndProgID ) )
	{
		return FALSE;
	}

	
	// add ProgId under the HKEY_CLASSES_ROOT
	// \<ProgID> = <HumanReadableNamen>
	//		\CLSID  Object's CLSID 
	//		\Insertable  Indicates that class is insertable in OLE 2 containers 
	//		\Protocol  Indicates class is insertable in OLE 1 container 
	//		\Shell  Windows 3.1 File Manager information  

	if ( !SetKeyAndValue( pszProgId, NULL, pszFriendlyName ) )
	{
		return FALSE;
	}
	if ( !SetKeyAndValue( pszProgId, _T("CLSID"), szClsid ) )
	{
		return FALSE;
	}
	



	// add version-independent ProgID under the HKEY_CLASSES_ROOT
	// HKEY_LOCAL_MACHINE\SOFTWARE\Classes
	//	\<VersionIndependentProgID> = <Vendor>.<Component>
	//	\CLSID = <CLSID>
	//	\CurVer = <ProgID>
	if ( !SetKeyAndValue( pszVerIndProgID, NULL, pszFriendlyName ) )
	{
		return FALSE;
	}
	if ( !SetKeyAndValue( pszVerIndProgID, _T("CLSID"), szClsid ) )
	{
		return FALSE;
	}
	if ( !SetKeyAndValue( pszVerIndProgID, _T("CurVer"), pszProgId ) )
	{
		return FALSE;
	}
	
	trace("register server successfully");
	return TRUE;
}

BOOL RecursiveDeleteKey( HKEY hParenKey, LPCTSTR pszSubKey )
{
	if ( !pszSubKey )
	{
		return FALSE;
	}

	HKEY hChildKey = NULL;
	if ( ERROR_SUCCESS != RegOpenKeyEx( hParenKey, pszSubKey, 0, KEY_ALL_ACCESS, &hChildKey ) )
	{
		return FALSE;
	}


	TCHAR szChildBuf[256] = {0};
	DWORD dwSize = _countof(szChildBuf);
	FILETIME time;
	while( ERROR_SUCCESS == RegEnumKeyEx( hChildKey, 
		0, szChildBuf, &dwSize, NULL, NULL, NULL, &time) )//第二个参数dwIndex每次传0是因为删除了键
	{
		if ( !RecursiveDeleteKey( hChildKey, szChildBuf ) )
		{
			return FALSE;
		}
		dwSize = _countof(szChildBuf);
	}

	RegCloseKey( hChildKey );
	return (BOOL)(ERROR_SUCCESS == RegDeleteKey( hParenKey, pszSubKey ));
}

BOOL UnRegisterServer( const CLSID& clsid, LPCTSTR pszProgId, LPCTSTR pszVerIndProgID)
{
	TCHAR szClsid[48] = {0};
	CLSIDToStr( clsid, szClsid, _countof(szClsid) );
	TCHAR szClsKey[64] = {0};
	_stprintf_s( szClsKey, _countof(szClsKey), _T("CLSID\\%s"), szClsid );

	// delete CLISID\<CLISID>
	if ( !RecursiveDeleteKey( HKEY_CLASSES_ROOT, szClsKey ) )
	{
		return FALSE;
	}

	// delete ProgId under the HKEY_CLASSES_ROOT
	if ( !RecursiveDeleteKey( HKEY_CLASSES_ROOT, pszProgId ) )
	{
		return FALSE;
	}

	// delete version-independent ProgID under the HKEY_CLASSES_ROOT
	if ( !RecursiveDeleteKey( HKEY_CLASSES_ROOT, pszVerIndProgID ) )
	{
		return FALSE;
	}

	trace("unregister server successfully");
	return TRUE;
}
