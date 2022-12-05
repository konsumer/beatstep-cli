#include <iostream>
#include <cstdlib>
#include "BeatStep.hpp"
#include <fstream>
#include <iterator>
#include <vector>

#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

int main(int argc, char *argv[]) {
  CLI::App app{"Use sysex to control BeatStep"};
  app.require_subcommand();

  int device = 1;
  std::string filename;

  auto subList = app.add_subcommand("list", "List available MIDI devices");
  
  auto subLoad = app.add_subcommand("load", "Load a .beatstep preset file on device");
  subLoad->add_option("-d,--device", device, "The device to use (see list)");
  subLoad->add_option("file", filename, "The .beatstep file")->required();

  auto subSave = app.add_subcommand("save", "Save a .beatstep preset file from device");
  subSave->add_option("-d,--device", device, "The device to use (see list)");
  subSave->add_option("file", filename, "The .beatstep file")->required();

  auto subUpdate = app.add_subcommand("update", "Install a .led firmware file on device");
  subUpdate->add_option("-d,--device", device, "The device to use (see list)");
  subUpdate->add_option("file", filename, "The .led file")->required();

  CLI11_PARSE(app, argc, argv);

  BeatStep* bs = new BeatStep();

  if (app.got_subcommand(subList)) {
    bs->list();
    return 0;
  }

  int n = 0;

  if (app.got_subcommand(subLoad)) {
    n = bs->loadBeatstep(device, filename);
  }else if (app.got_subcommand(subSave)) {
    n = bs->saveBeatstep(device, filename);
  }else if (app.got_subcommand(subUpdate)) {
    n = bs->updateFirmware(device, filename);
  }

  delete bs;
  return n;
}
