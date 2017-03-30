#include <fstream>
#include "g2field/caen1742.hh"
#include "g2field/dio_trigger_board.hh"

int main(int argc, char *argv[])
{
  // WFD variables.
  int trace_len = 1024;
  int num_ch = 32;
  int num_read_attempts = 10;
  std::string wfd_name("caen_1742_2");
  std::string wfd_conf("config/caen1742_2.json");

  // Trigger variables
  auto trg_card(hw::BOARD_D);
  int trg_port = 2;
  int trg_mask = 0x1 << 7;

  // Output
  std::string outfile("output/caen1742_2_test_data.csv");

  // Open an output file.
  std::ofstream out;

  // Create the digitizer.
  hw::Caen1742 wfd(wfd_name, wfd_conf, trace_len);
  hw::wfd_data_t data;
  usleep(20e6);

  // Create the trigger board.
  hw::DioTriggerBoard trg(0x0, trg_card, trg_port, false);

  // Grab an event (well, try a few times anyway).
  for (int i = 0; i < num_read_attempts; ++i) {

    // Check if there is already a triggered event.
    if (wfd.DataAvailable()) {
      data = wfd.PopEvent();
      break;

    } else {
      // Try to trigger the digitizer and NMR pulser.
      //      trg.FireTriggers(trg_mask, 1000);
      //      usleep(200000);
      usleep(1000);
      trg.FireTriggers(trg_mask, 200000);
    }
  }

  if (data.trace.size() == 0) {
    std::cout << "No data to read out from WFD." << std::endl;
    return 1;
  }

  // Save the waveforms.
  out.open(outfile);

  for (int idx = 0; idx < trace_len; ++idx) {
    for (int ch = 0; ch < num_ch; ++ch) {
      out << data.trace[ch][idx] << " ";
    }
    out << std::endl;
  }

  out.close();

  return 0;
}
