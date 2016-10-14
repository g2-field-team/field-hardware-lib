#include <fstream>
#include "g2field/sis3316.hh"


int main(int argc, char *argv[])
{
  int trace_len = 100000;
  int num_ch = 16;

  // Open an output file.
  std::ofstream out;
  out.open("output/sis3316_test_data.csv");

  // Create the digitizer.
  hw::Sis3316 wfd("Test WFD", "/usr/local/etc/g2field/sis3316_0.json", trace_len);
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
