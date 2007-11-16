//==================================================================================
// STATUS values returned in the errorStatus field of a GSkipDefaultResponsePacket.
//
// Note that if the SKIP_MASK_INPUT_PACKET_ERROR_FLAG bit is set in the header of
// a command response packet, then the errorStatus field is used.
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

