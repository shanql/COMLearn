#pragma once

#include <windows.h>

#include <tchar.h>
#include <strsafe.h>


void trace( const char* msg );

/**
 * @brief 注册com组件
 * @param hModule 注册的模块句柄
 * @param clisid 组件CLSID
 * @param pszFriendlyName 组件的友好名字
 * @param pszProgId 组件的ProgId
 * @param pszVerIndProgID 组件的独立ProgId
*/
BOOL RegisterServer( HMODULE hModule, const CLSID& clsid, LPCTSTR pszFriendlyName,
					LPCTSTR pszProgId, LPCTSTR pszVerIndProgID );

/**
 * @brief 反注册COM组件
 * @param clisid 组件CLSID
 * @param pszProgId 组件的ProgId
 * @param pszVerIndProgID 组件的独立ProgId
*/
BOOL UnRegisterServer( const CLSID& clsid, LPCTSTR pszProgId, LPCTSTR pszVerIndProgID);