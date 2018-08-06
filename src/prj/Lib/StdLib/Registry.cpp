//
// GammaLib: Computer Vision library
//
//    Copyright (C) 1998-2015 Luca Ballan <ballanlu@gmail.com> http://lucaballan.altervista.org/
//
//    Third party copyrights are property of their respective owners.
//
//
//    The MIT License(MIT)
//
//    Permission is hereby granted, free of charge, to any person obtaining a copy
//    of this software and associated documentation files (the "Software"), to deal
//    in the Software without restriction, including without limitation the rights
//    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//    copies of the Software, and to permit persons to whom the Software is
//    furnished to do so, subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in all
//    copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//    SOFTWARE.
//
//
//
//





#pragma once

#include <windows.h>
#include "shlwapi.h"
#include "Registry.h"


DWORD Registry::GetRegDWORD(CString cstrKeyName, CString cstrValueName,
	DWORD dwDefault, HKEY hBaseKey)
{
	HKEY hKey;
    DWORD dwBufLen = sizeof(DWORD);
	DWORD dwReturn = dwDefault;
	BOOL bError = TRUE;

    if (RegOpenKeyEx(hBaseKey, cstrKeyName, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx(hKey, cstrValueName, NULL, NULL, (LPBYTE) &dwReturn, &dwBufLen) == ERROR_SUCCESS) 
			bError = FALSE;

		RegCloseKey(hKey);
	}

	if (bError) DisplayErrorMessage("GetRegDWORD");

	return dwReturn;
};

// Set a DWORD value in the registry
void Registry::SetRegDWORD(CString cstrKeyName, CString cstrValueName, DWORD dwValue, 
	HKEY hBaseKey)
{
	HKEY hKey;
	LONG lRet;
	BOOL bError = TRUE;

	lRet = RegOpenKeyEx(hBaseKey, cstrKeyName, 0, KEY_SET_VALUE, &hKey);
    if (lRet != ERROR_SUCCESS)
		lRet = RegCreateKeyEx(hBaseKey, cstrKeyName, 0, "", 0, KEY_READ | KEY_WRITE, NULL, &hKey, NULL); 
	
	if (lRet == ERROR_SUCCESS)
	{
		if (RegSetValueEx(hKey, cstrValueName, NULL, REG_DWORD, (LPBYTE) &dwValue, sizeof(DWORD)) == ERROR_SUCCESS)
			bError = FALSE;

		RegCloseKey(hKey);
	}

	if (bError) DisplayErrorMessage("SetRegDWORD");
};

// Get a string value from the registry
CString Registry::GetRegString(CString cstrKeyName, CString cstrValueName, 
	CString cstrDefault = "", HKEY hBaseKey)
{
	HKEY hKey;
    char szValue[MAX_PATH + 1];
	CString cstrReturn = cstrDefault;
	DWORD dwBufLen = MAX_PATH + 1;
	BOOL bError = TRUE;

    if (RegOpenKeyEx(hBaseKey, cstrKeyName, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx(hKey, cstrValueName, NULL, NULL, (LPBYTE) szValue, 
			&dwBufLen) == ERROR_SUCCESS)
		{
			cstrReturn = szValue;
			bError = FALSE;
		}

		RegCloseKey(hKey);
	}

	if (bError) DisplayErrorMessage("GetRegString");

	return cstrReturn;
};

// Set a string value in the registry
void Registry::SetRegString(CString cstrKeyName, CString cstrValueName, CString cstrValue, HKEY hBaseKey)
{
	HKEY hKey;
	LONG lRet;
	BOOL bError = TRUE;

	lRet = RegOpenKeyEx(hBaseKey, cstrKeyName, 0, KEY_SET_VALUE, &hKey);
    if (lRet != ERROR_SUCCESS)
		lRet = RegCreateKeyEx(hBaseKey, cstrKeyName, 0, "", 0, KEY_READ | KEY_WRITE, NULL, &hKey, NULL); 
	
	if (lRet == ERROR_SUCCESS)
	{
		if (RegSetValueEx(hKey, cstrValueName, NULL, REG_SZ, (LPBYTE) cstrValue.GetBuffer(), cstrValue.GetLength()) == ERROR_SUCCESS)
			bError = FALSE;

		RegCloseKey(hKey);
	}

	if (bError) DisplayErrorMessage("SetRegString");
};

// Delete a value or key from the registry
BOOL Registry::RegDelete(CString cstrKeyName, CString cstrValueName, CString cstrSubKeyName, 
	HKEY hBaseKey)
{
	HKEY hKey;
	BOOL bRetVal = FALSE;
	BOOL bError = TRUE;

	if (RegOpenKeyEx(hBaseKey, cstrKeyName, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
	{
		if (!cstrValueName.IsEmpty())
		{
			// delete a value
			if (RegDeleteValue(hKey, cstrValueName) == ERROR_SUCCESS)
			{
				bRetVal = TRUE;
				bError = FALSE;
			}
		}
		else
		{
			if (!cstrSubKeyName.IsEmpty())
			{
				// delete an entire key
				if (SHDeleteKey(hKey, cstrSubKeyName) == ERROR_SUCCESS)
				{
					bRetVal = TRUE;
					bError = FALSE;
				}
			}
		}

		RegCloseKey(hKey);
	}

	if (bError) DisplayErrorMessage("RegDelete");

	return bRetVal;
}

