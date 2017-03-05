#pragma once

#include <windows.h>

#include <tchar.h>
#include <strsafe.h>


void trace( const char* msg );

/**
 * @brief ע��com���
 * @param hModule ע���ģ����
 * @param clisid ���CLSID
 * @param pszFriendlyName ������Ѻ�����
 * @param pszProgId �����ProgId
 * @param pszVerIndProgID ����Ķ���ProgId
*/
BOOL RegisterServer( HMODULE hModule, const CLSID& clsid, LPCTSTR pszFriendlyName,
					LPCTSTR pszProgId, LPCTSTR pszVerIndProgID );

/**
 * @brief ��ע��COM���
 * @param clisid ���CLSID
 * @param pszProgId �����ProgId
 * @param pszVerIndProgID ����Ķ���ProgId
*/
BOOL UnRegisterServer( const CLSID& clsid, LPCTSTR pszProgId, LPCTSTR pszVerIndProgID);