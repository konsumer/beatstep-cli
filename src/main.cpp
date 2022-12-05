#include <iostream>
#include <cstdlib>
#include "BeatStep.hpp"

#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

void listDevices(){
  BeatStep* bs = new BeatStep();
  bs->list();
  delete bs;
}

int loadBeatstep(int device, std::string filename) {
  return 0;
}

int saveBeatstep(int device, std::string filename) {
  return 0;
}

int updateFirmware(int device, std::string filename) {
  return 0;
}

int main(int argc, char *argv[]) {
  CLI::App app{"Use sysex to control BeatStep"};
  app.require_subcommand();

  int device = 1;
  std::string filename;

  auto subList = app.add_subcommand("list", "List available MIDI devices");
  subList->final_callback(listDevices);
  
  auto subLoad = app.add_subcommand("load", "Load a .beatstep preset file into device");
  subLoad->add_option("-d,--device", device, "The device to use (see list)");
  subLoad->add_option("file", filename, "The .beatstep file")->required();

  auto subSave = app.add_subcommand("save", "Save a .beatstep preset file from device");
  subSave->add_option("-d,--device", device, "The device to use (see list)");
  subSave->add_option("file", filename, "The .beatstep file")->required();

  auto subUpdate = app.add_subcommand("update", "Install a .led firmware file on device");
  subUpdate->add_option("-d,--device", device, "The device to use (see list)");
  subUpdate->add_option("file", filename, "The .led file")->required();

  CLI11_PARSE(app, argc, argv);

  if (app.got_subcommand(subLoad)) {
    return loadBeatstep(device, filename);
  }

  if (app.got_subcommand(subSave)) {
    return saveBeatstep(device, filename);
  }

  if (app.got_subcommand(subUpdate)) {
    return updateFirmware(device, filename);
  }
}
