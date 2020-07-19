#ifndef HistogramManager_h 
#define HistogramManager_h 

#include <string>

#include "TNtuple.h"

class TDirectory;

using namespace std;

namespace april_content
{

class HistogramManager {

public:
    HistogramManager(const char * filename="Monitoring.root");
    virtual ~HistogramManager();

    // Create (if doesn't exist) and Fill a fixed bin histogram (TH1F or TH2F)
    static void CreateFill(string tupleName, string varNamelist, std::vector<float> varList);

private:
	//static std::vector<TNtuple*> tupleVec;
};

}

#endif 		// HistogramManager_h

