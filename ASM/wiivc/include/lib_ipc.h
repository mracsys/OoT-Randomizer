/*-------------------------------------------------------------

ipc.h -- Interprocess Communication with Starlet

Copyright (C) 2008
Michael Wiedenbauer (shagkur)
Dave Murphy (WinterMute)
Hector Martin (marcan)

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.	The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.	This notice may not be removed or altered from any source
distribution.

-------------------------------------------------------------*/

#ifndef __IPC_H
#define __IPC_H

#include "types.h"

#define IPC_HEAP			 -1

#define IPC_OPEN_NONE		  0
#define IPC_OPEN_READ		  1
#define IPC_OPEN_WRITE		  2
#define IPC_OPEN_RW			  (IPC_OPEN_READ|IPC_OPEN_WRITE)

#define IPC_MAXPATH_LEN		 64

#define IPC_OK				  0
#define IPC_EINVAL			 -4
#define IPC_ENOHEAP			 -5
#define IPC_ENOENT			 -6
#define IPC_EQUEUEFULL		 -8
#define IPC_ENOMEM			-22

#ifdef __cplusplus
   extern "C" {
#endif /* __cplusplus */

typedef struct _ioctlv
{
	void *data;
	u32 len;
} ioctlv;

typedef s32 (*ipccallback)(s32 result,void *usrdata);

void* iosAlloc(int hid, size_t size);
s32 IOS_IoctlvFormat(s32 hId,s32 fd,s32 ioctl,const char *format,...);

#ifdef __cplusplus
   }
#endif /* __cplusplus */

#endif
