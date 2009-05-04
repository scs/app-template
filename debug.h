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
#ifndef DEBUG_H_
#define DEBUG_H_

#include "oscar.h"
/*********************************************************************//*!
 * @brief Write an image in int16 (or fract16) format to file (BMP)
 * for testing purposes.
 * 
 * Precision is automatically scaled down to 8 bit and the contents
 * are stored as greyscale image.
 * 
 * @param pData Data to be written as image
 * @param width Width of the image.
 * @param height Height of the image.
 * @param strPrefix Prefix of the file name of the image to be written.
 * This will be completed by a string representation of the sequence
 * number and the file type suffix (.bmp)
 * @param seq If the image is part of a sequence, a sequence number
 * can be specified here. It will be included as part of the file name.
 * Otherwise specify -1.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR WrDbgImgInt16(const int16 *pData, const uint16 width, const uint16 height, const char * strPrefix, int32 seq);

/*********************************************************************//*!
 * @brief Write an image in uint8 format to file (BMP)
 * for testing purposes.
 * 
 * @param pData Data to be written as image
 * @param width Width of the image.
 * @param height Height of the image.
 * @param strPrefix Prefix of the file name of the image to be written.
 * This will be completed by a string representation of the sequence
 * number and the file type suffix (.bmp)
 * @param seq If the image is part of a sequence, a sequence number
 * can be specified here. It will be included as part of the file name.
 * Otherwise specify -1.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR WrDbgImgUint8(const uint8 *pData, const uint16 width, const uint16 height, const char * strPrefix, int32 seq);

/*********************************************************************//*!
 * @brief Write a text to the file with the specified prefix for
 * debugging purposes.
 * 
 * @param strPrefix Prefix of the file name of the image to be written.
 * This will be completed by a string representation of the sequence
 * number and the file type suffix (.txt)
 * @param seq If the file is part of a sequence, a sequence number
 * can be specified here. It will be included as part of the file name.
 * Otherwise specify -1.
 * @param strFormat string for the content.
 * @param ... Format parameters of the content.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR WrDbgText(const char* strPrefix, int32 seq, const char *strFormat, ...);

/*********************************************************************//*!
 * @brief Write a data element to file
 * 
 * @param pData Pointer to the data to be written.
 * @param len Length of the data to be written.
 * @param strPrefix Prefix of the file name to be written to.
 * This will be completed by a string representation of the sequence
 * number and the file type suffix (.dat)
 * @param seq If the file is part of a sequence, a sequence number
 * can be specified here. It will be included as part of the file name.
 * Otherwise specify -1.
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR WrDbgData(void *pData, uint32 len, const char* strPrefix, int32 seq);

#endif /*DEBUG_H_*/
