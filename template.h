/*! @file template.h
 * @brief Global header file for the template application.
 */
#ifndef TEMPLATE_H_
#define TEMPLATE_H_

/*--------------------------- Includes -----------------------------*/
#include "inc/oscar.h"
#include "debug.h"
#include "template_ipc.h"
#include <stdio.h>

/*--------------------------- Settings ------------------------------*/
/*! @brief The number of frame buffers used. */
#define NR_FRAME_BUFFERS 2

/*! @brief Timeout (ms) when waiting for a new picture. */
#define CAMERA_TIMEOUT 1

/*! @brief The file name of the test image on the host. */
#define TEST_IMAGE_FN "test.bmp"

/*------------------- Main data object and members ------------------*/

/*! @brief The different states of a pending IPC request. */
enum EnIpcRequestState
{
    REQ_STATE_IDLE,
    REQ_STATE_ACK_PENDING,
    REQ_STATE_NACK_PENDING
};

/*! @brief Holds all the data needed for IPC with the user interface.*/
struct IPC_DATA
{
    /*! @brief ID of the IPC channel used to communicate with the 
     * webinterface. */
    OSC_IPC_CHAN_ID             ipcChan;
    /*! @brief An unacknowledged request. */
    struct OSC_IPC_REQUEST      req;
    /*! @brief The state of above IPC request. */
    enum EnIpcRequestState      enReqState;
    
    /*! @brief All the information requested by the web interface is gathered
     * here. */
    struct APPLICATION_STATE	state;
};

/*! @brief The structure storing all important variables of the application.
 * */
struct TEMPLATE
{
    /*! @brief The frame buffers for the frame capture device driver.*/
    uint8               u8FrameBuffers[NR_FRAME_BUFFERS][OSC_CAM_MAX_IMAGE_HEIGHT*OSC_CAM_MAX_IMAGE_WIDTH];
    /*! @brief A buffer to hold the resulting color image. */ 
    uint8				u8ResultImage[3*OSC_CAM_MAX_IMAGE_WIDTH*OSC_CAM_MAX_IMAGE_HEIGHT];
       
    /*! @brief Handle to the framework instance. */
    void                *hFramework;    
   	/*! @brief Camera-Scene perspective */
    enum EnOscCamPerspective perspective;
    
#if defined(OSC_HOST) || defined(OSC_SIM)
    /*! @brief File name reader for camera images on the host. */
    void				*hFileNameReader;
#endif /* OSC_HOST or OSC_SIM */
    /*! @brief The last raw image captured. Always points to one of the frame
     * buffers. */
    uint8*				pCurRawImg;
    /*! @brief All data necessary for IPC. */
    struct IPC_DATA     ipc;   

};

extern struct TEMPLATE data;

/*-------------------------- Functions --------------------------------*/
/*********************************************************************//*!
 * @brief Unload everything before exiting.
 * 
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR Unload();

/*********************************************************************//*!
 * @brief Give control to statemachine. 
 *
 * @return SUCCESS or an appropriate error code otherwise    
 *
 * The function does never return normally except in error case.
 *//*********************************************************************/
OSC_ERR StateControl( void);

/*********************************************************************//*!
 * @brief Handle any incoming IPC requests.
 * 
 * Check for incoming IPC requests and return the corresponding parameter
 * ID if there is a request available.
 * 
 * @param pParamId Pointer to the variable where the parameter ID is
 * stored in case of success.
 * @return SUCCESS, -ENO_MSG_AVAILABLE or an appropriate error code.
 *//*********************************************************************/
OSC_ERR CheckIpcRequests(uint32 *pParamId);

/*********************************************************************//*!
 * @brief Acknowledge any pending IPC requests.
 * 
 * It may take several calls to this function for an acknowledge to
 * succeed.
 * 
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OSC_ERR AckIpcRequests();

/*********************************************************************//*!
 * @brief Write an image of type fract16 to the result pointer of 
 * the current request.
 * 
 * @param f16Image The image to be sent.
 * @param nPixels The number of pixels in the image.
 *//*********************************************************************/
void IpcSendImage(fract16 *f16Image, uint32 nPixels);

/*********************************************************************//*!
 * @brief Process a newly captured frame.
 * 
 * In the case of this template, this consists just of debayering the
 * image and writing the result to the result image buffer. This should 
 * be the starting point where you add your code.
 * 
 * @param pRawImg The raw image to process.
 *//*********************************************************************/
void ProcessFrame(uint8 *pRawImg);

#endif /*TEMPLATE_H_*/
