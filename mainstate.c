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

/*! @file mainstate.c
 * @brief Main State machine for template application.
 * 
 * Makes use of Framework HSM module.
	************************************************************************/

#include "template.h"
#include "mainstate.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

const Msg mainStateMsg[] = {
	{ FRAMESEQ_EVT },
	{ FRAMEPAR_EVT },
	{ IPC_GET_APP_STATE_EVT },
	{ IPC_GET_COLOR_IMG_EVT },
	{ IPC_GET_RAW_IMG_EVT },
	{ IPC_SET_CAPTURE_MODE_EVT }
};

/*********************************************************************//*!
 * @brief Inline function to throw an event to be handled by the statemachine.
 * 
 * @param pHsm Pointer to state machine
 * @param evt Event to be thrown.
 *//*********************************************************************/
void ThrowEvent(struct MainState *pHsm, unsigned int evt)
{
	const Msg *pMsg = &mainStateMsg[evt];
	HsmOnEvent((Hsm*)pHsm, pMsg);
}

/*********************************************************************//*!
 * @brief Checks for IPC events, schedules their handling and
 * acknowledges any executed ones.
 * 
 * @param pMainState Initalized HSM main state variable.
 * @return 0 on success or an appropriate error code.
 *//*********************************************************************/
static OSC_ERR HandleIpcRequests(MainState *pMainState)
{
	OSC_ERR err;
	uint32 paramId;
	
	err = CheckIpcRequests(&paramId);
	if (err == SUCCESS)
	{
		/* We have a request. See to it that it is handled
		 * depending on the state we're in. */
		switch(paramId)
		{
		case GET_APP_STATE:
			/* Request for the current state of the application. */
			ThrowEvent(pMainState, IPC_GET_APP_STATE_EVT);
			break;
		case GET_COLOR_IMG:
			/* Request for the live image. */
			ThrowEvent(pMainState, IPC_GET_COLOR_IMG_EVT);
			break;
		case GET_RAW_IMG:
			/* Request for the live image. */
			ThrowEvent(pMainState, IPC_GET_RAW_IMG_EVT);
			break;
		case SET_CAPTURE_MODE:
			/* Set the debayering option. */
			ThrowEvent(pMainState, IPC_SET_CAPTURE_MODE_EVT);
			break;
		default:
			OscLog(ERROR, "%s: Unkown IPC parameter ID (%d)!\n", __func__, paramId);
			data.ipc.enReqState = REQ_STATE_NACK_PENDING;
			break;
		}
	}
	else if (err == -ENO_MSG_AVAIL)
	{
		/* No new message available => do nothing. */
	}
	else
	{
		/* Error.*/
		OscLog(ERROR, "%s: IPC request error! (%d)\n", __func__, err);
		return err;
	}
	
	/* Try to acknowledge the new or any old unacknowledged
	 * requests. It may take several tries to succeed.*/
	err = AckIpcRequests();
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: IPC acknowledge error! (%d)\n", __func__, err);
	}
	return err;
}

Msg const *MainState_top(MainState *me, Msg *msg)
{
	switch (msg->evt)
	{
	case START_EVT:
		STATE_START(me, &me->captureColor);
		return 0;
	case IPC_GET_COLOR_IMG_EVT:
	case IPC_GET_RAW_IMG_EVT:
	case IPC_GET_APP_STATE_EVT:
	case IPC_SET_CAPTURE_MODE_EVT:
		/* If the IPC event is not handled in the actual substate, a negative acknowledge is returned by default. */
		data.ipc.enReqState = REQ_STATE_NACK_PENDING;
	return 0;
	}
	return msg;
}

