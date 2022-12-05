#include <iostream>
#include <cstdlib>
#include "BeatStep.hpp"
#include "cxxopts.hpp"

int main(int argc, char *argv[]) {
  cxxopts::Options options("beatstep", "Control your BeatStep with sysex");

  options
    .positional_help("COMMAND")
    .show_positional_help();

  options.add_options()
    // ("b,bar", "Param bar", cxxopts::value<std::string>())
    // ("d,debug", "Enable debugging", cxxopts::value<bool>()->default_value("false"))
    ("d,device", "The MIDI device to use", cxxopts::value<int>()->default_value("1"))
    ("h,help", "Print usage")
  ;

  options.add_options("list")
    ("", "List available MIDI-out devices", cxxopts::value<std::vector<std::string>>())
  ;

  options.parse_positional({"list"});

  auto result = options.parse(argc, argv);

  if (result.count("help")){
    std::cout << options.help() << std::endl;
    return EXIT_SUCCESS;
  }

  BeatStep* bs = new BeatStep();
  bs->list();
  delete bs;
  return EXIT_SUCCESS;
}
