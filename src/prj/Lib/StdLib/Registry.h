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





#include "shlwapi.h"

class Registry {

public:
	// Get a DWORD value from the registry
	static DWORD GetRegDWORD(CString cstrKeyName, CString cstrValueName, 
		DWORD dwDefault = 0, HKEY hBaseKey = HKEY_CURRENT_USER);
	
	// Set a DWORD value in the registry
	static void SetRegDWORD(CString cstrKeyName, CString cstrValueName, DWORD dwValue, 
		HKEY hBaseKey = HKEY_CURRENT_USER);

	// Get a string value from the registry
	static CString GetRegString(CString cstrKeyName, CString cstrValueName, 
		CString cstrDefault = "", HKEY hBaseKey = HKEY_CURRENT_USER);

	// Set a string value in the registry
	static void SetRegString(CString cstrKeyName, CString cstrValueName, CString cstrValue, HKEY hBaseKey = HKEY_CURRENT_USER);

	// Delete a value or key from the registry
	static BOOL RegDelete(CString cstrKeyName, CString cstrValueName = "", CString cstrSubKeyName = "", 
		HKEY hBaseKey = HKEY_CURRENT_USER);
};
