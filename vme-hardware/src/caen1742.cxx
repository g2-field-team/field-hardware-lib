#include "caen1742.hh"

namespace hw {

Caen1742::Caen1742(std::string name, std::string conf, int trace_len) :
  CommonBase(name), VmeBase(name, conf), WfdBase(name, conf, 18, trace_len)
{
  conf_file_ = conf;
  
  LoadConfig();
  LogMessage("worker created");
  
  read_len_ = trace_len;
  
  thread_live_ = true;

  StartThread();
  StartWorker();
}
  
void Caen1742::LoadConfig()
{ 
  // Open the configuration file.
  boost::property_tree::ptree conf;
  boost::property_tree::read_json(conf_file_, conf);
  
  int rc;
  uint msg = 0;
  std::string tmp;

  // Get the correction plan
  drs_cell_corrections_ = conf.get<bool>("drs_cell_corrections", true);
  drs_peak_corrections_ = conf.get<bool>("drs_peak_corrections", true);
  drs_time_corrections_ = conf.get<bool>("drs_time_corrections", true);

  // Get the base address for the device.  Convert from hex.
  tmp = conf.get<std::string>("base_address");
  base_address_ = std::stoul(tmp, nullptr, 0);

  // Get the board info.
  rc = Read(0xf034, msg);
  if (rc != 0) {

    LogError("could not find device");

  }

  // Board type
  if ((msg & 0xff) == 0x00) {
    
    LogMessage("Found caen v1742");
    
  } else if ((msg & 0xff) == 0x01) {
      
    LogMessage("Found caen vx1742");
  }

  // Check the serial number 
  int sn = 0;

  rc = Read(0xf080, msg);
  if (rc != 0) {
    LogError("failed to read high byte of serial number");
  }
  
  sn += (msg & 0xff) << 2;
  
  rc = Read(0xf084, msg);
  if (rc != 0) {
    LogError("failed to read lower byte of serial number");
  }

  sn += (msg & 0xff);
  LogMessage("Serial Number: %i", sn);
  
  // Get the hardware revision numbers.
  uint rev[4];
  
  for (int i = 0; i < 4; ++i) {

    rc = Read(0xf040 + 4*i, msg);
    if (rc != 0) {
      LogError("failed to read hardware revision byte %i", i);
    }

    rev[i] = msg;
  }

  LogMessage("Board Hardware Release %i.%i.%i.%i", 
	     rev[0], rev[1], rev[2], rev[3]);
  
  // Check the temperature.
  for (int i = 0; i < 4; ++i) {

    rc = Read(0x1088 + i*0x100, msg);
    if (rc != 0) {
      LogError("failed to read status of DRS4 chip %i", i);
    }

    if (msg & ((0x1 << 2) | (0x1 << 8)))
      LogDebug("unit %i is busy", i);

    rc = Read(0x10A0 + i*0x100, msg);
    if (rc != 0) {
      LogError("failed to read temperature of DRS4 chip %i", i);
    }

    LogMessage("DRS4 chip %i at temperature %i C", i, msg & 0xff);
  }

  // Software board reset.
  rc = Write(0xef24, msg);
  if (rc != 0) {
    LogError("failure attempt device reset");
  }

  // Reset needs time to finish, data drive number.
  usleep(300000);

  // Make certain we aren't running
  rc = Read(0x8104, msg);
  if (rc != 0) {
    LogError("failed to check device status");
  }
  
  if (msg & (0x1 << 2)) {

    LogWarning("Unit was already running on init");

    rc = Read(0x8100, msg);
    if (rc != 0) {
      LogError("failure reading register 0x8100");
    }

    // Preserve all bits but one.
    msg &= ~(0x1 << 2);

    rc = Write(0x8100, msg);
    if (rc != 0) {
      LogError("failure writing register 0x8100");
    }
  }
    
  // Set the group enable mask to all groups.
  rc = Read(0x8120, msg);
  if (rc != 0) {
    LogError("failed to read group enable mask");
  }

  rc = Write(0x8120, msg | 0xf);
  if (rc != 0) {
    LogError("failed to set group enable mask to 0xf");
  }

  // Enable digitization of the triggers using config bit enable register.
  rc = Write(0x8004, 0x1 << 11);
  if (rc != 0) {
    LogError("failed to enable digitization of triggers");
  }

  rc = Read(0x8120, msg);
  if (rc != 0) {
    LogError("failed to read group enable mask");
  } else {
    LogDebug("group enable mask reads: %08x", msg);
  }

  // Set the trace length.
  rc = Read(0x8020, msg);
  if (rc != 0) {
    LogError("failed to read device configuration");
  }
  
  rc = Write(0x8020, msg & 0xfffffffc); // 1024
  if (rc != 0) {
    LogError("failed to write device configuration for trace length");
  }

  // Set the sampling rate.
  double sampling_rate = conf.get<double>("sampling_rate", 1.0);
  sampling_setting_ = 0;

  if (sampling_rate < 1.75) {

    sampling_setting_ |= 0x2; // 1.0 Gsps
    LogMessage("sampling rate set to 1.0 Gsps");

  } else if (sampling_rate >= 1.75 && sampling_rate < 3.75) {

    sampling_setting_ |= 0x1; // 2.5 Gsps
    LogMessage("sampling rate set to 2.5 Gsps");
    
  } else if (sampling_rate >= 3.75) {

    sampling_setting_ |= 0x0; // 5.0 Gsps
    LogMessage("sampling rate set to 5.0 Gsps");
  }

  // Write the sampling rate.
  rc = Write(0x80d8, sampling_setting_);
  if (rc != 0) {
    LogError("failed to write the sampling rate");
  }

  // Set "pretrigger" buffer.
  rc = Read(0x8114, msg);
  if (rc != 0) {
    LogError("failed to read the pre-trigger buffer register");
  }

  int pretrigger_delay = conf.get<int>("pretrigger_delay", 35);
  msg &= 0xfffffe00;
  msg |= (uint)(pretrigger_delay / 100.0 * 0x3ff);

  rc = Write(0x8114, msg);
  if (rc != 0) {
    LogError("failed to set the pre-trigger buffer");
  }

  //DAC offsets
  uint ch = 0;
  uint ch_idx = 0;
  uint group_size = kNumAdcChannels / kNumAdcGroups;
  for (auto &val : conf.get_child("channel_offset")) {

    // Set group
    int group_idx = ch / group_size;

    // Convert the voltage to a dac value.
    float volts = val.second.get_value<float>();
    uint dac = (uint)((volts / vpp_) * 0xffff + 0x8000);
    
    if (dac < 0x0) dac = 0;
    if (dac > 0xffff) dac = 0xffff;
    
    // Make sure the board isn't busy
    int count = 0;
    while (true) {

      rc = Read(0x1088 + 0x100*group_idx, msg);
      if (rc != 0) {
        
	LogError("failed to read boad status while setting DAC");
	break;

      } else if (!(msg & 0x84)) {

	// This is what should happen
	break;

      } else if (count++ > 1000) {
        
	LogError("timed out polling busy after setting DAC, ch%i", ch);
	break;
      }
    }
    
    ch_idx = (ch++) % group_size;

    rc = Write(0x1098 + 0x100*group_idx, dac | (ch_idx << 16));
    if (rc != 0) {
      LogError("failed to select next DAC");
    }
  }    

  if (conf.get<bool>("write_correction_data_csv", false)) {
    WriteCorrectionDataCsv();
  }

  // Enable external/software triggers.
  rc = Read(0x810c, msg);
  if (rc != 0) {
    LogError("failed to read trigger register");
  }

  rc = Write(0x810c, msg | (0x3 << 30));
  if (rc != 0) {
    LogError("failed to enable external/software triggers");
  }

  // Set BLT Event Number to 1 for now
  rc = Read(0xef1c, msg);
  if (rc != 0) {
    LogError("failed to read BLT Event Number register");
  }

  rc = Write(0xef1c, 0x1);
  if (rc != 0) {
    LogError("failed to set BLT Event Number to 1");
  }

  // Set BERR enable for BLT transfers
  rc = Read(0xef00, msg);
  if (rc != 0) {
    LogError("failed to read VME control register");
  }

  rc = Write(0xef00, 0x10);
  //rc = Write(0xef00, 0x0);
  if (rc != 0) {
    LogError("failed to enable BERR in VME control register");
  }

  // Start acquiring events.
  int count = 0;
  do {

    rc = Read(0x8104, msg);
    if (rc != 0) {
      LogError("failed to read board acquiring status");
    }

    usleep(100);

  } while ((count++ < 100) && !(msg & 0x100));

  rc = Read(0x8100, msg);
  if (rc != 0) {
    LogError("failed to read register 0x8100");
  }

  msg |= (0x1 << 2);
  rc = Write(0x8100, msg);
  if (rc != 0) {
    LogError("failed to write register 0x8100");
  }

  usleep(1000);

  // Send a test software trigger and read it out.
  rc = Write(0x8108, msg);
  if (rc != 0) {
    LogError("failed to send test software trigger");
  }

  // Read initial empty event.
  LogDebug("eating first empty event");
  if (EventAvailable()) {
    wfd_data_t bundle;
    GetEvent(bundle);
  }

  LogMessage("LoadConfig finished");

} // LoadConfig

void Caen1742::WorkLoop()
{
  t0_ = std::chrono::high_resolution_clock::now();

  while (thread_live_) {

    while (go_time_) {

      if (EventAvailable()) {

	static wfd_data_t bundle;
	GetEvent(bundle);

	queue_mutex_.lock();
	data_queue_.push(bundle);
	has_event_ = true;
	queue_mutex_.unlock();

	LogDebug("read out new event");
	
      } else {
	
	std::this_thread::yield();
	usleep(hw::short_sleep);
      }
    }

    std::this_thread::yield();
    usleep(hw::long_sleep);
  }

  // Stop acquiring events.
  uint rc, msg;
  rc = Read(0x8100, msg);
  if (rc != 0) {
    LogError("failed to read register 0x8100");
  }

  msg &= ~(0x1 << 2);
  rc = Write(0x8100, msg);
  if (rc != 0) {
    LogError("failed to write register 0x8100");
  }
}


wfd_data_t Caen1742::PopEvent()
{
  static wfd_data_t data;
  queue_mutex_.lock();

  if (data_queue_.empty()) {
    queue_mutex_.unlock();

    wfd_data_t str;
    return str;

  } else if (!data_queue_.empty()) {

    // Copy the data.
    data = data_queue_.front();
    data_queue_.pop();
    
    // Check if this is that last event.
    if (data_queue_.size() == 0) has_event_ = false;
    
    queue_mutex_.unlock();
    return data;
  }
}


bool Caen1742::EventAvailable()
{
  // Check acquisition status regsiter.
  uint msg, rc;

  /*
  //ready flag in acq status
  rc = Read(0x8104, msg);
  if (rc != 0) {
  LogError("failed to read acqusition status register");
  }

  if (msg & (0x1 << 3)) {

  return true;

  } else {

  return false;
  }
  */

  //event stored register
  rc = Read(0x812c, msg);
  if (rc != 0) {
    //LogError("failed to read event stored register");
    return false;
  }

  if (msg > 0) { 
    //acq status readout ready flag
    rc = Read(0x8104, msg);
    if (rc != 0) {
      LogError("failed to read acqusition status register");
      return false;
    }

    if (msg & (0x1 << 3)) {
      return true;
    }
  }

  return false;
}

void Caen1742::GetEvent(wfd_data_t &bundle)
{
  using namespace std::chrono;

  // Make sure the bundle can handle the data.
  if (bundle.dev_clock.size() != num_ch_) {
    bundle.dev_clock.resize(num_ch_);
  }

  if (bundle.trace.size() != num_ch_) {
    bundle.trace.resize(num_ch_);
  }

  for (int i = 0; i < num_ch_; ++i) {
    bundle.trace[i].resize(1024);
  }

  int ch, rc = 0; 
  char *evtptr = nullptr;
  uint msg, d, size;
  int sample;
  std::vector<uint> startcells(4, 0);

  static std::vector<uint> buffer;
  buffer.reserve(0x10000);
  std::fill(buffer.begin(), buffer.end(), 0);

  // Get the system time
  auto t1 = high_resolution_clock::now();
  auto dtn = t1.time_since_epoch() - t0_.time_since_epoch();
  bundle.sys_clock = duration_cast<milliseconds>(dtn).count();  

  /*
  // Get the size of the next event data
  rc = Read(0x814c, msg);
  if (rc != 0) {
  LogError("failed to attain size of next event");
  return;
  }
  */
  
  /*
  //works OK at about 20 Hz  
  buffer.resize(msg);
  read_trace_len_read_ = msg;
  LogDebug("begin readout of event length: %i", msg);
  ReadTraceDma32Fifo(0x0, &buffer[0]);
  */
  
  read_len_ = buffer.capacity();
  LogDebug("begin readout of event length: %i", msg);
  rc = ReadTraceMblt64SameBlock(0x0, &buffer[0]);

  //rc > 0: number of words read
  //rc < 0: -retval;
  if (rc < 0) {
    std::fill(buffer.begin(), buffer.end(), 0);
    buffer.resize(0);
    return;
  }

  // std::cout << "num words read: " << rc << std::endl;
  // std::cout << "header: " << std::hex << buffer[0] << " ";
  // std::cout << buffer[1] << " " << buffer[2] << " " << std::endl;

  // Make sure we aren't getting empty events
  // if (buffer.size() < 5) {
  if (rc < 5) {
    return;
  }

  LogDebug("finished, element zero is %08x", buffer[0]);
  // Get the number of current events buffered.
  //rc = Read(0x812c, msg);
  //if (rc != 0) {
  //  LogError("failed to read current number of buffered events");
  //}

  //LogDebug("%i events in memory", msg);

  // Figure out the group mask
  bool grp_mask[kNumAdcGroups];

  for (int i = 0; i < kNumAdcGroups; ++i) {
    grp_mask[i] = buffer[1] & (0x1 << i);
  }
  
  // Now unpack the data for each group
  uint header;
  uint chdata[8];
  int start_idx = 4; // Skip main event header
  int stop_idx = 4;

  LogDebug("beginning to unpack data");
  for (int grp_idx = 0; grp_idx < kNumAdcGroups; ++grp_idx) {

    // Skip if this group isn't present.
    if (!grp_mask[grp_idx]) {
      LogWarning("Skipping group %i", grp_idx);
      continue;
    }

    // Grab the group header info.
    header = buffer[start_idx++];

    // Check to make sure it is a header
    if ((~header & 0xc00ce000) != 0xc00ce000) {
      LogWarning("Missed header");
    }

    // Calculate the group size.
    int data_size = header & 0xfff;
    int nchannels = kNumAdcChannels / kNumAdcGroups;
    bool trg_saved = header & (0x1 << 12);
    startcells[grp_idx] = (header >> 20) & 0x3ff;
    
    stop_idx = start_idx + data_size;
    sample = 0;

    LogDebug("start = %i, stop = %i, size = %u", start_idx, stop_idx, data_size);
    for (int i = start_idx; i < stop_idx; i += 3) {
      uint ln0 = buffer[i];
      uint ln1 = buffer[i+1];
      uint ln2 = buffer[i+2];
      
      chdata[0] = ln0 & 0xfff;
      chdata[1] = (ln0 >> 12) & 0xfff;
      chdata[2] = ((ln0 >> 24) & 0xff) | ((ln1 & 0xf) << 8);
      chdata[3] = (ln1 >> 4) & 0xfff;
      chdata[4] = (ln1 >> 16) & 0xfff;
      chdata[5] = ((ln1 >> 28) & 0xf) | ((ln2 & 0xff) << 4);
      chdata[6] = (ln2 >> 8) & 0xfff;
      chdata[7] = (ln2 >> 20) & 0xfff;

      for (int j = 0; j < 8; ++j) {
	int ch_idx = j + grp_idx * nchannels;
	bundle.trace[ch_idx][sample] = chdata[j];
      }
      
      ++sample;
    }

    // Update our starting point.
    start_idx = stop_idx;
    sample = 0;

    // Now grab the trigger if it was digitized.
    if (trg_saved) {
      
      LogDebug("Digitizing trigger");
      stop_idx = start_idx + (data_size / 8);

      for (int i = start_idx; i < stop_idx; i += 3) {
	uint ln0 = buffer[i];
	uint ln1 = buffer[i+1];
	uint ln2 = buffer[i+2];
	
	chdata[0] = ln0 & 0xfff;
	chdata[1] = (ln0 >> 12) & 0xfff;
	chdata[2] = ((ln0 >> 24) & 0xff) | ((ln1 & 0xf) << 8);
	chdata[3] = (ln1 >> 4) & 0xfff;
	chdata[4] = (ln1 >> 16) & 0xfff;
	chdata[5] = ((ln1 >> 28) & 0xf) | ((ln2 & 0xff) << 4);
	chdata[6] = (ln2 >> 8) & 0xfff;
	chdata[7] = (ln2 >> 20) & 0xfff;
	
	for (int j = 0; j < 8; ++j) {
	  bundle.trace[kNumAdcChannels + grp_idx][sample++] = chdata[j];
	}
      }
    }

    // Grab the trigger time and increment starting index.
    uint timestamp = buffer[stop_idx++];
    LogDebug("timestamp: 0x%08x\n", timestamp);

    start_idx = stop_idx;
  }

  if (true) {
    ApplyCorrection(bundle, startcells);
  }

  LogDebug("data readout complete");
}


// This function does the caen corrections directly as they do.
int Caen1742::ApplyCorrection(wfd_data_t &data, 
			      const std::vector<uint> &startcells)
{
  static bool correction_loaded = false;

  LogDebug("Applying data correction");

  if (!correction_loaded) {
    GetCorrectionData();
    correction_loaded = true;
  }

  if (drs_cell_corrections_) {
    CellCorrection(data, startcells);
  }

  if (drs_peak_corrections_) {
    PeakCorrection(data);
  }
  
  if (drs_time_corrections_) {
    TimeCorrection(data, startcells);
  }

  return 0;
}


int Caen1742::CellCorrection(wfd_data_t &data, 
			     const std::vector<uint> &startcells)
{
  LogDebug("running cell correction");

  uint i, j, k;
  short startcell;
  uint nchannels = kNumAdcChannels / kNumAdcGroups;
  
  for (i = 0; i < kNumAdcChannels; ++i) {

    startcell = startcells[i / nchannels];

    for (j = 0; j < kNumAdcSamples; ++j) {
      
      data.trace[i][j] -= correction_table_.cell[i][(startcell + j) % 1024];
      data.trace[i][j] -= correction_table_.nsample[i][j];
    }
  }

  return 0;
}


// todo: make this human readable.
int Caen1742::PeakCorrection(wfd_data_t &data)
{
  LogDebug("running drs peak correction");
  int offset;
  uint i, j, k;
  auto wf = data.trace; // Shortened to clean up logic notation.

  // Drop first sample automatically apparently.
  for (i = 0; i < kNumAdcChannels; ++i) wf[i][0] = wf[i][1];
  
  // Now check the other waveform indexes.
  for (i = 1; i < kNumAdcSamples; ++i) {

    for (j = 0; j < kNumAdcChannels; ++j) {

      // Reset if we are at the beginning of a new group.
      if (j % (kNumAdcChannels / kNumAdcGroups) == 0) offset = 0;

      LogDump("peak correction on cell[%i][%i]", j, i);

      switch(i) {
      
      case 1: 
	if (wf[j][i+1] - wf[j][i] > peakthresh) 
	  offset++;
	break;

      case 2:
	if ((wf[j][i+1] - wf[j][i-1] > peakthresh) && 
	    (wf[j][i+1] - wf[j][i] > peakthresh))
	  offset++;
	break;

      case kNumAdcSamples - 2:
	if (wf[j][i-1] - wf[j][i] > peakthresh) 
	  offset++;
	break;

      case kNumAdcSamples - 1:
	if (wf[j][i-1] - wf[j][i] > peakthresh) 
	  offset++;
	break;
	  
      default:
	if ((wf[j][i-1] - wf[j][i] > peakthresh) && 
	    ((wf[j][i+1] - wf[j][i] > peakthresh) || 
	     (wf[j][i+2] - wf[j][i] > peakthresh))) {
	  offset++;
	}
	break;
      }
    
      if (offset == (kNumAdcChannels / kNumAdcGroups)) {
	
	for (k = j - (kNumAdcChannels / kNumAdcGroups) + 1; k < j + 1; ++k) {

	  switch(i) {
	    
	  case 1:
	    wf[k][0] = wf[k][2];
	    wf[k][1] = wf[k][2];
	    break;
	    
	  case 2:
	    wf[k][0] = wf[k][3];
	    wf[k][1] = wf[k][3];
	    wf[k][2] = wf[k][3];
	    break;
	      
	  case kNumAdcSamples - 1:
	    wf[k][i] = wf[k][i-1];
	    break;

	  case kNumAdcSamples - 2:
	    wf[k][i] = wf[k][i-1];
	    wf[k][i+1] = wf[k][i-1];
	    break;
	      
	  default:
	    if (wf[k][i+1] - wf[k][i] > peakthresh) {

	      wf[k][i] = 0.5 * (wf[k][i+1] + wf[k][i-1]);

	    } else if (wf[k][i+2] - wf[k][i] > peakthresh) {

	      wf[k][i] = 0.5 * (wf[k][i+2] + wf[k][i-1]);
	      wf[k][i+1] = wf[k][i];
	    }		
	    break;
	  }
	} // k
      } // peak fix
    } // j
  } // i

  return 0;
}


int Caen1742::TimeCorrection(wfd_data_t &data, 
			     const std::vector<uint> &startcells)
{
  LogDebug("performing drs time correction");
  uint i, j, k, grp_idx;
  float t0, t1, dv, dt, vcorr, vrem;
  static float time[kNumAdcGroups][kNumAdcSamples];
  static float wf[kNumAdcSamples];
  static float sample_time = 0.0; // in ns
  static bool initialized = false;

  // Set up time if first time.
  if (!initialized) {

    if (sampling_setting_ == 0x0) {

      sample_time = 0.2;

    } else if (sampling_setting_ == 0x1) {
      
      sample_time = 0.4;

    } else {

      sample_time = 1.0;
    }

    for (i = 0; i < kNumAdcGroups; ++i) {

      // Set a initial time reference
      t0 = correction_table_.time[i][startcells[i] % kNumAdcSamples];
      time[i][0] = 0.0;

      for (j = 1; j < kNumAdcSamples; ++j) {

	dt = correction_table_.time[i][(startcells[i]+j) % kNumAdcSamples] - t0;
	
	if (dt > 0) {

	  time[i][j] = time[i][j-1] + dt;
	  LogDump("time[%i][%i] = %.4f", i, j, time[i][j]);

	} else {
	  
	  time[i][j] = time[i][j-1] + dt + kNumAdcSamples * sample_time;
	  LogDump("time[%i][%i] = %.4f", i, j, time[i][j]);
	}

	t0 = correction_table_.time[i][(startcells[i]+j) % kNumAdcSamples];
      }
    }

    initialized = true;
  }
      
  // Now do a linear interpolation to the correct time points.
  for (i = 0; i < kNumAdcChannels; ++i) {
    
    grp_idx = i / (kNumAdcChannels / kNumAdcGroups);
    vcorr = 0.0;
    vrem = 0.0;
    dv = 0.0;
    k = 0;

    wf[0] = data.trace[i][0];

    for (j = 1; j < kNumAdcSamples; ++j) {
      
      // Find the next sample in time order.
      while ((k < kNumAdcSamples - 2) && (time[grp_idx][k+1] < j * sample_time))
	++k;
      
      dv = data.trace[i][k+1] - data.trace[i][k];
      dt = time[grp_idx][k+1] - time[grp_idx][k];
      vcorr = (float) dv / dt * (j * sample_time - time[grp_idx][k]);
      LogDump("vcorr[%i][%i] = %.4f", i, j, vcorr);

      wf[j] = data.trace[i][k] + vcorr;
    }

    for (j = 0; j < kNumAdcSamples; ++j) {
      data.trace[i][j] = wf[j];
    }
  }    
}


int Caen1742::GetChannelCorrectionData(uint ch)
{
  int rc = 0;
  int count = 0;
  uint32_t d32 = 0;
  uint16_t d16 = 0;

  uint32_t group = 0;
  uint32_t pagenum = 0;
  int start = 0;
  int chunk = 256;
  int last = 0;
  short cell = 0;

  std::vector<int8_t> vec;

  // Set the page index
  group = ch / (kNumAdcChannels / kNumAdcGroups);
  pagenum = (group % 2) ? 0xc00 : 0x800;
  pagenum |= (sampling_setting_) << 8;
  pagenum |= (ch % (kNumAdcChannels / kNumAdcGroups)) << 2;
  
  LogMessage("channel %i pagenum = 0x%08x", ch, pagenum);

  // Get the cell corrections
  for (int i = 0; i < 4; ++i) {

    do {
      rc = ReadFlashPage(group, pagenum, vec);
      
    } while ((rc != 0) && (count++ < 100));
    
    if (rc != 0) {
      LogError("failed to load cell correction data for channel %u", i);
    }

    // Peak correction if needed.
    last = chunk;
    for (int j = start; j < start + chunk; ++j) {
      
      // No correction.
      if (vec[j - start] != 0x7f) {
	
	correction_table_.cell[ch][j] = vec[j - start];

      } else {
	
	cell = (short)((vec[last + 1] << 8) | vec[last]);
	
	if (cell == 0) {
	  
	  correction_table_.cell[ch][j] = vec[j - start];

	} else {
	
	  correction_table_.cell[ch][j] = cell;
	}

	last += 2;

	if (last > 263) {
	  last = 256; // This should never happen
	}
      }
    } // Peak correction

      // Increment for next round.
    start += chunk;
    pagenum++;
  }

  // Now get the nsample correction data.
  start = 0;

  // Calculate new pagenum via magic numbers
  pagenum &= 0xf00;
  pagenum |= 0x40;
  pagenum |= (ch % (kNumAdcChannels / kNumAdcGroups)) << 2;

  for (int i = 0; i < 4; ++i) {

    do {
      rc = ReadFlashPage(group, pagenum, vec);
      
    } while ((rc != 0) && (count++ < 100));
    
    if (rc != 0) {
      LogError("failed to load nsample correction data for channel %u", i);
    }
    
    for (int j = start; j < start + chunk; ++j) {
      
      correction_table_.nsample[ch][j] = vec[j - start];

    }

    start += chunk;
    pagenum++;
  }

  // Read the time correction if it's the first channel in the group.
  if (ch % (kNumAdcChannels / kNumAdcGroups) == 0) {
    
    LogDebug("loading time corrections for gr %i", ch % (kNumAdcSamples / kNumAdcGroups));
    pagenum &= 0xf00;
    pagenum |= 0xa0;

    start = 0;

    for (int i = 0; i < 16; ++i) {
      
      do {
	rc = ReadFlashPage(group, pagenum, vec);
      
      } while ((rc != 0) && (count++ < 100));
    
      if (rc != 0) {
	LogError("failed to load time correction data for channel %u", i);
      }

      std::copy((float *)&vec[0], 
		(float *)&vec[chunk], 
		&correction_table_.time[group][start]);

      start += chunk / 4;
      pagenum++;
    }
  }
}

int Caen1742::ReadFlashPage(uint32_t group, 
			    uint32_t pagenum, 
			    std::vector<int8_t> &page)
{
  LogDebug("reading flash page 0x%08x for group %i", pagenum, group);

  // Some basic variables
  int rc = 0;
  uint32_t d32 = 0;
  uint16_t d16 = 0;

  // Set some vme registers for convenience.
  uint32_t gr_status =  0x1088 | (group << 8);
  uint32_t gr_sel_flash = 0x10cc | (group << 8);
  uint32_t gr_flash = 0x10d0 | (group << 8);
  uint32_t flash_addr = pagenum << 9;

  page.resize(0);   // clear data
  page.resize(264); // magic resize

  rc = Read16(gr_status, d16);
  if (rc != 0) {
    return rc;
  }

  LogDebug("group %i status is 0x%04x", group, d16);

  // Enable the flash memory and tell it to read the main memory page.
  rc = Write16(gr_sel_flash, 0x1);
  if (rc != 0) {
    return rc;
  }
  
  rc = Write16(gr_flash, 0xd2);
  if (rc != 0) {
    return rc;
  }

  rc = Write16(gr_flash, (flash_addr >> 16) & 0xff);
  if (rc != 0) {
    return rc;
  }

  rc = Write16(gr_flash, (flash_addr >> 8) & 0xff);
  if (rc != 0) {
    return rc;
  }

  rc = Write16(gr_flash, (flash_addr) & 0xff);
  if (rc != 0) {
    return rc;
  }

  // Requires four more writes for no apparent reason.
  rc = Write16(gr_flash, 0x0);
  if (rc != 0) {
    return rc;
  }

  rc = Write16(gr_flash, 0x0);
  if (rc != 0) {
    return rc;
  }

  rc = Write16(gr_flash, 0x0);
  if (rc != 0) {
    return rc;
  }

  rc = Write16(gr_flash, 0x0);
  if (rc != 0) {
    return rc;
  }

  // Now read the data into the output vector.
  for (int i = 0; i < 264; ++i) {

    rc = Read(gr_flash, d32);
    if (rc != 0) {
      return rc;
    }

    page[i] = (int8_t)(d32 & 0xff);

    if (i == 0) {
      LogDebug("first value in pagenum 0x%08x is %i", pagenum, d32 & 0xff);
    }

    rc = Read(gr_status, d32);
    if (rc != 0) {
      return rc;
    }
  }

  // Disable the flash memory
  rc = Write16(gr_sel_flash, 0x0);
  if (rc != 0) {
    return rc;
  }

  LogDebug("done with flash page");
  return 0;
}


// This ended up being a silly division of labor, 
// might merge with GetChannelCorrectionData.
int Caen1742::GetCorrectionData()
{
  for (uint ch = 0; ch < kNumAdcChannels; ++ch) {
    GetChannelCorrectionData(ch);
  }
}


int Caen1742::WriteCorrectionDataCsv()
{
  // Make sure we have the correction loaded.
  GetCorrectionData();

  // unfinished, just printing for now
  std::ofstream out;
  out.open("correction_correction_table_.csv");
  
  for (int i = -1; i < 1024; ++i) {

    // Cell corrections
    for (int j = 0; j < kNumAdcChannels; ++j) {
      
      if (i == -1) {

	out << "cell_ch_" << j;

      }	else {

	out << correction_table_.cell[j][i];
	
      }

      out << ", ";

    } // cell

      // nsample corrections
    for (int j = 0; j < kNumAdcChannels; ++j) {
      
      if (i == -1) {

	out << "nsample_ch_" << j;

      }	else {

	out << (int)correction_table_.nsample[j][i];
	
      }

      out << ", ";

    } // nsample

      // timing corrections
    for (int j = 0; j < kNumAdcGroups; ++j) {
      
      if (i == -1) {

	out << "time_gr_" << i + 1;

      }	else {

	out << correction_table_.time[j][i];
	
      }

      if (j != kNumAdcGroups - 1) {

	out << ", ";

      } else {
	
	out << std::endl;

      }
    } // time
  } // i
}

} // ::daq
