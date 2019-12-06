#ifndef MSSM2017_HttSystematics_MSSMRun2_h
#define MSSM2017_HttSystematics_MSSMRun2_h
#include "CombineHarvester/CombineTools/interface/CombineHarvester.h"

namespace ch {
// Run2 SM analysis systematics
// Implemented in src/HttSystematics_SMRun2.cc
void AddMSSMRun2Systematics(CombineHarvester& cb, bool jetfakes, bool embedding, bool ggh_wg1, int era);
}

#endif
