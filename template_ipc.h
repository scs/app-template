/*! @file template_ipc.h
 * @brief Shared header file between application and its
 * CGI. Contains all information relevant to IPC between these two.
 */
 #ifndef TEMPLATE_IPC_H_
#define TEMPLATE_IPC_H_

/* The parameter IDs to identify the different requests/responses.
 */
enum EnIpcParamIds
{
	GET_APP_STATE,
    GET_COLOR_IMG,
    GET_RAW_IMG,
    SET_CAPTURE_MODE
};

/*! @brief The path of the unix domain socket used for IPC between
 * the application and its user interface. */
#define USER_INTERFACE_SOCKET_PATH "/tmp/IPCSocket.sock"

/*! @brief Describes a rectangular sub-area of an image. */
struct IMG_RECT
{
    /*! @brief Rectangle width. */
    uint16      width;
    /*! @brief Rectangle height. */
    uint16      height;
    /*! @brief X Coordinate of the lower left corner.*/
    uint16      xPos;
    /*! @brief Y Coordinate of the lower left corne.*/
    uint16      yPos;
};

/*! @brief The different modes the application can be in. */
enum EnAppMode
{
	APP_OFF,
	APP_CAPTURE_COLOR,
	APP_CAPTURE_RAW	
};

/*! @brief Object describing all the state information the web interface needs 
 * to know about the application. */
struct APPLICATION_STATE
{
	/*! @brief Whether a new image is ready to display by the web interface. */
	bool		bNewImageReady;
	/*! @brief The time stamp when the last live image was taken. */
	uint32		imageTimeStamp;
	/*! @brief The mode the application is running in. Depending on the mode
	 * different information may have to be displayed on the web interface.*/	
	enum EnAppMode enAppMode;
};

#endif /*TEMPLATE_IPC_H_*/
