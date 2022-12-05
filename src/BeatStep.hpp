#include "RtMidi.h"

class BeatStep {
  public:
    BeatStep() {
      try {
        this->midiout = new RtMidiOut();
      } catch ( RtMidiError &error ) {
        error.printMessage();
        exit( EXIT_FAILURE );
      }
    }
    
    ~BeatStep() {
      delete this->midiout;
    }

    void list () {
      unsigned int nPorts = this->midiout->getPortCount();
      std::string portName;
      std::cout << "\nThere are " << nPorts << " MIDI output ports available.\n";
      for ( unsigned int i=0; i<nPorts; i++ ) {
        try {
          portName = this->midiout->getPortName(i);
        }
        catch (RtMidiError &error) {
          error.printMessage();
          return;
        }
        std::cout << "\t#" << i+1 << ": " << portName << '\n';
      }
      std::cout << '\n';
    }
  private:
    RtMidiOut *midiout;
};
