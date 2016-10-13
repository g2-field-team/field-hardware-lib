#include <fstream>
#include "g2field/sis3302.hh"


int main(int argc, char *argv[])
{
  int trace_len = 100000;

  // Open an output file.
  std::ofstream out;
  out.open("output/test_wfd.csv");

  // Create the digitizer.
  hw::Sis3302 wfd("Test WFD", "/usr/local/etc/g2field/sis3302_0.json", trace_len);
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
    for (int ch = 0; ch < 8; ++ch) {
      out << data.trace[ch][idx] << " ";
    }
    out << std::endl;
  }

  out.close();

  return 0;
}
