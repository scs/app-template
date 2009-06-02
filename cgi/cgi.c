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

/*! @file cgi_template.c
 * @brief CGI used for the webinterface of the SHT application.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cgi.h"

#include <time.h>

/*! @brief Main object structure of the CGI. Contains all 'global'
 * variables. */
struct CGI_TEMPLATE cgi;

/*! @brief All potential arguments supplied to this CGI. */
struct ARGUMENT args[] =
{
	{ "DoCaptureColor", BOOL_ARG, &cgi.args.bDoCaptureColor, &cgi.args.bDoCaptureColor_supplied }
};

/*! @brief Strips whiltespace from the beginning and the end of a string and returns the new beginning of the string. Be advised, that the original string gets mangled! */
char * strtrim(char * str) {
	char * end = strchr(str, 0) - 1;
	
	while (*str != 0 && strchr(" \t\n", *str) != NULL)
		str += 1;
	
	while (end > str && strchr(" \t\n", *end) != NULL)
		end -= 1;
	
	*(end + 1) = 0;
	
	return str;
}

/*********************************************************************//*!
 * @brief Split the supplied URI string into arguments and parse them.
 * 
 * Matches the argument string with the arguments list (args) and fills in
 * their values. Unknown arguments provoke an error, but missing
 * arguments are just ignored.
 * 
 * @param strSrc The argument string.
 * @param srcLen The length of the argument string.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static OSC_ERR CGIParseArguments()
{
	char buffer[1024];
	
	/* Intialize all arguments as 'not supplied' */
	for (int i = 0; i < sizeof args / sizeof (struct ARGUMENT); i += 1)
		*args[i].pbSupplied = false;
	
	while (fgets (buffer, sizeof buffer, stdin)) {
		struct ARGUMENT *pArg = NULL;
		char * key, * value = strchr(buffer, ':');
		
		if (value == NULL) {
			OscLog(ERROR, "%s: Invalid line: \"%s\"\n", __func__, buffer);
			return -EINVALID_PARAMETER;
		}

		*value = 0;
		value += 1;
		
		key = strtrim(buffer);
		value = strtrim(value);
				
		for (int i = 0; i < sizeof(args)/sizeof(struct ARGUMENT); i += 1) {
			if (strcmp(args[i].strName, key) == 0) {
				pArg = args + i;
				break;
			}
		}
		
		if (pArg == NULL) {
			OscLog(ERROR, "%s: Unknown argument encountered: \"%s\"\n", __func__, key);
			return -EINVALID_PARAMETER;
		} else {
			if (pArg->enType == STRING_ARG) {
				// FIXME: Could someone fix this buffer overflow?
				strcpy((char *) pArg->pData, value);
			} else if (pArg->enType == INT_ARG) {
				if (sscanf(value, "%d", (int *) pArg->pData) != 1) {
					OscLog(ERROR, "%s: Unable to parse int value of variable \"%s\" (%s)!\n", __func__, pArg->strName, value);
					return -EINVALID_PARAMETER;
				}
			} else if (pArg->enType == SHORT_ARG) {
				if (sscanf(value, "%hd", (short *) pArg->pData) != 1) {
					OscLog(ERROR, "%s: Unable to parse short value of variable \"%s\" (%s)!\n", __func__, pArg->strName, value);
					return -EINVALID_PARAMETER;
				}
			} else if (pArg->enType == BOOL_ARG) {
				if (strcmp(value, "true") == 0) {
					*((bool *) pArg->pData) = true;
				} else if (strcmp(value, "false") == 0) {
					*((bool *) pArg->pData) = false;
				} else {
					OscLog(ERROR, "CGI %s: Unable to parse boolean value of variable \"%s\" (%s)!\n", __func__, pArg->strName, value);
					return -EINVALID_PARAMETER;
				}
			}
			
			if (pArg->pbSupplied != NULL)
				*pArg->pbSupplied = true;
		}
	}
		
	return SUCCESS;
}

/*********************************************************************//*!
 * @brief Query the current state of the application and see what else
 * we need to get from it
 * 
 * Depending on the current state of the application, other additional
 * parameters may be queried.
 * 
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static OSC_ERR QueryApp()
{
	OSC_ERR err;
	struct OSC_PICTURE pic;
	
	/* First, get the current state of the algorithm. */
	err = OscIpcGetParam(cgi.ipcChan, &cgi.appState, GET_APP_STATE, sizeof(struct APPLICATION_STATE));
	if (err != SUCCESS)
	{
		/* This request is defined in all states, and thus must succeed. */
		OscLog(ERROR, "CGI: Error querying application! (%d)\n", err);
		return err;
	}
		
	switch(cgi.appState.enAppMode)
	{
	case APP_OFF:
		/* Algorithm is off, nothing else to do. */
		break;
	case APP_CAPTURE_COLOR:
		if (cgi.appState.bNewImageReady)
		{
			/* If there is a new image ready, request it from the application. */
			err = OscIpcGetParam(cgi.ipcChan,
					cgi.imgBuf,
					GET_COLOR_IMG,
					3*OSC_CAM_MAX_IMAGE_WIDTH*OSC_CAM_MAX_IMAGE_HEIGHT);
			if (err != SUCCESS)
			{
				OscLog(DEBUG, "CGI: Getting new image failed! (%d)\n", err);
				return err;
			}
			
			/* Write the image to the RAM file system where it can be picked
			 * up by the webserver on request from the browser. */
			pic.width = OSC_CAM_MAX_IMAGE_WIDTH;
			pic.height = OSC_CAM_MAX_IMAGE_HEIGHT;
			pic.type = OSC_PICTURE_BGR_24;
			pic.data = (void*)cgi.imgBuf;
			
			return OscBmpWrite(&pic, IMG_FN);
		}
		break;
	case APP_CAPTURE_RAW:
		if (cgi.appState.bNewImageReady)
		{
			/* If there is a new image ready, request it from the application. */
			err = OscIpcGetParam(cgi.ipcChan, cgi.imgBuf, GET_RAW_IMG, OSC_CAM_MAX_IMAGE_WIDTH*OSC_CAM_MAX_IMAGE_HEIGHT);
			if (err != SUCCESS)
			{
				OscLog(DEBUG, "CGI: Getting new image failed! (%d)\n", err);
				return err;
			}
			
			/* Write the image to the RAM file system where it can be picked up by the webserver on request from the browser. */
			pic.width = OSC_CAM_MAX_IMAGE_WIDTH;
			pic.height = OSC_CAM_MAX_IMAGE_HEIGHT;
			pic.type = OSC_PICTURE_GREYSCALE;
			pic.data = (void*)cgi.imgBuf;
			
			return OscBmpWrite(&pic, IMG_FN);
		}
		break;
	default:
		OscLog(ERROR, "%s: Invalid application mode (%d)!\n", __func__, cgi.appState.enAppMode);
		break;
	}
	return SUCCESS;
}

