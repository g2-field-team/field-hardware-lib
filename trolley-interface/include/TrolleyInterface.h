//==============================================================================
//
// Title:		TrolleyInterface.h
// Description:	This module contains the software for communicating with the
//				gm2 trolley Digitizer.  It will support two connection types:
//				1)	The FTDI FT2232H_mini_module is a dual USB device with a
//					serial port for the control path and FIFO for the data path.
//				2)	A gigabit Ethernet link is also available to provide control
//					and data paths over UDP.  Initially, this method will not be
//					used.
// Author:		Andrew Kreps, Ran Hong
//
//==============================================================================

#ifndef __TROLLEYINTERFACE_H__
#define __TROLLEYINTERFACE_H__

//==============================================================================
// Constants
//==============================================================================

#define EVENT_HEADER_SIZE	0x80
#define MAX_PAYLOAD_DATA	0x100000
#define MAX_NMR_SAMPLES		24000
#define MAX_BARCODE_SAMPLES	3000
#define MAX_CTRL_DATA 0x100

namespace TrolleyInterface{
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

    // File Error
    errorEOF			= -300,
    errorFileFail		= -301,

    errorUnknown			= -1000,
  };

  enum TrolleyCommRegister{
    reg_max_di_dt		=0x40000300,	//Trolley protection, maximum di/dt
    reg_v_prot_threshold	=0x40000304,	//Trolley protection, v threshold
    reg_i_prot_threshold	=0x40000308,	//Trolley protection, i threshold
    reg_v_prot_limit 		=0x4000030C,	//Trolley protection, v limit
    reg_i_prot_limit 		=0x40000310,	//Trolley protection, i limit

    reg_command			=0x40000400,	//Trolley command

    reg_comm_t_tid_stop		=0x40000404,	//Cycle Timing:
    reg_comm_t_td_start		=0x40000408,	//Cycle Timing:
    reg_comm_t_td		=0x4000040C,	//Cycle Timing:
    reg_comm_t_td_stop		=0x40000410,	//Cycle Timing:
    reg_comm_t_switch_rf	=0x40000414,	//Cycle Timing:
    reg_comm_t_power_on		=0x40000418,	//Cycle Timing:
    reg_comm_t_rf_on		=0x4000041C,	//Cycle Timing:
    reg_comm_t_switch_comm	=0x40000420,	//Cycle Timing:
    reg_comm_t_tid_start	=0x40000424,	//Cycle Timing:
    reg_comm_t_cycle_length	=0x40000428,	//Cycle Timing:

    reg_bc_sample_period	=0x4000042C,	//Barcode ADC: 
    reg_bc_t_acq		=0x40000430,	//Barcode ADC:
    reg_bc_refdac1		=0x40000434,	//Barcode ADC:
    reg_bc_refdac2		=0x40000438,	//Barcode ADC:
    reg_bc_refcm		=0x4000043C,	//Barcode ADC:

    reg_power_control1		=0x40000440,	//Power Control:
    reg_power_control2		=0x40000444,	//Power Control:

    reg_nmr_rf_prescale		=0x40000448,	//NMR Sequencing
    reg_ti_switch_rf_offset	=0x40000480,	//Cycle Timing:
    reg_ti_switch_comm_offset	=0x40000484,	//Cycle Timing:

    reg_timestamp_control	=0x40000600,	//Timing
    reg_timestamp_latch		=0x40000604,	//Timing
    reg_utc_time 		=0x40000608,	//Timing
    reg_timestamp_0 		=0x4000060C,	//Timing
    reg_timestamp_1		=0x40000610,	//Timing
    reg_irig_hms 		=0x40000614,	//Timing
    reg_irig_yd 		=0x40000618,	//Timing
    reg_irig_second_of_the_day 	=0x4000061C,	//Timing
    reg_irig_control_function 	=0x40000620,	//Timing
    reg_irig_raw_data_0 	=0x40000624,	//Timing
    reg_irig_raw_data_1 	=0x40000628,	//Timing
    reg_irig_raw_data_2 	=0x4000062C,	//Timing
    reg_irig_cycle_count 	=0x40000630,	//Timing
    reg_irig_phase_meas 	=0x40000634,	//Timing
    reg_irig_100pps_phase_meas	=0x40000638,	//Timing
    reg_irig_1khz_phase_meas 	=0x4000063C,	//Timing
    reg_irig_phase_offset 	=0x40000640,	//Timing

    reg_timing_control		=0x40000700,
    reg_diag_control		=0x40000704,
    reg_nmr_control		=0x40000708,
    reg_nmr_status		=0x4000070C,
    reg_power_control		=0x40000710,
    reg_power_status		=0x40000714,
    reg_led_control		=0x40000718,
    reg_led_status		=0x4000071C,
    reg_trolley_ldo_config	=0x40000720,
    reg_trolley_ldo_config_load	=0x40000724,
    reg_trolley_ldo_status	=0x40000728,

    reg_comm_fpga_fw_build	=0x40000810,
    reg_event_data_control	=0x40000944,	//Data folow On/Off, On=0, Off=1
    reg_free_event_memory	=0x40000994,	//Free event memory

    reg_comm_act_led		=0x40000A00,	//Internal Use Onlly
    reg_comm_link_led		=0x40000A04,	//Internal Use Onlly
    reg_event_data_head		=0x40000B00,	//Internal Use Onlly
    reg_event_data_tail		=0x40000B04,	//Internal Use Onlly
    reg_dsp_flash_control	=0x40000C00,	//Internal Use Onlly

    //FIFO
    fifo_out_nmr_seq_data	=0x40000F00,
    fifo_out_flash_data		=0x40000F04,
    fifo_out_test_data		=0x40000F08,
    fifo_in_test_data		=0x40000F0C
  };
  //==============================================================================
  // Types
  //==============================================================================
  
  typedef struct _NMR_Config {
    unsigned int NMR_Command;
    unsigned int NMR_Preamp_Delay;
    unsigned int NMR_Preamp_Period;	
    unsigned int NMR_ADC_Gate_Delay;
    unsigned int NMR_ADC_Gate_Offset;
    unsigned int NMR_ADC_Gate_Period;												
    unsigned int NMR_TX_Delay;
    unsigned int NMR_TX_Period;
    unsigned int User_Defined_Data;
  } NMR_Config;

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

  struct _trolleyReg{
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

  typedef struct _A_FRAME_INFO {
    unsigned int frame_length;
    unsigned int frame_number;
    unsigned int probe_number;
    unsigned int power_status;
    unsigned int nmr_samples;
    unsigned int barcode_samples;
    unsigned int flash_data_length;
    float voltage_monitor_min[2];
    float voltage_monitor_max[2];
    float temperature_monitor[4];
    unsigned int press_cal[7];
    unsigned int press_raw_press;
    unsigned int press_raw_temp;
    unsigned long long int timestamp_cycle_start;
    unsigned long long int timestamp_nmr_start;
    unsigned long long int timestamp_barcode_start;
    unsigned int nmr_waveform_checksum;
    unsigned int nmr_timestamp_offset;
    unsigned short int bc_config_out;        
    unsigned short int bc_refdac1_out;
    unsigned short int bc_refdac2_out;
    unsigned short int bc_seqfifo_out;        
    unsigned short int bc_refcm_out;       
    unsigned int rf_power_meas[2];
    const unsigned short int* trolley_command_frame;
    const short int* nmr_waveform;
    const unsigned short int* barcode_waveform[2][3];
    const unsigned short int* voltage_monitor_waveform[2];
    const unsigned char* flash_data;
    double press_meas_press;
    double press_meas_temp;
    double rf_power_factor;
  } A_FRAME_INFO;

typedef struct _B_FRAME_INFO {

  unsigned int mon_samples;
  unsigned int	power_status;    
  unsigned int	ldo_status;    
  unsigned int	reserved;    
  unsigned long long int timestamp_cycle_start_irig;
  unsigned long long int timestamp_cycle_start_local;
  unsigned long long int timestamp_monitor_start;
  float temperature_monitor_min;
  float temperature_monitor_max;
  float voltage_monitor_min[4];
  float voltage_monitor_max[4];
  unsigned int rf_power_meas[2];
  unsigned int reg_ti_switch_rf_offset;
  unsigned int reg_ti_switch_comm_offset;
  const unsigned short int* v_mon_waveform;
  const unsigned short int* i_mon_waveform;
  double rf_power_factor;

} B_FRAME_INFO;

  //==============================================================================
  // Function Prototypes
  //==============================================================================

  // Connection Functions
  void RegInit					(void);
  int DeviceConnect				(const char* deviceIP);
  int DeviceDisconnect			(void);
  int FileOpen(const char * filename);
  int FileClose();

  // Slow Control Command Functions
  int DeviceRead		(unsigned int address, unsigned int* value);
  int DeviceWrite		(unsigned int address, unsigned int value);
  int DeviceSet		(unsigned int address, unsigned int mask);
  int DeviceClear		(unsigned int address, unsigned int mask);
  int DeviceArrayRead	(unsigned int address, unsigned int size, unsigned int* data);
  int DeviceArrayWrite(unsigned int address, unsigned int size, unsigned int* data);

  // Event Data functions
  int DataReceive (void (*const a_frame), void (*const b_frame), unsigned int (*const a_frame_length), unsigned int (*const b_frame_length));
  void DataPreProcess(const void (*const a_frame), const void (*const b_frame), A_FRAME_INFO (*const a_frame_info), B_FRAME_INFO (*const b_frame_info));
  int DevicePurgeData				(void);

  //FIFO Write function
  int DeviceLoadNMRSetting (NMR_Config NMR_Setting);

  // Internal use only
  int DevicePurgeComm				(void);
  int SendReceive					(Ctrl_Packet* tx, Ctrl_Packet* rx);
  int ClientTCPWrite_ErrorCheck	(int txStatus);
  int ClientTCPRead_ErrorCheck	(int rxStatus);
  int DeviceWriteMask	(unsigned int address, unsigned int mask, unsigned int value);
  int DeviceReadMask	(unsigned int address, unsigned int mask, unsigned int* value);
  int DeviceNVWrite (unsigned int address, unsigned int value);
  int DeviceNVArrayWrite (unsigned int address, unsigned int size, unsigned int* data);
  int DeviceNVEraseSector (unsigned int address);
  int DeviceNVEraseBlock (unsigned int address);
  int DeviceNVEraseChip (unsigned int address);

  // Data Processing Functions:
  //Not yet defined

}
#endif
