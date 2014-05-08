#ifndef CTRLROOM_UTIL_ROOT_LOADED
#define CTRLROOM_UTIL_ROOT_LOADED

#include <TSystem.h>

namespace ctrlroom {
// suppress the ROOT signal handlers (they interfere with DAQ!)
void root_suppress_signals() {
  gSystem->ResetSignal(kSigChild);
  gSystem->ResetSignal(kSigBus);
  gSystem->ResetSignal(kSigSegmentationViolation);
  gSystem->ResetSignal(kSigIllegalInstruction);
  gSystem->ResetSignal(kSigSystem);
  gSystem->ResetSignal(kSigPipe);
  gSystem->ResetSignal(kSigAlarm);
  gSystem->ResetSignal(kSigUrgent);
  gSystem->ResetSignal(kSigFloatingException);
  gSystem->ResetSignal(kSigWindowChanged);
}
}

#endif
