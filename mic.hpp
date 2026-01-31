#pragma once
#include "wut.h"

/**
 * \defgroup mic Microphone
 * \ingroup mic
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef int MICHandle;

#define MIC_HANDLE_INVALID -1

typedef struct MICWorkMemory MICWorkMemory;
typedef struct MICStatus MICStatus;

typedef enum MICError
{
   MIC_ERROR_OK               = 0,
   MIC_ERROR_NOT_OPENED       = -1,
   MIC_ERROR_INVALID_HANDLE   = -2,
   MIC_ERROR_INIT             = -5,
   MIC_ERROR_ALREADY_CLOSED   = -7,
   MIC_ERROR_INVALID_INSTANCE = -8,
} MICError;

typedef enum MICStateFlags
{
   MIC_STATE_FLAG_INIT = 0x0000'0001,
   MIC_STATE_FLAG_OPEN = 0x0000'0004,
   MIC_STATE_FLAG_WORK_STOPPED = 0x0000'0008,
   MIC_STATE_FLAG_WORKING = 0x0000'0010,
   MIC_STATE_FLAG_AUTO_SELECTION = 0x0000'0020,
   MIC_STATE_FLAG_ECHO_CANCELLATION = 0x0000'0040,
   MIC_STATE_FLAG_INITIALIZING =  0x0000'1000,
   MIC_STATE_FLAG_UNINITIALIZING = 0x0000'2000,
   MIC_STATE_OPENING = 0x0000'4000,
   MIC_STATE_CLOSING = 0x0000'8000,
   MIC_STATE_GETTING_STATE = 0x0001'0000,
   MIC_STATE_SETTING_STATE = 0x0002'0000,

};

typedef enum MICInstance
{
   MIC_INSTANCE_0 = 0,
   MIC_INSTANCE_1 = 1
} MICInstance;

struct MICWorkMemory
{
   //! Maximum amount of samples at a time must be at least 0x2800.
   size_t sampleMaxCount;

   //! A 0x40 aligned buffer of size sampleMaxCount * 2.
   void *sampleBuffer;
};
WUT_CHECK_OFFSET(MICWorkMemory, 0x00, sampleMaxCount);
WUT_CHECK_OFFSET(MICWorkMemory, 0x04, sampleBuffer);
WUT_CHECK_SIZE(MICWorkMemory, 0x08);

struct MICStatus
{
   int state; // 1 << 1 = Open
   int availableData;
   int bufferPos;
};
WUT_CHECK_OFFSET(MICStatus, 0x00, state);
WUT_CHECK_OFFSET(MICStatus, 0x04, availableData);
WUT_CHECK_OFFSET(MICStatus, 0x08, bufferPos);
WUT_CHECK_SIZE(MICStatus, 0x0C);

/**
     * The second parameter to MICInit is unused, any value is valid.
     */
MICHandle
MICInit(MICInstance instance,
        int unused,
        MICWorkMemory *workMemory,
        MICError *outError);

MICError
MICOpen(MICHandle handle);

MICError
MICGetState(MICHandle handle,
            int state,
            uint32_t *outStateVal);

MICError
MICSetState(MICHandle handle,
            int state,
            uint32_t stateVal);

MICError
MICGetStatus(MICHandle handle,
             MICStatus *outStatus);

MICError
MICSetDataConsumed(MICHandle handle,
                   int dataAmountConsumed);

MICError
MICClose(MICHandle handle);

MICError
MICUninit(MICHandle handle);

#ifdef __cplusplus
}
#endif

/** @} */
