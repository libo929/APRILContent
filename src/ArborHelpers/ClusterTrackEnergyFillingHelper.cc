#include "ArborHelpers/ClusterTrackEnergyFillingHelper.h"
//#include "ArborHelpers/ClusterHelper.h"
//#include "ArborHelpers/ClusterPropertiesHelper.h"
//#include "ArborObjects/CaloHit.h"

TNtuple* arbor_content::ClusterTrackEnergyFillingHelper::m_ntuple = NULL;

float arbor_content::ClusterTrackEnergyFillingHelper::m_cluster = 0.;
float arbor_content::ClusterTrackEnergyFillingHelper::m_track   = 0.;

arbor_content::ClusterTrackEnergyFillingHelper::~ClusterTrackEnergyFillingHelper() 
{
	if(m_ntuple != NULL ) delete m_ntuple;
}

arbor_content::ClusterTrackEnergyFillingHelper::ClusterTrackEnergyFillingHelper() 
{   
   m_ntuple = new TNtuple("ClusterTrackEnergy","ClusterTrackEnergy", "cluster:track");
} 

pandora::StatusCode arbor_content::ClusterTrackEnergyFillingHelper::FillEnergy(float cluster, float track)
{ 
	static ClusterTrackEnergyFillingHelper filler;

	m_ntuple->Fill(cluster, track);

	//return pandora::STATUS_CODE_FAILURE;
	return pandora::STATUS_CODE_SUCCESS;
}
