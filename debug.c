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

/*! @file debug.h
 * @brief Contains a few facilities to be able to debug the code easier.
 */
#include "debug.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

OSC_ERR WrDbgImgInt16(const int16 *pData,  const uint16 width,  const uint16 height, const char * strPrefix, int32 seq)
{
	struct OSC_PICTURE pic;
	OSC_ERR err;
	int i;
	uint8 *pPix;
	char strName[256];
	char strTemp[16];
	
	pPix = (uint8*)malloc(width*height);
	pic.width = width;
	pic.height = height;
	pic.type = OSC_PICTURE_GREYSCALE;
	pic.data = (void*)pPix;
	strcpy(strName, strPrefix);
	if (seq >= 0)
	{
		sprintf(strTemp, "%05u.bmp", (unsigned int)seq);
		strcat(strName, strTemp);
	}
	else
	{
		strcat(strName, ".bmp");
	}
	
	for (i = 0; i < width*height; i++)
	{
		pPix[i] = (uint8)(((uint32)((int32)pData[i] + 0x8000)) >> 8);
	}
	err = OscBmpWrite(&pic, strName);
	
	free(pPix);
	return err;
}

OSC_ERR WrDbgImgUint16(const uint16 *pData, const uint16 width,  const uint16 height, const char * strPrefix, int32 seq)
{
	struct OSC_PICTURE pic;
	OSC_ERR err;
	int i;
	uint8 *pPix;
	char strName[256];
	char strTemp[16];
	
	pPix = (uint8*)malloc(width*height);
	pic.width = width;
	pic.height = height;
	pic.type = OSC_PICTURE_GREYSCALE;
	pic.data = (void*)pPix;
	strcpy(strName, strPrefix);
	if (seq >= 0)
	{
		sprintf(strTemp, "%05u.bmp", (unsigned int)seq);
		strcat(strName, strTemp);
	}
	else
	{
		strcat(strName, ".bmp");
	}
	
	for (i = 0; i < width*height; i++)
	{
		pPix[i] = (uint8)(pData[i] >> 8);
	}
	err = OscBmpWrite(&pic, strName);
	
	free(pPix);
	return err;
}

OSC_ERR WrDbgImgUint8(const uint8 *pData, const uint16 width, const uint16 height, const char * strPrefix, int32 seq)
{
	struct OSC_PICTURE pic;
	OSC_ERR err;
	char strName[256];
	char strTemp[16];
	
	
	pic.width = width;
	pic.height = height;
	pic.type = OSC_PICTURE_GREYSCALE;
	pic.data = (uint8*)pData;
	strcpy(strName, strPrefix);
	if (seq != -1)
	{
		sprintf(strTemp, "%05u.bmp", (unsigned int)seq);
		strcat(strName, strTemp);
	}
	else
	{
		strcat(strName, ".bmp");
	}
	
	err = OscBmpWrite(&pic, strName);
	
	return err;
}

OSC_ERR WrDbgText(const char* strPrefix, int32 seq, const char *strFormat, ...)
{
	FILE *pF;
	char strName[256];
	char strTemp[16];
	va_list ap; /*< The dynamic argument list */
	
	strcpy(strName, strPrefix);
	if (seq != -1)
	{
		sprintf(strTemp, "%05u.txt", (unsigned int)seq);
		strcat(strName, strTemp);
	}
	else
	{
		strcat(strName, ".txt");
	}
	
	pF = fopen(strName, "w");
	if (pF == NULL)
	{
		return -EUNABLE_TO_OPEN_FILE;
	}
	
	va_start(ap, strFormat);
	vfprintf(pF, strFormat, ap);
	va_end(ap);
	fflush(pF);
	
	fclose(pF);
	
	return SUCCESS;
}

OSC_ERR WrDbgData(void *pData, uint32 len, const char* strPrefix, int32 seq)
{
	char strName[256];
	char strTemp[16];
	int32 ret;
	FILE *pF;
	
	strcpy(strName, strPrefix);
	if (seq >= 0)
	{
		sprintf(strTemp, "%05u.dat", (unsigned int)seq);
		strcat(strName, strTemp);
	}
	else
	{
		strcat(strName, ".dat");
	}
	
	pF = fopen(strName, "wb");
	if (pF == NULL)
	{
		return -EUNABLE_TO_OPEN_FILE;
	}
	
	ret = fwrite(pData, 1, len, pF);
	if (ret != len)
	{
		fclose(pF);
		return -EFILE_ERROR;
	}
	fclose(pF);
	
	return SUCCESS;
}
