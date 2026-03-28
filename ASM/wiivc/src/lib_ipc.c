/*-------------------------------------------------------------

ipc.c -- Interprocess Communication with Starlet

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

-------------------------------------------------------------

Modified by mracsys to link against the Ocarina of Time
Virtual Console titles instead of libogc.

-------------------------------------------------------------*/

#include "lib_ipc.h"
#include "stdarg.h"
#include "vc.h"

#define IOS_MAXFMT_PARAMS		32

struct _ioctlvfmt_bufent
{
	void *ipc_buf;
	void *io_buf;
	s32 copy_len;
};

struct _ioctlvfmt_cbdata
{
	ipccallback user_cb;
	void *user_data;
	s32 num_bufs;
	u32 hId;
	struct _ioctlvfmt_bufent *bufs;
};

static s32 _ipc_hid = -1;

void* iosAlloc(s32 handle, size_t size) { return iosAllocAligned(handle, size, 32); }

static s32 __ioctlvfmtCB(s32 result,void *userdata)
{
	ipccallback user_cb;
	void *user_data;
	struct _ioctlvfmt_cbdata *cbdata;
	struct _ioctlvfmt_bufent *pbuf;

	cbdata = (struct _ioctlvfmt_cbdata*)userdata;

	// deal with data buffers
	if(cbdata->bufs) {
		pbuf = cbdata->bufs;
		while(cbdata->num_bufs--) {
			if(pbuf->ipc_buf) {
				// copy data if needed
				if(pbuf->io_buf && pbuf->copy_len)
					memcpy(pbuf->io_buf, pbuf->ipc_buf, pbuf->copy_len);
				// then free the buffer
				iosFree(cbdata->hId, pbuf->ipc_buf);
			}
			pbuf++;
		}
	}

	user_cb = cbdata->user_cb;
	user_data = cbdata->user_data;

	// mracsys
	// Move the `free` calls to the same function the cbdata
	// buffers are allocated

	// call the user callback
	if(user_cb)
		return user_cb(result, user_data);

	return result;
}

