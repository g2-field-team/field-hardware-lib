#include "sis3302.hh"
#include "common_extdef.hh" // Included only once, here.

namespace hw {

Sis3302::Sis3302(std::string name, std::string conf, int trace_len) :
  CommonBase(name), VmeBase(name, conf), WfdBase(name, conf, 8, trace_len)
{
  conf_file_ = conf;

  LoadConfig();
  LogMessage("worker created");

  read_len_ = trace_len / 2;
  has_event_ = false;
  generate_software_trigger_ = false;

  StartThread();
  StartWorker();
}

void Sis3302::LoadConfig()
{
  using std::string;

  int rc = 0, nerrors = 0;
  uint msg = 0;
  char str[256];

  LogMessage("configuring %s with file: %s", name_.c_str(), conf_file_.c_str());

  // Open the configuration file.
  boost::property_tree::ptree conf;
  boost::property_tree::read_json(conf_file_, conf);

  // Get the base address for the device.  Convert from hex.
  base_address_ = std::stoul(conf.get<string>("base_address"), nullptr, 0);

  // Read the base register.
  rc = Read(CONTROL_STATUS, msg);
  if (rc != 0) {

    LogError("could not find SIS3302 device at 0x%08x", base_address_);
    ++nerrors;

  } else {

    LogMessage("SIS3302 Found at 0x%08x", base_address_);
  }

  // Reset the device.
  rc = Write(KEY_RESET, 0x1);
  if (rc != 0) {
    LogError("failure writing sis3302 reset register");
    ++nerrors;
  }

  // Get device ID.
  rc = Read(MODID, msg);
  if (rc != 0) {

    LogError("failed reading device ID");
    ++nerrors;

  } else {

    LogMessage("ID: %04x, maj rev: %02x, min rev: %02x",
               msg >> 16, (msg >> 8) & 0xff, msg & 0xff);
  }

  LogMessage("setting the control/status register");
  msg = 0;

  if (conf.get<bool>("invert_ext_lemo")) {
    msg = 0x10; // invert EXT TRIG
  }

  if (conf.get<bool>("user_led_on")) {
    msg |= 0x1; // LED on
  }

  // Toggle reserved bits
  msg = ((~msg & 0xffff) << 16) | msg; // j/k
  msg &= ~0xfffefffe;

  rc = Write(CONTROL_STATUS, msg);
  if (rc != 0) {
    LogError("failed setting the control register");
    ++nerrors;
  }

  rc = Read(CONTROL_STATUS, msg);
  if (rc != 0) {

    LogError("failed reading status/control register");
    ++nerrors;

  } else {

    LogMessage("user LED is %s", (msg & 0x1) ? "ON" : "OFF");
  }

  LogMessage("setting the acquisition register");
  msg = 0;

  if (conf.get<bool>("enable_int_stop", true))
    msg |= 0x1 << 6; //enable internal stop trigger

  if (conf.get<bool>("enable_ext_lemo", true))
    msg |= 0x1 << 8; //enable external lemo

  // Set the clock source
  if (conf.get<bool>("enable_ext_clk", false)) {

    LogMessage("enabling external clock");
    msg |= (0x5 << 12);

  } else {

    clk_rate_ = conf.get<int>("int_clk_setting_MHz", 100);
    LogMessage("enabling internal clock");

    if (clk_rate_ > 100) {

      LogWarning("set point for internal clock too high, using 100MHz");
      clk_rate_ = 100.0;
      msg |= (0x0 << 12);

    } else if (clk_rate_ == 100) {

      msg |= (0x0 << 12);

    } else if ((clk_rate_ < 100) && (clk_rate_ > 50)) {

      LogWarning("set point for internal clock floored to 50Mhz");
      clk_rate_ = 50.0;
      msg |= (0x1 << 12);

    } else if (clk_rate_ == 50) {

      msg |= (0x1 << 12);

    } else if ((clk_rate_ < 50) && (clk_rate_ > 25)) {

      LogWarning("set point for internal clock floored to 25MHz");
      clk_rate_ = 25.0;
      msg |= (0x2 << 12);

    } else if (clk_rate_ == 25) {

      msg |= (0x2 << 12);

    } else if ((clk_rate_ < 25) && (clk_rate_ > 10)) {

      LogWarning("set point for internal clock floored to 10MHz");
      clk_rate_ = 10.0;
      msg |= (0x3 << 12);

    } else if (clk_rate_ == 10) {

      msg |= (0x3 << 12);

    } else if (clk_rate_ == 1) {

      msg |= (0x4 << 12);

    } else {

      LogWarning("set point for internal clock changed to 1MHz");
      clk_rate_ = 1.0;
      msg |= (0x4 << 12);
    }
  }

  msg = ((~msg & 0xffff) << 16) | msg; // j/k
  msg &= 0x7df07df0; // zero reserved bits / disable bits

  rc = Write(ACQUISITION_CONTROL, msg);
  if (rc != 0) {
    LogError("failed writing acquisition register");
    ++nerrors;
  }

  rc = Read(ACQUISITION_CONTROL, msg);
  if (rc != 0) {

    LogError("failed reading acquisition register");
    ++nerrors;

  } else {

    LogMessage("acquisition register set to: 0x%08x", msg);
  }

  LogMessage("setting start/stop delays");
  msg = conf.get<int>("start_delay", 0);

  rc = Write(START_DELAY, msg);
  if (rc != 0) {
    LogError("failed to set start delay");
    ++nerrors;
  }

  msg = conf.get<int>("stop_delay", 0);
  rc = Write(STOP_DELAY, msg);
  if (rc != 0) {
    LogError("failed to set stop delay");
    ++nerrors;
  }

  LogMessage("setting event configuration register");
  rc = Read(EVENT_CONFIG_ADC12, msg);
  if (rc != 0) {
    LogError("failed to read event configuration register");
    ++nerrors;
  }

  // Set event configure register with changes
  if (conf.get<bool>("enable_event_length_stop", true)) {
    msg |= 0x1 << 5;
  }

  rc = Write(EVENT_CONFIG_ALL_ADC, msg);
  if (rc != 0) {
    LogError("failed to set event configuration register");
    ++nerrors;
  }

  LogMessage("setting event length register and pre-trigger buffer");
  // @hack -> The extra 512 pads against a problem in the wfd.
  msg = (trace_len_ - 4 + 512) & 0xfffffc;
  rc = Write(SAMPLE_LENGTH_ALL_ADC, msg);
  if (rc != 0) {
    LogError("failed to set event length");
    ++nerrors;
  }

  // Set the pre-trigger buffer length.
  msg = std::stoi(conf.get<string>("pretrigger_samples", "0x0"), nullptr, 0);
  rc = Write(PRETRIGGER_DELAY_ALL_ADC, msg);
  if (rc != 0) {
    LogError("failed to set pre-trigger buffer");
    ++nerrors;
  }

  LogMessage("setting memory page");
  msg = 0; //first 8MB chunk

  rc = Write(ADC_MEMORY_PAGE, msg);
  if (rc != 0) {
    LogError("failed to set memory page");
    ++nerrors;
  }

  if (nerrors > 0) {
    LogMessage("configuration failed with %i errors, killing worker", nerrors);
    exit(-1);
  }
} // LoadConfig

void Sis3302::WorkLoop()
{
  // Dump first event (they are corrupted).
  if (EventAvailable()) {
    wfd_data_t tmp;
    GetEvent(tmp);
  }

  t0_ = std::chrono::high_resolution_clock::now();

  while (thread_live_) {

    while (go_time_) {

      if (generate_software_trigger_) {
        GenerateTrigger();
      }

      if (EventAvailable()) {

        static wfd_data_t bundle;
        GetEvent(bundle);

        queue_mutex_.lock();
        data_queue_.push(bundle);
        has_event_ = true;
        queue_mutex_.unlock();

      } else {

	std::this_thread::yield();
	usleep(hw::short_sleep);
      }
    }

    std::this_thread::yield();
    usleep(hw::long_sleep);
  }
}

wfd_data_t Sis3302::PopEvent()
{
  static wfd_data_t data;

  queue_mutex_.lock();

  if (data_queue_.empty()) {
    queue_mutex_.unlock();

    wfd_data_t str;
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


bool Sis3302::EventAvailable()
{
  // Check acq reg.
  static uint msg = 0;
  static bool is_event;
  static int count, rc;

  count = 0;
  rc = 0;
  do {

    rc = Read(ACQUISITION_CONTROL, msg);
    if (rc != 0) {
      LogError("failed to read event status register");
    }

    usleep(1);
  } while ((rc != 0) && (count++ < kMaxPoll));

  is_event = !(msg & 0x10000);

  if (is_event && go_time_) {
    // rearm the logic
    count = 0;
    rc = 0;

    do {

      rc = Write(KEY_ARM, 0x1);
      if (rc != 0) {
        LogError("failed to rearm sampling logic");
      }
    } while ((rc != 0) && (count++ < kMaxPoll));

    LogMessage("detected event and rearmed trigger logic");

    return is_event;
  }

  return false;
}

void Sis3302::GetEvent(wfd_data_t &bundle)
{
  using namespace std::chrono;
  int ch, offset, rc, count = 0;

  // Make sure the bundle can handle the data.
  if (bundle.dev_clock.size() != num_ch_) {
    bundle.dev_clock.resize(num_ch_);
  }

  if (bundle.trace.size() != num_ch_) {
    bundle.trace.resize(num_ch_);
  }

  uint next_sample_address[num_ch_];
  //  static trace = new uint[num_ch_][trace_len_ / 2];
  static uint timestamp[2];

  for (ch = 0; ch < num_ch_; ch++) {

    next_sample_address[ch] = 0;

    offset = 0x02000010;
    offset |= (ch >> 1) << 24;
    offset |= (ch & 0x1) << 2;

    count = 0;
    rc = 0;
    do {
      rc = Read(offset, next_sample_address[ch]);
      ++count;
    } while ((rc < 0) && (count < 100));
  }

  // Get the system time
  auto t1 = high_resolution_clock::now();
  auto dtn = t1.time_since_epoch() - t0_.time_since_epoch();
  bundle.sys_clock = duration_cast<milliseconds>(dtn).count();
  LogMessage("reading out event at time: %u", bundle.sys_clock);

  rc = Read(0x10000, timestamp[0]);
  if (rc != 0) {
    LogError("failed to read first byte of the device timestamp");
  }

  rc = Read(0x10001, timestamp[1]);
  if (rc != 0) {
    LogError("failed to read second byte of the device timestamp");
  }

  for (ch = 0; ch < num_ch_; ch++) {

    //decode the event (little endian arch)
    bundle.dev_clock[ch] = 0;
    bundle.dev_clock[ch] = timestamp[1] & 0xfff;
    bundle.dev_clock[ch] |= (timestamp[1] & 0xfff0000) >> 4;
    bundle.dev_clock[ch] |= (timestamp[0] & 0xfffULL) << 24;
    bundle.dev_clock[ch] |= (timestamp[0] & 0xfff0000ULL) << 20;

    offset = (0x8 + ch) << 23;
    count = 0;

    do {

      bundle.trace[ch].resize(trace_len_);
      rc = ReadTrace(offset, (uint *)&bundle.trace[ch][0]);
      if (rc != 0) {
        LogError("failed reading trace for channel %i", ch);
      }
    } while ((rc < 0) && (count++ < kMaxPoll));
  }


  bundle.trace_length = trace_len_;
  bundle.sampling_rate = clk_rate_;

  // for (ch = 0; ch < num_ch_; ch++) {

  //   std::copy((ushort *)trace[ch],
  //   	      (ushort *)trace[ch] + trace_len_,
  //   	      bundle.trace[ch]);
  // }
}


void Sis3302::GenerateTrigger() 
{
  int rc = Write(KEY_START, 0x1);
  if (rc != 0) {
    LogError("failed to generate internal trigger");
  }

  generate_software_trigger_ = false;
}

} // ::hw
