#include <fstream>
#include "g2field/sis3302.hh"


int main(int argc, char *argv[])
{
  int trace_len = 100000;
  int num_ch = 8;

  // Open an output file.
  std::ofstream out;
  out.open("output/sis3302_test_data.csv");

  // Create the digitizer.
  std::string conf_file("sis3302_0.json");
  hw::Sis3302 wfd("Test WFD", conf_file, trace_len);
  hw::wfd_data_t data;

  // Grab an event (well, try a few times anyway).
  for (int i = 0; i < 10; ++i) {

    if (wfd.DataAvailable()) {
      data = wfd.PopEvent();
      break;

    } else {
      usleep(200000);
    }
  }

  for (int idx = 0; idx < trace_len; ++idx) {
    for (int ch = 0; ch < num_ch; ++ch) {
      out << data.trace[ch][idx] << " ";
    }
    out << std::endl;
  }

  out.close();

  return 0;
}
