#include <fstream>
#include "g2field/sis3350.hh"


int main(int argc, char *argv[])
{
  int trace_len = 1024;
  int num_ch = 4;

  // Open an output file.
  std::ofstream out;
  out.open("output/sis3350_test_data.csv");
  std::string wfd_conf("/usr/local/etc/g2field/sis3350_0.json");

  // Create the digitizer.
  hw::Sis3350 wfd("Test WFD", wfd_conf , trace_len);
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
