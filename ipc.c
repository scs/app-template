/*	A collection of example applications for the LeanXcam platform.
	Copyright (C) 2008 Supercomputing Systems AG
	
	This library is free software; you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2.1 of the License, or (at
	your option) any later version.
	
	This library is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
	General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public License
	along with this library; if not, write to the Free Software Foundation,
	Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*! @file main.c
 * @brief Implements the IPC handling of the template application.
 */

#include "template.h"
#include <string.h>

OSC_ERR CheckIpcRequests(uint32 *pParamId)
{
	OSC_ERR err;
	struct IPC_DATA *pIpc = &data.ipc;
	struct OSC_IPC_REQUEST *pReq = &pIpc->req;
	
	if (pIpc->enReqState != REQ_STATE_IDLE)
	{
		/* This means we still have an unacknowledged request from
		 * last time. Proceed with the acknowledgement instead of
		 * already getting new ones.*/
		return -ENO_MSG_AVAIL;
	}
	
	/* Get the next request. */
	err = OscIpcGetRequest(pIpc->ipcChan, pReq);
	if (err == SUCCESS)
	{
		/* We have a request. */
		
		/* In case of success simply return the parameter ID of the requested parameter. */
		*pParamId = pReq->paramID;
	}
	else
	{
		/* Getting request not successful => analyze why. */
		if (likely(err == -ENO_MSG_AVAIL))
		{
			/* Simply no request available. */
		}
		else
		{
			OscLog(ERROR, "%s: Error getting IPC request! (%d)\n", __func__, err);
		}
	}
	return err;
}

OSC_ERR AckIpcRequests()
{
	struct IPC_DATA *pIpc = &data.ipc;
	struct OSC_IPC_REQUEST *pReq = &pIpc->req;
	OSC_ERR err;
	bool bSuccess;
	
	if (pIpc->enReqState == REQ_STATE_IDLE)
	{
		/* Nothing to acknowledge. */
		return SUCCESS;
	}
	else if (pIpc->enReqState == REQ_STATE_NACK_PENDING)
	{
		bSuccess = FALSE;
	}
	else
	{
		bSuccess = TRUE;
	}
	
	err = OscIpcAckRequest(pIpc->ipcChan, pReq, bSuccess);
	if (err == SUCCESS)
	{
		/* Ack sent successfully. Now we're ready for the next request.*/
		pIpc->enReqState = REQ_STATE_IDLE;
	}
	else if (err == -ETRY_AGAIN)
	{
		/* Not really an error, just means we have to try again later, which will happen soon enough. */
		err = SUCCESS;
	}
	return err;
}

void IpcSendImage_fr16(fract16 *f16Image, uint32 nPixels)
{
	fract16 *pSrc = f16Image;
	int i;
	uint8 *pDst = (uint8*)data.ipc.req.pAddr;
	
	/* Copy the corresponding image to the address supplied in
	 * the request. Only copy the more significant byte since
	 * it will be written as an 8bit bmp and the source image
	 * is 16 bit. */
	for (i = 0; i < nPixels; i++)
	{
		*pDst = (uint8)(((uint32)((int32)(*pSrc) + 0x7fff)) >> 8);
		pSrc++;
		pDst++;
	}
}
