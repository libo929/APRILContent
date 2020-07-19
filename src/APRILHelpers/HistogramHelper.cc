#include "APRILHelpers/HistogramHelper.h"

#include "TDirectory.h"
#include "TH2F.h"
#include "TNtuple.h"

#include <iostream>

using namespace std;

namespace april_content
{

//______________________________________________________________________________
HistogramManager::HistogramManager(const char * /* filename */)
{
}

//______________________________________________________________________________
HistogramManager::~HistogramManager()
{
	//std::cout << "deleting HistogramManager..." << std::endl;
	// the tuple is written in file, so we don't need to delete them

#if 0
	for(int i=0; i<tupleVec.size(); ++i)
	{
		auto ptr = tupleVec.at(i);
		delete ptr;
	}
#endif
}

//______________________________________________________________________________
void HistogramManager::CreateFill(string tupleName, string varNamelist, std::vector<float> varList)
{
    string hname = tupleName;

	//std::cout << " hname: " << hname << std::endl;
	TNtuple* ptr = (TNtuple*) gDirectory->Get( hname.c_str() );      

	if(ptr == NULL )
	{
		ptr = new TNtuple(tupleName.c_str(), tupleName.c_str(), varNamelist.c_str());

		// not needed
		// tupleVec.push_back(ptr);
	}
	else
	{
		//std::cout << tupleName << " is here: " << ptr << std::endl;
	}

	float* listToFill = &varList[0];
	ptr->Fill(listToFill);
}

}
