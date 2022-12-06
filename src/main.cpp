#include <iostream>
#include <cstdlib>
#include "BeatStep.hpp"

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
  subLoad->add_option("FILE", filename, "The .beatstep file")->required();

  auto subSave = app.add_subcommand("save", "Save a .beatstep preset file from device");
  subSave->add_option("-d,--device", device, "The device to use (see list)");
  subSave->add_option("FILE", filename, "The .beatstep file")->required();

  auto subUpdate = app.add_subcommand("update", "Install a .led firmware file on device");
  subUpdate->add_option("-d,--device", device, "The device to use (see list)");
  subUpdate->add_option("FILE", filename, "The .led file")->required();

  int led = 0;
  std::string color = "red";
  auto subSeq = app.add_subcommand("color", "Change color of an LED");
  subSeq->add_option("LED", led, "The led to change color")->required();
  subSeq->add_option("COLOR", color, "The color to change it to (off, red, pink, blue)")->required();

  CLI11_PARSE(app, argc, argv);

  BeatStep* bs = new BeatStep();

  if (app.got_subcommand(subList)) {
    bs->list();
    return 0;
  }

  bool n = true;

  if (app.got_subcommand(subSeq)) {
    bs->openPort(device - 1);
    BeatstepColor c = BEATSTEP_COLORS_OFF;
    if (color == "red") {
      c = BEATSTEP_COLORS_RED;
    }
    if (color == "pink") {
      c = BEATSTEP_COLORS_PINK;
    }
    if (color == "blue") {
      c = BEATSTEP_COLORS_BLUE;
    }
    bs->color(0x70 + led, c);
  } else if (app.got_subcommand(subLoad)) {
    bs->openPort(device - 1);
    n = bs->loadPreset(filename);
  } else if (app.got_subcommand(subSave)) {
    bs->openPort(device - 1);
    n = bs->savePreset(filename);
  } else if (app.got_subcommand(subUpdate)) {
    bs->openPort(device - 1);
    n = bs->updateFirmware(filename);
  }

  delete bs;
  return n ? 0 : 1;
}
