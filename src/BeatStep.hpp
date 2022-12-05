#include "RtMidi.h"
#include <vector>

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

    // update firmware
    int updateFirmware(int device, std::string filename){
      return 0;
    }

    // save prest
    int saveBeatstep(int device, std::string filename){
      return 0;
    }


    // load prest
    int loadBeatstep(int device, std::string filename){
      return 0;
    }
  private:
    RtMidiOut *midiout;
};
