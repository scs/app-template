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

/*! @file cgi_template.h
 * @brief Header file of the CGI used for the webinterface of the
 * template application.
 */

#ifndef CGI_TEMPLATE_H_
#define CGI_TEMPLATE_H_

#include "../template_ipc.h"

/*! @brief The maximum length of the POST argument string supplied
 * to this CGI.*/
#define MAX_ARGUMENT_STRING_LEN 1024
/*! @brief The maximum length of the name string of a GET/POST
 * argument. */
#define MAX_ARG_NAME_LEN 32

/*! @brief The file name of the live image. */
#define IMG_FN "../img.bmp"

/* @brief The different data types of the argument string. */
enum EnArgumentType
{
	STRING_ARG,
	INT_ARG,
	SHORT_ARG,
	BOOL_ARG
};

/*! @brief A single POST/GET argument. */
struct ARGUMENT
{
	/*! @brief The name of the argument. */
	char strName[MAX_ARG_NAME_LEN];
	/*! brief The data type of the argument. */
	enum EnArgumentType enType;
	/*! @brief Pointer to the variable this argument should be parsed
	 * to.*/
	void *pData;
	/*! @brief Pointer to a variable storing whether this argument has
	 * been supplied or not.*/
	bool *pbSupplied;
};

/*! @brief Holds the values parsed from the argument string. */
struct ARGUMENT_DATA
{
	/*! @brief True if color images should be returned.*/
	bool bDoCaptureColor;
	/*! @brief Says whether the argument bDoDebayering has been
	 * supplied or not. */
	bool bDoCaptureColor_supplied;
};

/*! @brief Main object structure of the CGI. Contains all 'global'
 * variables. */
struct CGI_TEMPLATE
{
	/*! @brief Handle to the framework. */
	void *hFramework;
	/*! @brief IPC channel ID*/
	OSC_IPC_CHAN_ID ipcChan;
	
	/*! @brief The raw argument string as supplied by the web server. */
	char strArgumentsRaw[MAX_ARGUMENT_STRING_LEN];
	/*! @brief Temporary variable for argument extraction. */
	char strArgumentsTemp[MAX_ARGUMENT_STRING_LEN];
	
	/*! @brief The state queried from the application. */
	struct APPLICATION_STATE appState;
	/*! @brief The GET/POST arguments of the CGI. */
	struct ARGUMENT_DATA    args;
	/*! @brief Temporary data buffer for the images to be saved. */
	uint8 imgBuf[3*OSC_CAM_MAX_IMAGE_WIDTH*OSC_CAM_MAX_IMAGE_HEIGHT];
};
#endif /*CGI_TEMPLATE_H_*/
