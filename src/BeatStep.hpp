#include "RtMidi.h"
#include <vector>
#include <fstream>
#include <iterator>
#include <json.hpp>

using json = nlohmann::json;

// Platform-dependent sleep routines.
#if defined(WIN32)
  #include <windows.h>
  #define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants
  #include <unistd.h>
  #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

enum BeatstepControls {
  BEATSTEP_CONTROLS_VOLUME = 0x30,
  BEATSTEP_CONTROLS_PLAY = 0x58,
  BEATSTEP_CONTROLS_STOP = 0x59,
  BEATSTEP_CONTROLS_SEQ = 0x5A,
  BEATSTEP_CONTROLS_SYNC = 0x5B,
  BEATSTEP_CONTROLS_RECALL = 0x5C,
  BEATSTEP_CONTROLS_STORE = 0x5D,
  BEATSTEP_CONTROLS_SHIFT = 0x5E,
  BEATSTEP_CONTROLS_CHAN = 0x5F
};

enum BeatstepScale {
  BEATSTEP_SCALES_CHROMATIC,
  BEATSTEP_SCALES_MAJOR,
  BEATSTEP_SCALES_MINOR,
  BEATSTEP_SCALES_DORIAN,
  BEATSTEP_SCALES_MIXOLYDIAN,
  BEATSTEP_SCALES_HARMONIC_MINOR,
  BEATSTEP_SCALES_BLUES,
  BEATSTEP_SCALES_USER
};

enum BeatstepSeqMode {
  BEATSTEP_SEQ_MODES_FORWARD,
  BEATSTEP_SEQ_MODES_REVERSE,
  BEATSTEP_SEQ_MODES_ALTERNATING,
  BEATSTEP_SEQ_MODES_RANDOM
};

enum BeatstepKnobAccelerationMode {
  BEATSTEP_KNOB_ACCELERATION_MODES_SLOW,
  BEATSTEP_KNOB_ACCELERATION_MODES_MEDIUM,
  BEATSTEP_KNOB_ACCELERATION_MODES_FAST
};

enum BeatstepPadVelocityMode {
  BEATSTEP_PADVELOCITY_MODES_LINEAR,
  BEATSTEP_PADVELOCITY_MODES_LOGARITHMIC,
  BEATSTEP_PADVELOCITY_MODES_EXPONENTIAL,
  BEATSTEP_PADVELOCITY_MODES_FULL
};

enum BeatstepStepSize {
  BEATSTEP_STEP_SIZES_STEP4,
  BEATSTEP_STEP_SIZES_STEP8,
  BEATSTEP_STEP_SIZES_STEP16,
  BEATSTEP_STEP_SIZES_STEP32
};

enum BeatstepLegatoMode {
  BEATSTEP_LEGATO_MODES_OFF,
  BEATSTEP_LEGATO_MODES_ON,
  BEATSTEP_LEGATO_MODES_RESET
};

enum BeatstepControllerMode {
  BEATSTEP_CONTROLLER_MODES_SILENT = 0x01,
  BEATSTEP_CONTROLLER_MODES_MMC = 0x07,
  BEATSTEP_CONTROLLER_MODES_CC = 0x08,
  BEATSTEP_CONTROLLER_MODES_NOTE = 0x09,
  BEATSTEP_CONTROLLER_MODES_PROGRAM = 0x0B
};

// named colors
enum BeatstepColor {
  BEATSTEP_COLORS_OFF = 0x00,
  BEATSTEP_COLORS_RED = 0x01,
  BEATSTEP_COLORS_PINK = 0x11,
  BEATSTEP_COLORS_BLUE = 0x10
};

enum BeatstepControllerBehavior {
  BEATSTEP_CONTROLLER_BEHAVIORS_TOGGLE,
  BEATSTEP_CONTROLLER_BEHAVIORS_GATE
};

class BeatStep {
  public:
    BeatStep () {
      try {
        this->midiout = new RtMidiOut();
        this->midiin = new RtMidiIn();
        // Don't ignore sysex messages.
        this->midiin->ignoreTypes(false, true, true);
      } catch ( RtMidiError &error ) {
        error.printMessage();
        exit( EXIT_FAILURE );
      }
    }
    
