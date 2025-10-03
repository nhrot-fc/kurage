#include "engine/engine.h"
#include <unistd.h>

int main() {
  while(1) {
    engine_show_constants();
    sleep(1);
  }
  return 0;
}