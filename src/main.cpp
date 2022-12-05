#include <iostream>
#include <cstdlib>

#include "BeatStep.hpp"

int main() {
  BeatStep* bs = new BeatStep();
  bs->list();
  delete bs;
  return 0;
}
