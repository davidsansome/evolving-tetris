#include "utilities.h"

#include <sys/time.h>

namespace Utilities {

unsigned int RandomSeed() {
  timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec * tv.tv_sec;
}

} // namespace Utilities

