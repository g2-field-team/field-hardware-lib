//==============================================================================
//
// Title:		Device.h
// Description:	This module contains the software for communicating with the
//				LBNE Digitizer.  It will support two connection types:
//				1)	The FTDI FT2232H_mini_module is a dual USB device with a
//					serial port for the control path and FIFO for the data path.
//				2)	A gigabit Ethernet link is also available to provide control
//					and data paths over UDP.  Initially, this method will not be
//					used.
// Author:		Andrew Kreps
//
//==============================================================================

#ifndef __DEVICE_H__
#define __DEVICE_H__

//==============================================================================
// Constants
//==============================================================================

#define EVENT_HEADER_SIZE	0
#define MAX_PAYLOAD_DATA	0x400000000
#define MAX_CTRL_DATA 256

//==============================================================================
// Enumerated Constants
//==============================================================================

enum commConstants {
	commNone	= 0,
	commTCP		= 1,
	commUDP		= 2,
	commUSB		= 3
};

enum commandConstants {
	cmdNone			= 0,
	// Basic Commands
	cmdRead			= 1,
	cmdReadMask		= 2,
	cmdWrite		= 3,
	cmdWriteMask	= 4,
	// Array Commands
	cmdArrayRead	= 5,
	cmdArrayWrite	= 6,
	// Fifo Commands
	cmdFifoRead		= 7,
	cmdFifoWrite	= 8,
	// NV Write Commands
	cmdNVWrite		= 9,
	cmdNVArrayWrite	= 10,
	cmdNVEraseSector= 11,	// 4KB  - 16 pages
	cmdNVEraseBlock	= 12,	// 64KB - 256 pages
	cmdNVEraseChip	= 13,	// 16MB - 65536 pages
	numCommands
};

enum statusConstants {
	statusNoError			= 0,
	statusSendError			= 1,
	statusReceiveError		= 2,
	statusTimeoutError		= 3,
	statusAddressError		= 4,
	statusAlignError		= 5,
	statusCommandError		= 6,
	statusSizeError			= 7,
	statusReadError			= 8,		// Returned if read-only address is written
	statusWriteError		= 9,		// Returned if read-only address is written
	statusFlashReadError	= 10,
	statusFlashWriteError	= 11,
	statusFlashEraseError	= 12,
};

enum errorConstants {
	errorNoError			= 0,

	// Device Communication Errors
	errorCommConnect		= -1,
	errorCommDisconnect		= -2,
	errorCommDiscover		= -3,
	errorCommReceive		= -4,
	errorCommSend			= -5,
	errorCommReceiveZero	= -6,
	errorCommReceiveTimeout	= -7,
	errorCommSendZero		= -8,
	errorCommSendTimeout	= -9,
	errorCommType			= -10,
	errorCommPurge			= -11,
	errorCommQueue			= -12,
	
	// Device Data Errors
	errorDataConnect		= -101,
	errorDataLength			= -102,
	errorDataPurge			= -103,
	errorDataQueue			= -104,
	errorDataReceive		= -105,
	errorDataTimeout		= -106,
	errorDataDisconnect		= -107,
	
	// LBNE Errors
	errorEventTooLarge		= -201,
	errorEventTooSmall		= -202,
	errorEventTooMany		= -203,
	errorEventHeader		= -204,
	
	errorUnknown			= -1000,
};

//==============================================================================
// Types
//==============================================================================

typedef struct _Ctrl_Header {
	unsigned int length;
	unsigned int address;
	unsigned int command;
	unsigned int size;
	unsigned int status;
} Ctrl_Header;

typedef struct _Ctrl_Packet {
	Ctrl_Header	header;
	unsigned int		data[MAX_CTRL_DATA];
} Ctrl_Packet;

struct {
	// Registers in the ARM Processor			Address
	unsigned int comm_arm_fw_build;					//	0x00000010
	unsigned int IP4Address;						//	0x00000100
	unsigned int IP4Netmask;						//	0x00000104
	unsigned int IP4Gateway;						//	0x00000108
	unsigned int MACAddressLSB;						//	0x00000110
	unsigned int MACAddressMSB;						//	0x00000114
	unsigned int EthernetReset;						//	0x00000180
	unsigned int RestoreSelect;						//	0x00000200
	unsigned int Restore;   						//	0x00000204
	unsigned int PurgeDDR;							//	0x00000300
	unsigned int TCPSendBlockSize;					//	0x00000400
	
	// Registers in the Zynq FPGA				Address
	unsigned int comm_fpga_fw_build;				//	0x40000010
	unsigned int eventDataInterfaceSelect;			//	0x40000020
	unsigned int eventDataControl;					//	0x40000144
	unsigned int eventDataStatus;					//	0x40000190
} trolleyReg;

//==============================================================================
// Function Prototypes
//==============================================================================

// Connection Functions
void RegInit					(void);
int DeviceConnect				(char* deviceIP);
int DeviceDisconnect			(void);

// Slow Control Command Functions
int DeviceRead		(unsigned int address, unsigned int* value);
int DeviceWrite		(unsigned int address, unsigned int value);
int DeviceSet		(unsigned int address, unsigned int mask);
int DeviceClear		(unsigned int address, unsigned int mask);
int DeviceArrayRead	(unsigned int address, unsigned int size, unsigned int* data);
int DeviceArrayWrite(unsigned int address, unsigned int size, unsigned int* data);

// Event Data functions
int DataReceive					(void* data);
int DevicePurgeData				(void);

// Internal use only
int DevicePurgeComm				(void);
int SendReceive					(Ctrl_Packet* tx, Ctrl_Packet* rx);
int ClientTCPWrite_ErrorCheck	(int txStatus);
int ClientTCPRead_ErrorCheck	(int rxStatus);
int DeviceWriteMask	(unsigned int address, unsigned int mask, unsigned int value);
int DeviceReadMask	(unsigned int address, unsigned int mask, unsigned int* value);

// Data Processing Functions:
//int		LBNE_EventUnpack(Event* event);
//void	EventDataProcess (EventDataProcess_Args* edp_args);

#endif