Msg const *MainState_CaptureColor(MainState *me, Msg *msg)
{
	struct APPLICATION_STATE *pState;
	bool bCaptureColor;
	
	switch (msg->evt)
	{
	case ENTRY_EVT:
		data.ipc.state.enAppMode = APP_CAPTURE_COLOR;
		data.pCurRawImg = data.u8FrameBuffers[0];
		return 0;
	case FRAMESEQ_EVT:
		/* Sleep here for a short while in order not to violate the vertical
		 * blank time of the camera sensor when triggering a new image
		 * right after receiving the old one. This can be removed if some
		 * heavy calculations are done here. */
		usleep(1000);
		return 0;
	case FRAMEPAR_EVT:
		/* Process the image. */
		ProcessFrame(data.pCurRawImg);
		
		/* Timestamp the capture of the image. */
		data.ipc.state.imageTimeStamp = OscSupCycGet();
		data.ipc.state.bNewImageReady = TRUE;
		return 0;
	case IPC_GET_APP_STATE_EVT:
		/* Fill in the response and schedule an acknowledge for the request. */
		pState = (struct APPLICATION_STATE*)data.ipc.req.pAddr;
		memcpy(pState, &data.ipc.state, sizeof(struct APPLICATION_STATE));
		
		data.ipc.enReqState = REQ_STATE_ACK_PENDING;
		return 0;
	case IPC_GET_COLOR_IMG_EVT:
		/* Write out the image to the address space of the CGI. */
		memcpy(data.ipc.req.pAddr, data.u8ResultImage, sizeof(data.u8ResultImage));
		
		data.ipc.state.bNewImageReady = FALSE;
		
		/* Mark the request as executed, so it will be acknowledged later. */
		data.ipc.enReqState = REQ_STATE_ACK_PENDING;
		return 0;
	case IPC_SET_CAPTURE_MODE_EVT:
		/* Read the option from the address space of the CGI. */
		bCaptureColor = *((bool*)data.ipc.req.pAddr);
		if (bCaptureColor == FALSE)
		{
			/* Need to capture raw images from now on, this is done in the captureRaw state.  */
			STATE_TRAN(me, &me->captureRaw);
		}
		data.ipc.enReqState = REQ_STATE_ACK_PENDING;
		return 0;
	}
	return msg;
}

Msg const *MainState_CaptureRaw(MainState *me, Msg *msg)
{
	struct APPLICATION_STATE *pState;
	bool bCaptureColor;
	
	switch (msg->evt)
	{
	case ENTRY_EVT:
		data.ipc.state.enAppMode = APP_CAPTURE_RAW;
		data.pCurRawImg = data.u8FrameBuffers[0];
		return 0;
	case FRAMESEQ_EVT:
		/* Timestamp the capture of the image. */
		data.ipc.state.imageTimeStamp = OscSupCycGet();
		data.ipc.state.bNewImageReady = TRUE;
		
		/* Sleep here for a short while in order not to violate the vertical
		 * blank time of the camera sensor when triggering a new image
		 * right after receiving the old one. This can be removed if some
		 * heavy calculations are done here. */
		usleep(1000);
		
		return 0;
	case FRAMEPAR_EVT:
		return 0;
	case IPC_GET_APP_STATE_EVT:
		/* Fill in the response and schedule an acknowledge for the request. */
		pState = (struct APPLICATION_STATE*)data.ipc.req.pAddr;
		memcpy(pState, &data.ipc.state, sizeof(struct APPLICATION_STATE));
		
		data.ipc.enReqState = REQ_STATE_ACK_PENDING;
		return 0;
	case IPC_GET_RAW_IMG_EVT:
		/* Write out the raw image to the address space of the CGI. */
		memcpy(data.ipc.req.pAddr, data.pCurRawImg, OSC_CAM_MAX_IMAGE_WIDTH*OSC_CAM_MAX_IMAGE_HEIGHT);
		
		data.ipc.state.bNewImageReady = FALSE;
		
		/* Mark the request as executed, so it will be acknowledged later. */
		data.ipc.enReqState = REQ_STATE_ACK_PENDING;
		return 0;
	case IPC_SET_CAPTURE_MODE_EVT:
		/* Read the option from the address space of the CGI. */
		bCaptureColor = *((bool*)data.ipc.req.pAddr);
		if (bCaptureColor == TRUE)
		{
			/* Need to capture colored images from now on, this is done in the captureRaw state.  */
			STATE_TRAN(me, &me->captureColor);
		}
		data.ipc.enReqState = REQ_STATE_ACK_PENDING;
		return 0;
	}
	return msg;
}