static s32 __ios_ioctlvformat_parse(const char *format,va_list args,struct _ioctlvfmt_cbdata *cbdata,s32 *cnt_in,s32 *cnt_io,struct _ioctlv **argv,s32 hId)
{
	s32 ret,i;
	void *pdata;
	void *iodata;
	char type,*ps;
	s32 len,maxbufs = 0;
	ioctlv *argp = NULL;
	struct _ioctlvfmt_bufent *bufp;

	if(hId == IPC_HEAP) hId = _ipc_hid;
	if(hId < 0) return IPC_EINVAL;

	maxbufs = strnlen(format,IOS_MAXFMT_PARAMS);
	if(maxbufs>=IOS_MAXFMT_PARAMS) return IPC_EINVAL;

	cbdata->hId = hId;
	// mracsys
	// Remove LWP heap references
	cbdata->bufs = iosAlloc(hId, (sizeof(struct _ioctlvfmt_bufent)*(maxbufs+1)));
	if(cbdata->bufs==NULL) return IPC_ENOMEM;

	argp = iosAlloc(hId,(sizeof(struct _ioctlv)*(maxbufs+1)));
	if(argp==NULL) {
		// mracsys
		// Remove LWP heap references
		iosFree(hId, cbdata->bufs);
		return IPC_ENOMEM;
	}

	*argv = argp;
	bufp = cbdata->bufs;
	memset(argp,0,(sizeof(struct _ioctlv)*(maxbufs+1)));
	memset(bufp,0,(sizeof(struct _ioctlvfmt_bufent)*(maxbufs+1)));

	cbdata->num_bufs = 1;
	bufp->ipc_buf = argp;
	bufp++;

	*cnt_in = 0;
	*cnt_io = 0;

	ret = IPC_OK;
	while(*format) {
		// mracsys
		// remove `tolower` safety check as the only call
		// for this function already uses lowercase chars
		type = *format;
		switch(type) {
			case 'b':
				pdata = iosAlloc(hId,sizeof(u8));
				if(pdata==NULL) {
					ret = IPC_ENOMEM;
					goto free_and_error;
				}
				*(u8*)pdata = va_arg(args,u32);
				argp->data = pdata;
				argp->len = sizeof(u8);
				bufp->ipc_buf = pdata;
				cbdata->num_bufs++;
				(*cnt_in)++;
				argp++;
				bufp++;
				break;
			case 'h':
				pdata = iosAlloc(hId,sizeof(u16));
				if(pdata==NULL) {
					ret = IPC_ENOMEM;
					goto free_and_error;
				}
				*(u16*)pdata = va_arg(args,u32);
				argp->data = pdata;
				argp->len = sizeof(u16);
				bufp->ipc_buf = pdata;
				cbdata->num_bufs++;
				(*cnt_in)++;
				argp++;
				bufp++;
				break;
			case 'i':
				pdata = iosAlloc(hId,sizeof(u32));
				if(pdata==NULL) {
					ret = IPC_ENOMEM;
					goto free_and_error;
				}
				*(u32*)pdata = va_arg(args,u32);
				argp->data = pdata;
				argp->len = sizeof(u32);
				bufp->ipc_buf = pdata;
				cbdata->num_bufs++;
				(*cnt_in)++;
				argp++;
				bufp++;
				break;
			case 'q':
				pdata = iosAlloc(hId,sizeof(u64));
				if(pdata==NULL) {
					ret = IPC_ENOMEM;
					goto free_and_error;
				}
				*(u64*)pdata = va_arg(args,u64);
				argp->data = pdata;
				argp->len = sizeof(u64);
				bufp->ipc_buf = pdata;
				cbdata->num_bufs++;
				(*cnt_in)++;
				argp++;
				bufp++;
				break;
			case 'd':
				argp->data = va_arg(args, void*);
				argp->len = va_arg(args, u32);
				(*cnt_in)++;
				argp++;
				break;
			case 's':
				ps = va_arg(args, char*);
				len = strnlen(ps,256);
				if(len>=256) {
					ret = IPC_EINVAL;
					goto free_and_error;
				}

				pdata = iosAlloc(hId,(len+1));
				if(pdata==NULL) {
					ret = IPC_ENOMEM;
					goto free_and_error;
				}
				memcpy(pdata,ps,(len+1));
				argp->data = pdata;
				argp->len = (len+1);
				bufp->ipc_buf = pdata;
				cbdata->num_bufs++;
				(*cnt_in)++;
				argp++;
				bufp++;
				break;
			case ':':
				format++;
				goto parse_io_params;
			default:
				ret = IPC_EINVAL;
				goto free_and_error;
		}
		format++;
	}

parse_io_params:
	while(*format) {
		// mracsys
		// remove `tolower` safety check as the only call
		// for this function already uses lowercase chars
		type = *format;
		switch(type) {
			case 'b':
				pdata = iosAlloc(hId,sizeof(u8));
				if(pdata==NULL) {
					ret = IPC_ENOMEM;
					goto free_and_error;
				}
				iodata = va_arg(args,u8*);
				*(u8*)pdata = *(u8*)iodata;
				argp->data = pdata;
				argp->len = sizeof(u8);
				bufp->ipc_buf = pdata;
				bufp->io_buf = iodata;
				bufp->copy_len = sizeof(u8);
				cbdata->num_bufs++;
				(*cnt_io)++;
				argp++;
				bufp++;
				break;
			case 'h':
				pdata = iosAlloc(hId,sizeof(u16));
				if(pdata==NULL) {
					ret = IPC_ENOMEM;
					goto free_and_error;
				}
				iodata = va_arg(args,u16*);
				*(u16*)pdata = *(u16*)iodata;
				argp->data = pdata;
				argp->len = sizeof(u16);
				bufp->ipc_buf = pdata;
				bufp->io_buf = iodata;
				bufp->copy_len = sizeof(u16);
				cbdata->num_bufs++;
				(*cnt_io)++;
				argp++;
				bufp++;
				break;
			case 'i':
				pdata = iosAlloc(hId,sizeof(u32));
				if(pdata==NULL) {
					ret = IPC_ENOMEM;
					goto free_and_error;
				}
				iodata = va_arg(args,u32*);
				*(u32*)pdata = *(u32*)iodata;
				argp->data = pdata;
				argp->len = sizeof(u32);
				bufp->ipc_buf = pdata;
				bufp->io_buf = iodata;
				bufp->copy_len = sizeof(u32);
				cbdata->num_bufs++;
				(*cnt_io)++;
				argp++;
				bufp++;
				break;
			case 'q':
				pdata = iosAlloc(hId,sizeof(u64));
				if(pdata==NULL) {
					ret = IPC_ENOMEM;
					goto free_and_error;
				}
				iodata = va_arg(args,u64*);
				*(u64*)pdata = *(u64*)iodata;
				argp->data = pdata;
				argp->len = sizeof(u64);
				bufp->ipc_buf = pdata;
				bufp->io_buf = iodata;
				bufp->copy_len = sizeof(u64);
				cbdata->num_bufs++;
				(*cnt_io)++;
				argp++;
				bufp++;
				break;
			case 'd':
				argp->data = va_arg(args, void*);
				argp->len = va_arg(args, u32);
				(*cnt_io)++;
				argp++;
				break;
			default:
				ret = IPC_EINVAL;
				goto free_and_error;
		}
		format++;
	}
	return IPC_OK;

free_and_error:
	for(i=0;i<cbdata->num_bufs;i++) {
		if(cbdata->bufs[i].ipc_buf!=NULL) iosFree(hId,cbdata->bufs[i].ipc_buf);
	}
	// mracsys
	// Remove LWP heap references
	iosFree(hId, cbdata->bufs);
	return ret;
}

s32 IOS_IoctlvFormat(s32 hId,s32 fd,s32 ioctl,const char *format,...)
{
	s32 ret;
	va_list args;
	s32 cnt_in,cnt_io;
	struct _ioctlv *argv;
	struct _ioctlvfmt_cbdata *cbdata;

	// mracsys
	// Remove LWP heap references
	// USB heap is slightly smaller, but should have space for the one
	// use of this function in `USB_GetDeviceList`
	cbdata = iosAlloc(hId, sizeof(struct _ioctlvfmt_cbdata));
	if(cbdata==NULL) return IPC_ENOMEM;

	memset(cbdata,0,sizeof(struct _ioctlvfmt_cbdata));

	va_start(args,format);
	ret = __ios_ioctlvformat_parse(format,args,cbdata,&cnt_in,&cnt_io,&argv,hId);
	va_end(args);
	if(ret<0) {
		// mracsys
		// Remove LWP heap references
		iosFree(hId, cbdata);
		return ret;
	}

	ret = IOS_Ioctlv(fd,ioctl,cnt_in,cnt_io,argv);
	__ioctlvfmtCB(ret,cbdata);

	// free buffer list
	iosFree(hId, cbdata->bufs);

	// free callback data
	iosFree(hId, cbdata);

	return ret;
}