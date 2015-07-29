/*
MIT License

Copyright (c) <2015> <David Reguera Garcia aka Dreg - dreg@fr33project.org>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in the
Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all copies
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifdef GETPROCLIB_EXPORTS // when building DLL, target project defines this macro
#define GETPROCLIB_API __declspec(dllexport)
#elif defined(GETPROCLIB_IMPORTS) // when using DLL, client project defines this macro
#define GETPROCLIB_API __declspec(dllimport)
#else // when building or using target static library, or whatever: define it as nothing
#define GETPROCLIB_API
#endif 

#ifndef _GET_PROC_LIB_H__
#define _GET_PROC_LIB_H__

#include <stdlib.h>
#include <windows.h>
#include "prepos.h"

#define CONCAT(x, y) x ## y
#define CONCAT2(x, y) CONCAT(x, y)

#define FUNC_NAME(name) CONCAT2(START_FUNC, CONCAT2(name, END_FUNC))
#define FUNC_TYPEF(name) CONCAT2(START_TYPDEF_FUNC, CONCAT2(name, END_TYPDEF_FUNC))

#define FUNC_DEC_PRE_DEF(returnf, typef, name, pre_def, ...) \
	typedef returnf (typef * FUNC_TYPEF(name)) ( __VA_ARGS__); \
	pre_def FUNC_TYPEF(name) FUNC_NAME(name);

#define FUNC_DEC(returnf, typef, name, ...) \
	typedef returnf (typef * FUNC_TYPEF(name)) ( __VA_ARGS__); \
	FUNC_TYPEF(name) FUNC_NAME(name);

#define SET_PTR_FUNC_VALUE(name, value) FUNC_NAME(name) = (FUNC_TYPEF(name)) value;

#define FUNC_DEC_VALUE(returnf, typef, name, value, ...) \
	typedef returnf (typef * FUNC_TYPEF(name)) ( __VA_ARGS__); \
	FUNC_TYPEF(name) SET_PTR_FUNC_VALUE(name, value);

#define FUNC_DEC_VALUE_PRE_DEF(returnf, typef, name, pre_def, value, ...) \
	typedef returnf (typef * FUNC_TYPEF(name)) ( __VA_ARGS__); \
	pre_def FUNC_TYPEF(name) SET_PTR_FUNC_VALUE(name, value);

#define API_TAB_ENTRY_END {NULL, NULL, 0, NULL, 0, true}

typedef struct
{
	void * api_addr;
	char * api_name;
	bool need_resolv;
	void ** func_ptr;
	bool optional;
	bool end;
} API_TAB_t;

#define LIB_API_TAB_CREATE_t(type_lib_name, endt) typedef struct \
{ \
	type_lib_name lib_name; \
	bool load_library; \
	API_TAB_t * api_table; \
	bool optional; \
	bool end; \
	bool ignore; \
} CONCAT2(LIB_API_TAB_, endt) ## _t;

#define LIB_API_TAB_ENTRY_END {NULL, 0, NULL, 0, true, false}

LIB_API_TAB_CREATE_t(LPCWSTR, W)
LIB_API_TAB_CREATE_t(char *, A)

#ifdef UNICODE
#define LIB_API_TAB_t CONCAT2(LIB_API_TAB_, W) ## _t
#else
#define LIB_API_TAB_t CONCAT2(LIB_API_TAB_, A) ## _t
#endif // !UNICODE

#define LIB_API_TAB_GEN(typef, name, ...) typef name [] = { \
	__VA_ARGS__ ,\
	LIB_API_TAB_ENTRY_END \
};

#define LIB_API_TAB(name_table, ...) LIB_API_TAB_GEN(LIB_API_TAB_t, name_table, __VA_ARGS__)
#define LIB_API_TAB_A(name_table, ...) LIB_API_TAB_GEN(LIB_API_TAB_A_t, name_table, __VA_ARGS__)
#define LIB_API_TAB_W(name_table, ...) LIB_API_TAB_GEN(LIB_API_TAB_W_t, name_table, __VA_ARGS__)

#define API_TAB(name_table, ...) API_TAB_t name_table [] = { \
	__VA_ARGS__ ,\
	API_TAB_ENTRY_END \
};

#define API_TAB_ASSERT(ct, name_table, ...) API_TAB(name_table, __VA_ARGS__) \
	static_assert(ct == ARRAYSIZE(name_table) - 1, "The table nr elements and ct must be the same");

typedef enum
{
	GP_RET_ERR = -1,
	GP_RET_OK = 0,
	GP_RET_ERR_MEMALLOC,
	GP_RET_ERR_CONVUNICODE,
	GP_RET_ERR_GETADDRLIB,
	GP_RET_ERR_GETADDRFUNC

} GP_RET_t;

typedef struct
{
	union
	{
		DWORD last_ERR;
	};

} GP_EXT_ERR_t;

GETPROCLIB_API GP_RET_t GetProcW(
	_In_ LIB_API_TAB_W_t * lib_api_tabs,
	_Inout_opt_ int * lib_i,
	_Inout_opt_ int * api_i,
	_Inout_opt_ GP_EXT_ERR_t * ext_err
	);

GETPROCLIB_API GP_RET_t GetProcA(
	_In_ LIB_API_TAB_A_t * lib_api_tabs,
	_Inout_opt_ int * lib_i,
	_Inout_opt_ int * api_i,
	_Inout_opt_ GP_EXT_ERR_t * ext_err
	);

#ifdef UNICODE
#define GetProc GetProcW
#else
#define GetProc GetProcA
#endif // !UNICODE

#endif /*  _GET_PROC_LIB_H__ */