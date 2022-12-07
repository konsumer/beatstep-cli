#include "RtMidi.h"
#include <vector>
#include <fstream>
#include <iterator>

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
      SLEEP(100);
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
      // TODO: check other bytes
      version[0] = message[15];
      version[1] = message[14];
      version[2] = message[13];
      version[3] = message[12];
      return version;
    }

    // get a setting
    unsigned char get (unsigned char pp, unsigned char cc) {
      std::vector<unsigned char> message = { 0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x01, 0x00, pp, cc, 0xF7 };
      this->midiout->sendMessage(&message);
      SLEEP(10);
      message.clear();
      unsigned char out = 0;
      this->midiin->getMessage(&message);
      
      // TODO: check other bytes
      return message[10];
    }

    // save preset
    bool savePreset (std::string filename){
      std::vector<unsigned char> v = this->version();
      std::cout << (int)v[0] << '.' << (int)v[1] << '.' << (int)v[2] << '.' << (int)v[3] << '\n';

      unsigned char s;

      // TODO: do this in a loop

      s = this->get(0x52, 0x00);
      std::cout << (int)s << '\n';

      s = this->get(0x53, 0x00);
      std::cout << (int)s << '\n';

      s = this->get(0x50, 0x0A);
      std::cout << (int)s << '\n';

      s = this->get(0x52, 0x0A);
      std::cout << (int)s << '\n';

      s = this->get(0x53, 0x0A);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x70);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x70);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x70);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x70);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x70);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x70);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x71);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x71);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x71);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x71);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x71);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x71);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x72);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x72);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x72);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x72);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x72);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x72);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x73);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x73);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x73);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x73);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x73);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x73);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x74);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x74);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x74);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x74);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x74);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x74);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x75);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x75);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x75);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x75);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x75);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x75);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x76);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x76);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x76);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x76);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x76);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x76);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x77);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x77);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x77);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x77);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x77);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x77);
      std::cout << (int)s << '\n';

      s = this->get(0x50, 0x0B);
      std::cout << (int)s << '\n';

      s = this->get(0x52, 0x0B);
      std::cout << (int)s << '\n';

      s = this->get(0x53, 0x0B);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x78);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x78);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x78);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x78);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x78);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x78);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x79);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x79);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x79);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x79);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x79);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x79);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x7A);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x7A);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x7A);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x7A);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x7A);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x7A);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x7B);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x7B);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x7B);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x7B);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x7B);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x7B);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x7C);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x7C);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x7C);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x7C);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x7C);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x7C);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x7D);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x7D);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x7D);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x7D);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x7D);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x7D);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x7E);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x7E);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x7E);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x7E);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x7E);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x7E);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x7F);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x7F);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x7F);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x7F);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x7F);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x7F);
      std::cout << (int)s << '\n';

      s = this->get(0x50, 0x0C);
      std::cout << (int)s << '\n';

      s = this->get(0x52, 0x0C);
      std::cout << (int)s << '\n';

      s = this->get(0x53, 0x0C);
      std::cout << (int)s << '\n';

      s = this->get(0x52, 0x0D);
      std::cout << (int)s << '\n';

      s = this->get(0x53, 0x0D);
      std::cout << (int)s << '\n';

      s = this->get(0x52, 0x0E);
      std::cout << (int)s << '\n';

      s = this->get(0x53, 0x0E);
      std::cout << (int)s << '\n';

      s = this->get(0x52, 0x0F);
      std::cout << (int)s << '\n';

      s = this->get(0x53, 0x0F);
      std::cout << (int)s << '\n';

      s = this->get(0x50, 0x01);
      std::cout << (int)s << '\n';

      s = this->get(0x52, 0x01);
      std::cout << (int)s << '\n';

      s = this->get(0x53, 0x01);
      std::cout << (int)s << '\n';

      s = this->get(0x50, 0x02);
      std::cout << (int)s << '\n';

      s = this->get(0x52, 0x02);
      std::cout << (int)s << '\n';

      s = this->get(0x53, 0x02);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x20);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x20);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x20);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x20);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x20);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x20);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x21);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x21);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x21);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x21);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x21);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x21);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x22);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x22);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x22);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x22);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x22);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x22);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x23);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x23);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x23);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x23);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x23);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x23);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x24);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x24);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x24);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x24);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x24);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x24);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x25);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x25);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x25);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x25);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x25);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x25);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x26);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x26);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x26);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x26);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x26);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x26);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x27);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x27);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x27);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x27);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x27);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x27);
      std::cout << (int)s << '\n';

      s = this->get(0x41, 0x03);
      std::cout << (int)s << '\n';

      s = this->get(0x50, 0x03);
      std::cout << (int)s << '\n';

      s = this->get(0x52, 0x03);
      std::cout << (int)s << '\n';

      s = this->get(0x53, 0x03);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x28);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x28);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x28);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x28);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x28);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x28);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x29);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x29);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x29);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x29);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x29);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x29);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x2A);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x2A);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x2A);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x2A);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x2A);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x2A);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x2B);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x2B);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x2B);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x2B);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x2B);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x2B);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x2C);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x2C);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x2C);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x2C);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x2C);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x2C);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x2D);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x2D);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x2D);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x2D);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x2D);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x2D);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x2E);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x2E);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x2E);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x2E);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x2E);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x2E);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x2F);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x2F);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x2F);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x2F);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x2F);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x2F);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x30);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x30);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x30);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x30);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x30);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x30);
      std::cout << (int)s << '\n';

      s = this->get(0x41, 0x04);
      std::cout << (int)s << '\n';

      s = this->get(0x50, 0x04);
      std::cout << (int)s << '\n';

      s = this->get(0x52, 0x04);
      std::cout << (int)s << '\n';

      s = this->get(0x53, 0x04);
      std::cout << (int)s << '\n';

      s = this->get(0x50, 0x05);
      std::cout << (int)s << '\n';

      s = this->get(0x52, 0x05);
      std::cout << (int)s << '\n';

      s = this->get(0x53, 0x05);
      std::cout << (int)s << '\n';

      s = this->get(0x40, 0x06);
      std::cout << (int)s << '\n';

      s = this->get(0x50, 0x06);
      std::cout << (int)s << '\n';

      s = this->get(0x52, 0x06);
      std::cout << (int)s << '\n';

      s = this->get(0x53, 0x06);
      std::cout << (int)s << '\n';

      s = this->get(0x50, 0x07);
      std::cout << (int)s << '\n';

      s = this->get(0x52, 0x07);
      std::cout << (int)s << '\n';

      s = this->get(0x53, 0x07);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x58);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x58);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x58);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x58);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x58);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x58);
      std::cout << (int)s << '\n';

      s = this->get(0x01, 0x59);
      std::cout << (int)s << '\n';

      s = this->get(0x02, 0x59);
      std::cout << (int)s << '\n';

      s = this->get(0x03, 0x59);
      std::cout << (int)s << '\n';

      s = this->get(0x04, 0x59);
      std::cout << (int)s << '\n';

      s = this->get(0x05, 0x59);
      std::cout << (int)s << '\n';

      s = this->get(0x06, 0x59);
      std::cout << (int)s << '\n';

      s = this->get(0x50, 0x08);
      std::cout << (int)s << '\n';

      s = this->get(0x52, 0x08);
      std::cout << (int)s << '\n';

      s = this->get(0x53, 0x08);
      std::cout << (int)s << '\n';

      s = this->get(0x50, 0x09);
      std::cout << (int)s << '\n';

      s = this->get(0x52, 0x09);
      std::cout << (int)s << '\n';

      s = this->get(0x53, 0x09);
      std::cout << (int)s << '\n';

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
