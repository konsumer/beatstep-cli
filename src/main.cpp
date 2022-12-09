#include <iostream>
#include <cstdlib>
#include "BeatStep.hpp"

#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

BeatStep* bs;

void emulate_callback (double deltatime, std::vector< unsigned char > *message, void *userData) {
  unsigned int nBytes = message->size();
  for ( unsigned int i=0; i<nBytes; i++ )
    std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
  if ( nBytes > 0 )
    std::cout << "stamp = " << deltatime << std::endl;
}

int main(int argc, char *argv[]) {
  CLI::App app{"Use sysex to control BeatStep"};
  app.require_subcommand();

  int device = 1;
  std::string filename;

  app.add_option("-d,--device", device, "The device to use (see list)");

  auto subList = app.add_subcommand("list", "List available MIDI devices");
  
  auto subLoad = app.add_subcommand("load", "Load a .beatstep preset file on device");
  subLoad->add_option("FILE", filename, "The .beatstep file")->required();

  auto subSave = app.add_subcommand("save", "Save a .beatstep preset file from device");
  subSave->add_option("FILE", filename, "The .beatstep file")->required();

  // auto subUpdate = app.add_subcommand("update", "Install a .led firmware file on device");
  // subUpdate->add_option("-d,--device", device, "The device to use (see list)");
  // subUpdate->add_option("FILE", filename, "The .led file")->required();

  int led = 0;
  std::string color = "red";
  auto subColor = app.add_subcommand("color", "Change color of an LED");
  subColor->add_option("LED", led, "The pad-number to change color")->required();
  subColor->add_option("COLOR", color, "The color to change it to (off, red, pink, blue)")->required();

  auto subFw = app.add_subcommand("fw", "Get the firmware version on the device");

  int pp;
  int cc;
  int vv;
  bool intOut = false;

  auto subGet = app.add_subcommand("get", "Get a param-value");
  subGet->add_flag("-i,--int", intOut, "Output decimal value, instead of hex");
  subGet->add_option("PROGRAM", pp, "The number of the program")->required();
  subGet->add_option("CONTROL", cc, "The number of the control")->required();

  auto subSet = app.add_subcommand("set", "Set a param-value");
  subSet->add_option("PROGRAM", pp, "The number of the program")->required();
  subSet->add_option("CONTROL", cc, "The number of the control")->required();
  subSet->add_option("VALUE", vv, "The number of the value to set")->required();

  auto subEmu = app.add_subcommand("emulate", "Emulate a beatstep (for debugging)");


  CLI11_PARSE(app, argc, argv);

  bs = new BeatStep();

  if (app.got_subcommand(subList)) {
    bs->list();
    return 0;
  }

  bool n = true;

  if (app.got_subcommand(subColor)) {
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
    std::cout << "OK" << std::endl;
  } else if (app.got_subcommand(subFw)) {
    bs->openPort(device - 1);
    std::vector<unsigned char> v = bs->version();
    std::cout << (int)v[0] << '.' << (int)v[1] << '.' << (int)v[2] << '.' << (int)v[3] << std::endl;
  } else if (app.got_subcommand(subGet)) {
    bs->openPort(device - 1);
    int r = bs->get(pp, cc);
    if (intOut) {
      std::cout << r << std::endl;
    } else {
      std::cout << std::hex << "0x" << r << std::endl;
    }
  } else if (app.got_subcommand(subSet)) {
    bs->openPort(device - 1);
    bs->set(pp, cc, vv);
    std::cout << "OK" << std::endl;
  } else if (app.got_subcommand(subLoad)) {
    bs->openPort(device - 1);
    n = bs->loadPreset(filename);
    std::cout << "OK" << std::endl;
  } else if (app.got_subcommand(subSave)) {
    bs->openPort(device - 1);
    n = bs->savePreset(filename);
    std::cout << "OK" << std::endl;
  } else if (app.got_subcommand(subEmu)) {
    bs->midiout->openVirtualPort ("Arturia BeatStep");
    bs->midiin->openVirtualPort("Arturia BeatStep");
    bs->midiin->setCallback(&emulate_callback);

    std::cout << "A virtual device has been created. Press ENTER to stop." << std::endl;
    std::cin.get();
  }
  /*
  else if (app.got_subcommand(subUpdate)) {
    
    bs->openPort(device - 1);
    // TODO: get firmware version as a way to check it's in normal-mode before entering update-modde
    bs->updateMode();
    std::cout << "Your keyboard needs to be unplugged and plugged back in." << std::endl << "The lights will change." << std::endl << std::endl << "Press ENTER once you have done this." << std::endl;
    std::cin.get();
    bs->updateFirmware(filename);
    std::cout << "OK" << std::endl;
  }
  */

  delete bs;
  return n ? 0 : 1;
}