    ~BeatStep () {
      delete this->midiout;
      delete this->midiin;
    }

    // get a list of MIDI devices
    void list () {
      unsigned int nPorts = this->midiout->getPortCount();
      std::string portName;
      
      if (nPorts == 1) {
        std::cout << "\nThere is 1 MIDI output port available:\n";
      } else {
        std::cout << "\nThere are " << nPorts << " MIDI output ports available:\n";
      }

      for ( unsigned int i=0; i<nPorts; i++ ) {
        try {
          portName = this->midiout->getPortName(i);
        }
        catch (RtMidiError &error) {
          error.printMessage();
          return;
        }
        std::cout << '\t' << i+1 << ": " << portName << '\n';
      }
      
      std::cout << '\n';
    }

    void openPort(int device) {
      this->midiout->openPort(device);
      this->midiin->openPort(device);
    }

    // set a beatstep param
    void set (unsigned char pp, unsigned char cc, unsigned char vv) {
      std::vector<unsigned char> message = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x00, pp, cc, vv, 0xF7};
      this->midiout->sendMessage(&message);
      SLEEP(10);
    }

    // set the color of a pad's LED
    void color (unsigned char pad, BeatstepColor color) {
      this->set(0x10, pad, color);
    }

    // update firmware
    bool updateFirmware (std::string filename){
      std::ifstream input(filename, std::ios::binary);
      std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
      return true;
    }

    // get the firmware version on the device
    std::vector<unsigned char> version() {
      std::vector<unsigned char> message = { 0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7 };
      this->midiout->sendMessage(&message);

      SLEEP(10);
      message.clear();
      std::vector<unsigned char> version = {0,0,0,0};
      
      this->midiin->getMessage(&message);
      int nBytes = message.size();

      if (
        nBytes == 17 &&
        message[0] == 0xF0 &&
        message[1] == 0x7E &&
        message[2] == 0x00 &&
        message[3] == 0x06 &&
        message[4] == 0x02 &&
        message[5] == 0x00 &&
        message[6] == 0x20 &&
        message[7] == 0x6B &&
        message[8] == 0x02 &&
        message[9] == 0x00 &&
        message[10] == 0x06 &&
        message[11] == 0x00 &&
        message[16] == 0xF7
      ) {
        version[0] = message[15];
        version[1] = message[14];
        version[2] = message[13];
        version[3] = message[12];
      }
      
      return version;
    }

    // get a setting
    unsigned char get (unsigned char pp, unsigned char cc) {
      std::vector<unsigned char> message = { 0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x01, 0x00, pp, cc, 0xF7 };
      this->midiout->sendMessage(&message);
      int nBytes;
      int tryCount = 0;
      while (true) {
        tryCount++;
        SLEEP(10);
        message.clear();
        this->midiin->getMessage(&message);
        nBytes = message.size();
        if (
          nBytes == 12 &&
          message[0] == 0xF0 &&
          message[1] == 0x00 &&
          message[2] == 0x20 &&
          message[3] == 0x6B &&
          message[4] == 0x7F &&
          message[5] == 0x42 &&
          message[6] == 0x02 &&
          message[7] == 0x00 &&
          message[8] == pp &&
          message[9] == cc &&
          message[11] == 0xF7
        ) {
          return message[10];
        }
        if (tryCount > 10) {
          throw std::invalid_argument("No response.");
        }
      }
    }

    // save preset
    bool savePreset (std::string filename) {
      unsigned char cc;
      unsigned char pp;
      std::cout << '\n';
      for (cc = 96; cc < 154; cc++) {
        for (pp = 1; pp < 7; pp++) {
          std::cout << std::hex
            << "0x" << (int)cc << ' '
            << "0x" << (int)pp << ' '
            << "0x" << (int)this->get(pp, cc)
            << '\n';
        }
      }
      return true;
    }

    // load preset
    bool loadPreset (std::string filename){
      return true;
    }
  private:
    RtMidiOut *midiout;
    RtMidiIn *midiin;
};