void MainStateConstruct(MainState *me)
{
	HsmCtor((Hsm *)me, "MainState", (EvtHndlr)MainState_top);
	StateCtor(&me->captureRaw, "Capture Raw", &((Hsm *)me)->top, (EvtHndlr)MainState_CaptureRaw);
	StateCtor(&me->captureColor, "Capture Color", &((Hsm *)me)->top, (EvtHndlr)MainState_CaptureColor);
}

OSC_ERR StateControl( void)
{
	OSC_ERR camErr, err;
	MainState mainState;
	uint8 *pCurRawImg = NULL;
	
	/* Setup main state machine */
	MainStateConstruct(&mainState);
	HsmOnStart((Hsm *)&mainState);
	
	OscSimInitialize();
	
	/*----------- initial capture preparation*/
	camErr = OscCamSetupCapture( OSC_CAM_MULTI_BUFFER);
	if (camErr != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to setup initial capture (%d)!\n", __func__, camErr);
		return camErr;
	}
	err = OscGpioTriggerImage();
	if (err != SUCCESS)
	{
	OscLog(ERROR, "%s: Unable to trigger initial capture (%d)!\n", __func__, err);
	}
	
	/*----------- infinite main loop */
	while (TRUE)
	{
		/*----------- wait for captured picture */
		while (TRUE)
		{
			err = HandleIpcRequests(&mainState);
			if (err != SUCCESS)
			{
				OscLog(ERROR, "%s: IPC error! (%d)\n", __func__, err);
				Unload();
				return err;
			}
			
			camErr = OscCamReadPicture(OSC_CAM_MULTI_BUFFER, &pCurRawImg, 0, CAMERA_TIMEOUT);
			if (camErr != -ETIMEOUT)
			{
				/* Anything other than a timeout means that we should
				 * stop trying and analyze the situation. */
				break;
			}
			else
			{
				/*----------- procress CGI request
				 * Check for CGI request only if ReadPicture generated a
				 * time out. Process request directely or involve state
				 * engine with event */
				
				/* Read request. */
				err = HandleIpcRequests(&mainState);
				if (err != SUCCESS)
				{
					OscLog(ERROR, "%s: IPC error! (%d)\n", __func__, err);
					Unload();
					return err;
				}
			}
		}
		
		if (camErr == -EPICTURE_TOO_OLD)
		{
			/* We have a picture, but it already has been laying
			 * around for a while. Most likely we won't be able to
			 * make the deadline for this picture, so we better just
			 * give it up and don't portrude our delay to the next
			 * frame. */
			OscLog(WARN, "%s: Captured picture too old!\n", __func__);
			
			/*----------- prepare next capture */
			camErr = OscCamSetupCapture( OSC_CAM_MULTI_BUFFER);
			if (camErr != SUCCESS)
			{
				OscLog(ERROR, "%s: Unable to setup capture (%d)!\n", __func__, camErr);
				break;
			}
			err = OscGpioTriggerImage();
			if (err != SUCCESS)
			{
				OscLog(ERROR, "%s: Unable to trigger capture (%d)!\n", __func__, err);
				break;
			}
			continue;
		}
		else if (camErr != SUCCESS)
		{
			/* Fatal error, giving up. */
			OscLog(ERROR, "%s: Unable to read picture from cam!\n", __func__);
			break;
		}
		
		data.pCurRawImg = pCurRawImg;
		
		/*----------- process frame by state engine (pre-setup) Sequentially with next capture */
		ThrowEvent(&mainState, FRAMESEQ_EVT);
		
		/*----------- prepare next capture */
		camErr = OscCamSetupCapture( OSC_CAM_MULTI_BUFFER);
		if (camErr != SUCCESS)
		{
			OscLog(ERROR, "%s: Unable to setup capture (%d)!\n", __func__, camErr);
			break;
		}
	err = OscGpioTriggerImage();
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Unable to trigger capture (%d)!\n", __func__, err);
		break;
	}
	
	
	/*----------- process frame by state engine (post-setup) Parallel with next capture */
	ThrowEvent(&mainState, FRAMEPAR_EVT);
	
	/* Advance the simulation step counter. */
	OscSimStep();
	} /* end while ever */
	
	return SUCCESS;
}