/*********************************************************************//*!
 * @brief Set the parameters for the application supplied by the web
 * interface.
 * 
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
static OSC_ERR SetOptions()
{
	OSC_ERR err;
	struct ARGUMENT_DATA *pArgs = &cgi.args;
	
	if (pArgs->bDoCaptureColor_supplied)
	{
		err = OscIpcSetParam(cgi.ipcChan, &pArgs->bDoCaptureColor, SET_CAPTURE_MODE, sizeof(pArgs->bDoCaptureColor));
		if (err != SUCCESS)
		{
			OscLog(DEBUG, "CGI: Error setting option! (%d)\n", err);
			return err;
		}
	}
	
	return SUCCESS;
}

/*********************************************************************//*!
 * @brief Take all the gathered info and formulate a valid AJAX response
 * that can be parsed by the Javascript in the browser.
 *//*********************************************************************/
static void FormCGIResponse()
{
	struct APPLICATION_STATE  *pAppState = &cgi.appState;
	
	/* Header */
	printf("Content-type: text/plain\n\n" );
	
	printf("imgTS: %u\n", (unsigned int)pAppState->imageTimeStamp);
	
	fflush(stdout);
}

/*********************************************************************//*!
 * @brief Execution starting point
 * 
 * Handles initialization, control and unloading.
 * @return 0 on success, -1 otherwise
 *//*********************************************************************/
int main()
{
	char *strContentLen;
	int contentLen;
	OSC_ERR err;
	struct stat socketStat;
	
	/* Initialize */
	
	memset(&cgi, 0, sizeof(struct CGI_TEMPLATE));
	
	/* First, check if the algorithm is even running and ready for IPC
	 * by looking if its socket exists.*/
	if(stat(USER_INTERFACE_SOCKET_PATH, &socketStat) != 0)
	{
		/* Socket does not exist => Algorithm is off. */
		/* Form a short reply with that info and shut down. */
		cgi.appState.enAppMode = APP_OFF;
		return 1;
	}
	
	err = OscCreate(&cgi.hFramework);
	if(err != SUCCESS)
	{
		return 1;
	}
	
	err = OscLogCreate(cgi.hFramework);
	if(err != SUCCESS)
	{
		OscDestroy(cgi.hFramework);
		return 1;
	}
	OscLogSetConsoleLogLevel(CRITICAL);
	OscLogSetFileLogLevel(DEBUG);
	
	err = OscIpcCreate(cgi.hFramework);
	if(err != SUCCESS)
	{
		OscLog(ERROR, "CGI: Unable to create IPC module (%d)!\n", err);
		OscLogDestroy(cgi.hFramework);
		OscDestroy(cgi.hFramework);
		return 1;
	}

	err = OscIpcRegisterChannel(&cgi.ipcChan, USER_INTERFACE_SOCKET_PATH, 0);
	if(err != SUCCESS)
	{
		OscLog(ERROR, "CGI: Unable to register IPC channel (%d)!\n", err);
		OscIpcDestroy(cgi.hFramework);
		OscLogDestroy(cgi.hFramework);
		OscDestroy(cgi.hFramework);
		return 1;
	}
	
	err = CGIParseArguments();
	if(err != SUCCESS) {
		OscLog(ERROR, "CGI: Error parsing command line arguments! \"%s\"\n", cgi.strArgumentsRaw);
		goto exit_unload;
	}
	
	/* The algorithm negative acknowledges if it cannot supply
	 * the requested data, i.e. it changed state during the
	 * process of getting the data.
	 * Try again until we succeed. */
	do
	{
		do
		{
			err = QueryApp();
		} while (err == -ENEGATIVE_ACKNOWLEDGE);
		
		if (err != SUCCESS)
		{
			OscLog(ERROR, "CGI: Error querying algorithm!\n");
			goto exit_unload;
		}
	err = SetOptions();
	} while (err == -ENEGATIVE_ACKNOWLEDGE);
	FormCGIResponse();

exit_unload:
	/* Unload */
	OscLogDestroy(cgi.hFramework);
	OscIpcDestroy(cgi.hFramework);
	OscDestroy(cgi.hFramework);
	return 0;
}
