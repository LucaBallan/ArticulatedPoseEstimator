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





//
// Hidden functions
//



//
// Crittografia
//          * the caller has to delete the buffer with delete[]
//          * if count==-1 -> content is a null terminated string
//          * output always a null terminated string
//          * file must be a text file without \0 character
//
char *crypt_strX(const char *content,int count);
char *crypt_strX_readable(const char *content,int count=-1,char *var_name=NULL);
char *crypt_file_readable(const char *filename,char *var_name=NULL);
char *crypt_file_include_readable(const char *filename,char *var_name=NULL);

//
// Crittografia shaders
// Usage: 	e.g. CryptShaders("D:\\Luca\\Mot64\\data\\shaders"); -> shaders_lib.hpp
//
void CryptShaders(char *directory);


//
// Funzioni di protezione libreria
//
char *getTitle(char *app_title);








