/* Copying and distribution of this file, with or without modification,
 * are permitted in any medium without royalty. This file is offered as-is,
 * without any warranty.
 */

/*! @file process_frame.c
 * @brief Contains the actual algorithm and calculations.
 */

/* Definitions specific to this application. Also includes the Oscar main header file. */
#include "template.h"

void ProcessFrame(uint8 *pRawImg)
{
	OSC_ERR err;
	enum EnBayerOrder enBayerOrder;
	
	err = OscCamGetBayerOrder(&enBayerOrder, 0, 0);
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Error getting bayer order! (%d)\n", __func__, err);
		return;
	}
	
	/* Use a framework function to debayer the image. */
	err = OscVisDebayer(pRawImg, OSC_CAM_MAX_IMAGE_WIDTH, OSC_CAM_MAX_IMAGE_HEIGHT, enBayerOrder, data.u8ResultImage);
	if (err != SUCCESS)
	{
		OscLog(ERROR, "%s: Error debayering image! (%d)\n", __func__, err);
		return;
	}
	
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
	/* |                                                                 */
	/* |                    Add your code here                           */
	/* |                                                                 */
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
}
