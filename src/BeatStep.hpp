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

void _midicallback(double deltatime, std::vector< unsigned char > *message, void *userData) {
  unsigned int nBytes = message->size();
  for ( unsigned int i=0; i<nBytes; i++ ){
    std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
  }
  if ( nBytes > 0 ){
    std::cout << "stamp = " << deltatime << std::endl;
  }
}

class BeatStep {
  public:
    BeatStep () {
      try {
        this->midiout = new RtMidiOut();
        this->midiin = new RtMidiIn();
        // Don't ignore sysex messages.
        this->midiin->ignoreTypes(false, true, true);
        this->midiin->setCallback(&_midicallback);
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

    unsigned char get (unsigned char pp, unsigned char cc) {
      std::vector<unsigned char> message = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x01, 0x00, pp, cc, 0xF7};
      this->midiout->sendMessage(&message);

      int nBytes, i;
      double stamp;
      
      std::cout << "Reading MIDI from port ... quit with Ctrl-C.\n";
      while ( !done ) {
        stamp = midiin->getMessage( &message );
        nBytes = message.size();
        for ( i=0; i<nBytes; i++ )
          std::cout << "Byte " << i << " = " << (int)message[i] << ", ";
        if ( nBytes > 0 )
          std::cout << "stamp = " << stamp << std::endl;
        SLEEP( 10 );
      }


      // TODO: process response
      return 0;
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

    // save preset
    bool savePreset (std::string filename){
      std::vector<unsigned char> message = { 0xF0, 0x7E, 0x00, 0x06, 0x02, 0x00, 0x20, 0x6B, 0x02, 0x00, 0x06, 0x00, 0x03, 0x00, 0x02, 0x01, 0xF7 };
      this->midiout->sendMessage(&message);
      this->midiout->sendMessage(&message);

      message = { 0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x00, 0x52, 0x00, 0x3C, 0xF7 };
      this->midiout->sendMessage(&message);

      unsigned char v = this->get(112, 1);
      v = this->get(112, 2);
      v = this->get(112, 3);
      v = this->get(112, 4);

      // TODO: read current buffer

      std::cout << "\nReading MIDI input ... press <enter> to quit.\n";
      char input;
      std::cin.get(input);

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
