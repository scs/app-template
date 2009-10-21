/* Copying and distribution of this file, with or without modification,
 * are permitted in any medium without royalty. This file is offered as-is,
 * without any warranty.
 */

/*! @file main.c
 * @brief Main file of the template application. It contains initialization
 * code.
 */

#include "template.h"
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

/*! @brief This stores all variables needed by the algorithm. */
struct TEMPLATE data;

/*********************************************************************//*!
 * @brief Initialize everything so the application is fully operable
 * after a call to this function.
 *
 * @return SUCCESS or an appropriate error code.
 *//*********************************************************************/
OscFunction(static Init, const int argc, const char * argv[])

	uint8 multiBufferIds[2] = {0, 1};

	memset(&data, 0, sizeof(struct TEMPLATE));

	/******* Create the framework **********/
	OscCall( OscCreate,
		&OscModule_cam,
		&OscModule_bmp,
		&OscModule_vis,
		&OscModule_hsm,
		&OscModule_ipc,
		&OscModule_gpio,
		&OscModule_log,
		&OscModule_sup);

	/* Seed the random generator */
	srand(OscSupCycGet());

	/* Set the camera registers to sane default values. */
	OscCall( OscCamPresetRegs);
	OscCall( OscCamSetupPerspective, OSC_CAM_PERSPECTIVE_DEFAULT);

	/* Configure camera emulation on host */
#if defined(OSC_HOST) || defined(OSC_SIM)
	OscCall( OscFrdCreateConstantReader, &data.hFileNameReader, TEST_IMAGE_FN);
	OscCall( OscCamSetFileNameReader, data.hFileNameReader);
#endif /* OSC_HOST or OSC_SIM */

	/* Set up two frame buffers for maximum image size. Cached memory.
	 * Register the buffers as multi-buffer for the camera */
	OscCall( OscCamSetFrameBuffer, 0, OSC_CAM_MAX_IMAGE_WIDTH*OSC_CAM_MAX_IMAGE_HEIGHT, data.u8FrameBuffers[0], TRUE);
	OscCall( OscCamSetFrameBuffer, 1, OSC_CAM_MAX_IMAGE_WIDTH*OSC_CAM_MAX_IMAGE_HEIGHT, data.u8FrameBuffers[1], TRUE);
	OscCall( OscCamCreateMultiBuffer, 2, multiBufferIds);

	/* Register an IPC channel to the CGI for the web interface. */
	OscCall( OscIpcRegisterChannel, &data.ipc.ipcChan, USER_INTERFACE_SOCKET_PATH, F_IPC_SERVER | F_IPC_NONBLOCKING);

OscFunctionCatch()
	/* Destruct framwork due to error above. */
	OscDestroy();
	OscMark_m( "Initialization failed!");

OscFunctionEnd()


/*********************************************************************//*!
 * @brief Program entry
 *
 * @param argc Command line argument count.
 * @param argv Command line argument strings.
 * @return 0 on success
 *//*********************************************************************/
OscFunction( mainFunction, const int argc, const char * argv[])

	/* Initialize system */
	OscCall( Init, argc, argv);

	OscLogSetConsoleLogLevel(INFO);
	OscLogSetFileLogLevel(WARN);

	StateControl();

OscFunctionCatch()
	OscDestroy();
	OscLog(INFO, "Quit application abnormally!\n");
OscFunctionEnd()

int main(const int argc, const char * argv[]) {
	if (mainFunction(argc, argv) == SUCCESS)
		return 0;
	else
		return 1;
}
