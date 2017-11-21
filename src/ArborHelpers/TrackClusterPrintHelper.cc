#include "ArborHelpers/TrackClusterPrintHelper.h"
#include "ArborApi/ArborContentApi.h"
#include "Objects/Track.h"


pandora::StatusCode arbor_content::TrackClusterPrintHelper::PrintCluster(const pandora::ClusterList *pClusterList)
{
	if(NULL == pClusterList) return pandora::STATUS_CODE_SUCCESS;

    pandora::ClusterVector clusterVector(pClusterList->begin(), pClusterList->end());

    for(pandora::ClusterVector::iterator iter = clusterVector.begin(), endIter = clusterVector.end() ;
        endIter != iter ; ++iter)
    {
      const pandora::Cluster *const pCluster = *iter;

	  std::cout << "----> Get cluster energy: " << pCluster->GetElectromagneticEnergy() << std::endl;
      const pandora::TrackList &trackList(pCluster->GetAssociatedTrackList());
	  if(trackList.empty() || trackList.size()>1) 
	  {
		  std::cout << "   --->no associatied track " << std::endl;
		  continue;
	  }

	  const pandora::Track* track = *(trackList.begin());
	  std::cout << "   ---track energy: " << track->GetEnergyAtDca() << std::endl;
	}

    return pandora::STATUS_CODE_SUCCESS;
}
