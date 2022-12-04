```
git clone --recursive https://github.com/konsumer/beatstep-cli.git
cd beatstep-cli

cmake -B rtmidi/build rtmidi
cmake --build rtmidi/build

cmake -B build
cmake --build build

./build/beatstep
```