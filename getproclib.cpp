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

#include "stdafx.h"
#include "getproclib.h"

// TODO: More comments..

GETPROCLIB_API GP_RET_t GetProcW(
	_In_ LIB_API_TAB_W_t * lib_api_tabs,
	_Inout_opt_ int * lib_i,
	_Inout_opt_ int * api_i,
	_Inout_opt_ GP_EXT_ERR_t * ext_err
	)
{
	int i = 0;
	int x = 0;
	GP_RET_t returnf = GET_RET_NAME(OK);

	// TODO: refactor this...
	for (i = 0; !lib_api_tabs[i].end && returnf == GP_RET_OK; i++)
	{
		HMODULE hlibrary;
		if (!lib_api_tabs[i].ignore)
		{
			if (lib_api_tabs[i].load_library)
				hlibrary = LoadLibraryW(lib_api_tabs[i].lib_name);
			else
				hlibrary = GetModuleHandleW(lib_api_tabs[i].lib_name);

			if (hlibrary == NULL)
			{
				if (!lib_api_tabs[i].optional)
				{
					returnf = GET_RET_NAME(ERR_GETADDRLIB);
					if (ext_err != NULL)
						ext_err->last_ERR = GetLastError();
				}
			}
			else
			{
				for (x = 0; !lib_api_tabs[i].api_table[x].end && returnf == GP_RET_OK; x++)
				{
					if (lib_api_tabs[i].api_table[x].need_resolv)
					{
						lib_api_tabs[i].api_table[x].api_addr = GetProcAddress(hlibrary, lib_api_tabs[i].api_table[x].api_name);
						if (lib_api_tabs[i].api_table[x].api_addr == NULL)
						{
							if (!lib_api_tabs[i].api_table[x].optional)
							{
								returnf = GET_RET_NAME(ERR_GETADDRFUNC);
								if (ext_err != NULL)
									ext_err->last_ERR = GetLastError();
							}
						}
						else
						{
							if (lib_api_tabs[i].api_table[x].func_ptr != NULL)
								*lib_api_tabs[i].api_table[x].func_ptr = lib_api_tabs[i].api_table[x].api_addr;
						}
					}
				}
			}
		}
	}

	if (returnf != GET_RET_NAME(OK))
	{
		if (lib_i != NULL)
			* lib_i = i - 1;

		if (api_i != NULL)
			* api_i = x - 1;
	}

	return returnf;
}

GETPROCLIB_API GP_RET_t GetProcA(
	_In_ LIB_API_TAB_A_t * lib_api_tabs,
	_Inout_opt_ int * lib_i,
	_Inout_opt_ int * api_i,
	_Inout_opt_ GP_EXT_ERR_t * ext_err
	)
{
	GP_RET_t returnf = GET_RET_NAME(OK);
	WCHAR lib_name[(MAX_PATH * 2) + 1];
	LPCWSTR ptr_to_name = lib_name;
	int size_ptr_to_name = sizeof(lib_name);
	bool alloc = false;
	LIB_API_TAB_W_t lib_api_tab_w[2] = { 0 };
	int i;

	lib_api_tab_w[1].end = true;

	// TODO: refactor this..
	for (i = 0; !lib_api_tabs[i].end && returnf == GP_RET_OK; i++)
	{
		if (!lib_api_tabs[i].ignore)
		{
			int size_buffer = MultiByteToWideChar(CP_ACP, 0, lib_api_tabs[i].lib_name, -1, NULL, 0);
			size_buffer *= sizeof(WCHAR);
			if (size_buffer > size_ptr_to_name)
			{
				if (alloc)
					free((void *)ptr_to_name);
				alloc = false;

				size_ptr_to_name = size_buffer;
				ptr_to_name = (LPCWSTR)calloc(1, size_buffer);
				if (ptr_to_name == NULL)
				{
					returnf = GET_RET_NAME(ERR_MEMALLOC);
					break;
				}
				else
					alloc = true;
			}
			MultiByteToWideChar(CP_ACP, 0, lib_api_tabs[i].lib_name, -1, (LPWSTR)ptr_to_name, size_buffer / sizeof(WCHAR));

			lib_api_tab_w->lib_name = ptr_to_name;

			lib_api_tab_w->api_table = lib_api_tabs[i].api_table;
			lib_api_tab_w->end = lib_api_tabs[i].end;
			lib_api_tab_w->load_library = lib_api_tabs[i].load_library;
			lib_api_tab_w->optional = lib_api_tabs[i].optional;

			returnf = GetProc(lib_api_tab_w, NULL, api_i, ext_err);
		}
	}

	if (alloc)
		free((void *)ptr_to_name);

	if (returnf != GET_RET_NAME(OK))
	{
		if (lib_i != NULL)
			* lib_i = i - 1;
	}

	return returnf;
}

