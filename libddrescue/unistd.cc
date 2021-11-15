// Copyright (C) 2021 mana
//
// This file is part of ddrescue-windows.
//
// ddrescue-windows is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// ddrescue-windows is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ddrescue-windows.  If not, see <http://www.gnu.org/licenses/>.

#include "unistd.h"
#include <Windows.h>

unsigned int sleep(unsigned int seconds)
{
	Sleep(seconds * 1000);
	return 0;
}

int fsync(int fd)
{
	HANDLE hFile =(HANDLE) _get_osfhandle(fd);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return EBADF;
	}

	if(!FlushFileBuffers(hFile))
	{
		return EIO;
	}

	return 0;
}

int ftruncate(int fd, off_t length)
{
	HANDLE hFile = (HANDLE)_get_osfhandle(fd);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return EBADF;
	}

	if(!SetFileValidData(hFile, length))
	{
		return EIO;
	}

	return 0;
}



long sysconf(int name)
{
	switch (name)
	{
	case _SC_PAGESIZE:
		{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		return si.dwPageSize;
		}
	}
	return 0;
}
