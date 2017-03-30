#ifndef VME_HARDWARE_INCLUDE_CAEN1742_HH_
#define VME_HARDWARE_INCLUDE_CAEN1742_HH_

/*===========================================================================*\

  author: Matthias W. Smith
  email:  mwsmith2@uw.edu
  file:   caen1742.hh
  
  about:  Implements the core functionality of the CAEN 1742 device needed
          to operate.  It loads a config file for several
          settings, then launches a data gathering thread to poll for 
          triggered events.

\*===========================================================================*/

//--- std includes ----------------------------------------------------------//
#include <chrono>
#include <iostream>

//--- project includes ------------------------------------------------------//
#include "vme_base.hh"
#include "wfd_base.hh"


// This class pulls data from a caen_1742 device.
namespace hw {

class Caen1742 : public VmeBase, public WfdBase {

 public:
  
  // ctor
  Caen1742(std::string name, std::string conf, int trace_len);

  // Load parameters from a json file.
  // Example file:
  // {
  //     "name":"caen_6742_vec_0",
  //     "base_address": "0xD0000000",
  //     "sampling_rate":1.0,
  //     "pretrigger_delay":50,
  //     "drs_cell_corrections":true,
  //     "drs_peak_corrections":false,
  //     "drs_time_corrections":true,
  //     "channel_offset":[
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15,
  // 	     0.15
  //     ]
  // }
  void LoadConfig();

  // Thread that collects data.
  void WorkLoop();

  // Returns oldest event data to event builder/frontend.
  wfd_data_t PopEvent();

private:

  static constexpr double vpp_ = 1.0; // Scale of the device's voltage range
  const static ushort peakthresh = 30; // For peak corrections
  const static uint kNumAdcGroups = 2;
  const static uint kNumAdcChannels = 32;
  const static uint kNumAdcSamples = 1024;
  
  int device_;
  uint sampling_setting_;
  uint size_, bsize_;
  char *buffer_;
  bool drs_cell_corrections_;
  bool drs_peak_corrections_;
  bool drs_time_corrections_;

  std::chrono::high_resolution_clock::time_point t0_;

  typedef struct {
    int16_t cell[kNumAdcChannels][kNumAdcSamples];
    int8_t  nsample[kNumAdcChannels][kNumAdcSamples];
    float   time[kNumAdcGroups][kNumAdcSamples];
  } drs_correction;

  drs_correction correction_table_;

  std::string conf_file_;

  // Ask device whether it has data.
  bool EventAvailable();

  // If EventAvailable, read the data and add it to the queue.
  void GetEvent(wfd_data_t &bundle);

  // A function that runs through the three different DRS4 corrections
  // remove effects produce by imperfection in the domino sampling process.
  int ApplyCorrection(wfd_data_t &data, const std::vector<uint> &startcells);

  // Subtracts off an average inherent bias in the chip based on the sampling
  // start index in the domino ring cycle.
  int CellCorrection(wfd_data_t &data, const std::vector<uint> &startcells);

  // Check each channel for spikes above threshold and remove it if present
  // in all channels for a group.
  int PeakCorrection(wfd_data_t &data);

  // Interpolates values to on evenly spaced grid from the unevenly sampled
  // values reported by the DRS4
  int TimeCorrection(wfd_data_t &data, const std::vector<uint> &startcells);

  // Readout correction data from the board.
  int GetCorrectionData();

  // Readout individual channel's correction data.
  int GetChannelCorrectionData(uint ch);
 
  // Read a page of flash memory on the device, used in getting correction data
  int ReadFlashPage(uint32_t group, 
		    uint32_t pagenum, 
		    std::vector<int8_t> &page);

  void WaitForSpi(int group_index);

  // Optionally write out the correction data as a csv to observe.
  int WriteCorrectionDataCsv();

  int LoadCorrectionDataCsv(std::string fn);
};

} // ::daq

#endif
