#include "sis3350.hh"

namespace hw {

Sis3350::Sis3350(std::string name, std::string conf, int trace_len) :
  CommonBase(name), VmeBase(name, conf), WfdBase(name, conf, 4, trace_len)
{
  // Grab the config file and load it.
  conf_file_ = conf;
  LoadConfig();

  // Set the read length for VME ReadTrace routine.
  read_len_ = trace_len_ / 2 + 4;

  StartThread();
  StartWorker();
}


void Sis3350::LoadConfig()
{
  LogDebug("configuring %s with file: %s", name_.c_str(), conf_file_.c_str());

  int rc = 0;
  uint msg = 0;

  // Open the configuration file.
  boost::property_tree::ptree conf;
  boost::property_tree::read_json(conf_file_, conf);

  // Get the device filestream.  If it isn't open, open it.
  std::string dev_path = conf.get<std::string>("device");

  // Get the base address.  Needs to be converted from hex.
  base_address_ = std::stoul(conf.get<std::string>("base_address"), nullptr, 0);

  // Check for device.
  rc = Read(0x0, msg);
  if (rc != 0) {

    LogError("failed to communicate with SIS3350 module.");

  } else {

    LogMessage("found %s, a SIS3350 at 0x%08x", name_.c_str(), base_address_);
  }

  // Reset device.
  rc = Write(0x400, 0x1);
  if (rc != 0) {
    LogError("failed to reset device");
  }

  // Get and print device ID.
  rc = Read(0x4, msg);
  if (rc != 0) {

    LogError("failed to read device ID register");

  } else {

    LogMessage("ID: %04x, maj rev: %02x, min rev: %02x",
               msg >> 16, (msg >> 8) & 0xff, msg & 0xff);
  }

  // Set and check the control/status register.
  msg = 0;

  if (conf.get<bool>("invert_ext_lemo")) {
    msg |= 0x10; // invert EXT TRIG
  }

  if (conf.get<bool>("user_led_on")) {
    msg |= 0x1; // LED on
  }

  msg = ((~msg & 0xffff) << 16) | msg; // j/k
  msg &= 0x00110011; // zero reserved bits

  rc = Write(0x0, msg);
  if (rc != 0) {
    LogError("failed to set control/status register");
  }

  rc = Read(0x0, msg);
  if (rc != 0) {
    LogError("failed to readback control/status register");
  }

  LogDebug("external trigger: %s", ((msg & 0x10) == 0x10) ? "NIM" : "TTL");
  LogDebug("user LED: %s", (msg & 0x1) ? "ON" : "OFF");

  // Set to the acquisition register.
  msg = 0x1;//sync ring buffer mode

  if (conf.get<bool>("enable_ext_lemo")) {
    msg |= 0x1 << 8; //enable EXT LEMO
  }

  msg = ((~msg & 0xffff) << 16) | msg; // j/k
  msg &= ~0xcc98cc98; //reserved bits

  rc = Write(0x10, msg);
  if (rc != 0) {
    LogError("failed to set acquisition register");
  }

  rc = Read(0x10, msg);
  if (rc != 0) {

    LogError("failed to readback acquisition register");

  } else {

    LogDebug("ACQ register set to: 0x%08x", msg);
  }

  // Set the synthesizer register.
  msg = 0x14; //500 MHz
  rc = Write(0x1c, msg);
  if (rc != 0) {
    LogError("failed to set synthesizer register");
  }

  // Set the memory page.
  msg = 0; //first 8MB chunk
  rc = Write(0x34, msg);
  if (rc != 0) {
    LogError("failed to set memory page");
  }

  // Set the trigger output.
  msg = 0x1;// LEMO IN -> LEMO OUT
  rc = Write(0x38, msg);
  if (rc != 0) {
    LogError("failed to enable lemo trigger output");
  }

  // Set ext trigger threshold.
  //first load data, then clock in, then ramp
  msg = 35000; // +1.45V TTL
  rc = Write(0x54, msg);
  if (rc != 0) {
    LogError("failed to set trigger threshold on DAC");
  }

  msg = 0;
  msg |= 0x1 << 4; //TRG threshold
  msg |= 0x1; //load shift register DAC

  rc = Write(0x50, msg);
  if (rc != 0) {
    LogError("failed to start loading threshold into DAC");
  }

  uint pollmax = 1000;
  uint count = 0;
  do {

    msg = 0;
    rc = Read(0x50, msg);
    if (rc != 0) {
      LogError("failure to read DAC status register");
    }

  } while (((msg & 0x8000) == 0x8000) && (count++ < pollmax));

  if (count >= pollmax) {
    LogError("Failure loading external trigger shift register");
  }

  msg = 0;
  msg |= 0x1 << 4; //TRG threshold
  msg |= 0x2; //load DAC
  rc = Write(0x50, msg);
  if (rc != 0) {
    LogError("failure to start loading DAC");
  }

  count = 0;
  do {

    msg = 0;
    rc = Read(0x50, msg);
    if (rc != 0) {
      LogError("failure to read DAC status register");
    }

  } while (((msg & 0x8000) == 0x8000) && (count++ < pollmax));

  if (count >= pollmax) {
    LogError("Failure loading external trigger to DAC");
  }

  //board temperature
  msg = 0;

  rc = Read(0x70, msg);
  if (rc != 0) {

    LogError("failed to read temperature for board");

  } else {

    LogDebug("board temperature: %.2f degC", (float)msg / 4.0);
  }

  //ring buffer sample length
  msg = trace_len_;
  rc = Write(0x01000020, msg);
  if (rc != 0) {
    LogError("failed to set ring buffer trace length");
  }

  //ring buffer pre-trigger sample length
  msg = std::stoi(conf.get<std::string>("pretrigger_samples"), nullptr, 0);
  rc = Write(0x01000024, msg);
  if (rc != 0) {
    LogError("failed to set ring buffer pre-trigger buffer");
  }

  //range -1.5 to +0.3 V
  uint ch = 0;
  //DAC offsets
  for (auto &val : conf.get_child("channel_offset")) {

    int offset = 0x02000050;
    offset |= (ch >> 1) << 24;

    float volts = val.second.get_value<float>();
    msg = (int)(33000 + 3377.77 * volts + 0.5);
    rc = Write(offset | 4, msg);
    if (rc != 0) {
      LogError("failed to write into DAC register");
    }

    msg = 0;
    msg |= (ch % 2) << 4;
    msg |= 0x1; //load shift register DAC
    rc = Write(offset, msg);
    if (rc != 0) {
      LogError("failed to start load to DAC shift register");
    }

    pollmax = 1000;
    count = 0;

    do {

      rc = Read(offset, msg);
      if (rc != 0) {
        LogError("failed to read DAC busy status");
      }
    } while (((msg & 0x8000) == 0x8000) && (count++ < pollmax));

    if (count == pollmax) {
      LogError("Failure loading ext trg shift reg");
    }

    msg = 0;
    msg |= (ch % 2) << 4;
    msg |= 0x2; //load DAC
    rc = Write(offset, msg);
    if (rc != 0) {
      LogError("failed to start load to DAC shift register");
    }

    count = 0;
    do {

      Read(offset, msg);
      if (rc != 0) {
        LogError("failed to read DAC busy status");
      }
    } while (((msg & 0x8000) == 0x8000) && (count++ < pollmax));

    if (count == pollmax) {
      LogError("Failure loading adc dac");
    }

    ++ch;
  }

  usleep(20000); // Magic sleep needed before setting gain.

  //gain - factory default 18 -> 5V
  ch = 0;
  for (auto &val : conf.get_child("channel_gain")) {
    //  for (ch = 0; ch < num_ch_; ch++) {
    msg = val.second.get_value<int>();

    int offset = 0x02000048;
    offset |= (ch >> 1) << 24;
    offset |= (ch % 2) << 2;
    rc = Write(offset, msg);
    if (rc != 0) {
      LogError("failed to set gain for channel %i", ch);
    }

    LogDebug("ADC %d gain %d", ch, msg);

    ++ch;
    usleep(20000);
  }

  uint armit = 1;
  rc = Write(0x410, armit);
  if (rc != 0) {
    LogError("failure to arm acquisition logic");
  }
} // LoadConfig


void Sis3350::WorkLoop()
{
  t0_ = std::chrono::high_resolution_clock::now();

  while (thread_live_) {

    // Grab the event if we have one.
    if (EventAvailable()) {

      static wfd_data_t bundle;

      GetEvent(bundle);

      queue_mutex_.lock();
      data_queue_.push(bundle);
      has_event_ = true;

      // Drop old events
      if (data_queue_.size() > kMaxQueueSize_)
	data_queue_.pop();

      queue_mutex_.unlock();

    } else {

      static auto dur1 = std::chrono::microseconds(hw::short_sleep);
      std::this_thread::sleep_for(dur1);
    }
    
    static auto dur2 = std::chrono::microseconds(hw::long_sleep);
    std::this_thread::sleep_for(dur2);
  }
}


wfd_data_t Sis3350::PopEvent()
{
  static wfd_data_t data;

  queue_mutex_.lock();

  if (data_queue_.empty()) {
    wfd_data_t str;

    queue_mutex_.unlock();
    return str;
  }

  // Copy the data.
  data = data_queue_.front();
  data_queue_.pop();

  // Check if this is that last event.
  if (data_queue_.size() == 0) has_event_ = false;

  queue_mutex_.unlock();
  return data;
}


bool Sis3350::EventAvailable()
{
  // Check acq reg.
  static uint msg = 0;
  static bool is_event;
  uint count = 0, rc = 0;

  do {

    rc = Read(0x10, msg);
    if (rc != 0) {
      LogError("failure to read acquisition status register");
    }
  } while ((rc != 0) && (count++ < 100));

  is_event = !(msg & 0x10000);

  // rearm the logic
  if (is_event) {

    count = 0;
    do {
      rc = Write(0x410, 0x1);
      if (rc != 0) {
        LogError("failure to rearm acquisition logic");
      }
    } while ((rc != 0) && (count++ < 100));

    LogMessage("found event");

    return is_event;

  } else {

    return false;
  }
}


// Pull the event.
void Sis3350::GetEvent(wfd_data_t &bundle)
{
  using namespace std::chrono;

  LogMessage("reading event data");

  // Make sure the bundle can handle the data.
  if (bundle.dev_clock.size() != num_ch_) {
    bundle.dev_clock.resize(num_ch_);
  }

  if (bundle.trace.size() != num_ch_) {
    bundle.trace.resize(num_ch_);
  }

  // Local variables.
  int ch, offset, rc = 0;
  bool is_event = true;

  uint next_sample_address[4] = {0, 0, 0, 0};

  for (ch = 0; ch < num_ch_; ch++) {

    offset = 0x02000010;
    offset |= (ch >> 1) << 24;
    offset |= (ch & 0x1) << 2;

    rc = Read(offset, next_sample_address[ch]);
    if (rc != 0) {
      LogError("failure to get next address for channel %i", ch);
    }
  }

  // Get the system time.
  auto t1 = high_resolution_clock::now();
  auto dtn = t1.time_since_epoch() - t0_.time_since_epoch();
  bundle.sys_clock = duration_cast<milliseconds>(dtn).count();

  //todo: check it has the expected length
  std::vector<uint> trace(read_len_, 0.0);

  for (ch = 0; ch < num_ch_; ch++) {

    offset = (0x4 + ch) << 24;

    rc = ReadTrace(offset, (uint *)&trace[0]);
    if (rc != 0) {
      LogError("failed to read trace for channel %i", ch);
    }
    
    // Copy the data to the vector.
    bundle.dev_clock[ch] = 0;
    bundle.dev_clock[ch] = trace[1] & 0xfff;
    bundle.dev_clock[ch] |= (trace[1] & 0xfff0000) >> 4;
    bundle.dev_clock[ch] |= (trace[0] & 0xfffULL) << 24;
    bundle.dev_clock[ch] |= (trace[0] & 0xfff0000ULL) << 20;

    // Make sure we can handle the data in.
    bundle.trace[ch].resize(trace_len_);

    uint idx;
    for (idx = 0; idx < trace_len_ / 2; idx++) {
      bundle.trace[ch][2 * idx] = trace[idx + 4] & 0xfff;
      bundle.trace[ch][2 * idx + 1] = (trace[idx + 4] >> 16) & 0xfff;
    }
  }
}

} // ::hw
