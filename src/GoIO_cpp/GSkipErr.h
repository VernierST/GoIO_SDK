/*********************************************************************************

Copyright (c) 2010, Vernier Software & Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Vernier Software & Technology nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL VERNIER SOFTWARE & TECHNOLOGY BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**********************************************************************************/
//==================================================================================
// STATUS values returned in the errorStatus field of a GSkipDefaultResponsePacket.
//
// If SendCmdAndGetResponse() returns -1 and (1 == *pnRespBytes), then *pRespBuf contains
// NGIODefaultCmdResponse, even if a different response structure is defined for the command.
// Note that if you only need to know whether or not SendCmdAndGetResponse() fails, but you do not care about why,
// then you can always set the pRespBuf paramter to NULL.
//==================================================================================
#define SKIP_STATUS_SUCCESS 0
#define SKIP_STATUS_NOT_READY_FOR_NEW_CMD 0x30
#define SKIP_STATUS_CMD_NOT_SUPPORTED 0x31
#define SKIP_STATUS_INTERNAL_ERROR1 0x32
#define SKIP_STATUS_INTERNAL_ERROR2 0x33
#define SKIP_STATUS_ERROR_CANNOT_CHANGE_PERIOD_WHILE_COLLECTING 0x34
#define SKIP_STATUS_ERROR_CANNOT_READ_NV_MEM_BLK_WHILE_COLLECTING_FAST 0x35
#define SKIP_STATUS_ERROR_INVALID_PARAMETER 0x36
#define SKIP_STATUS_ERROR_CANNOT_WRITE_FLASH_WHILE_COLLECTING 0x37
#define SKIP_STATUS_ERROR_CANNOT_WRITE_FLASH_WHILE_HOST_FIFO_BUSY 0x38
#define SKIP_STATUS_ERROR_OP_BLOCKED_WHILE_COLLECTING 0x39
#define SKIP_STATUS_ERROR_CALCULATOR_CANNOT_MEASURE_WITH_NO_BATTERIES 0x3A
#define SKIP_STATUS_ERROR_SLAVE_POWERUP_INIT 0x40
#define SKIP_STATUS_ERROR_SLAVE_POWERRESTORE_INIT 0x41

