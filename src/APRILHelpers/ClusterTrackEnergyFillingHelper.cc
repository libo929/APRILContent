#include "APRILHelpers/ClusterTrackEnergyFillingHelper.h"
//#include "APRILHelpers/ClusterHelper.h"
//#include "APRILHelpers/ClusterPropertiesHelper.h"
//#include "APRILObjects/CaloHit.h"

TNtuple* april_content::ClusterTrackEnergyFillingHelper::m_ntuple = NULL;

float april_content::ClusterTrackEnergyFillingHelper::m_cluster = 0.;
float april_content::ClusterTrackEnergyFillingHelper::m_track   = 0.;

april_content::ClusterTrackEnergyFillingHelper::~ClusterTrackEnergyFillingHelper() 
{
	if(m_ntuple != NULL ) delete m_ntuple;
}

april_content::ClusterTrackEnergyFillingHelper::ClusterTrackEnergyFillingHelper() 
{   
   m_ntuple = new TNtuple("ClusterTrackEnergy","ClusterTrackEnergy", "cluster:track");
} 

pandora::StatusCode april_content::ClusterTrackEnergyFillingHelper::FillEnergy(float cluster, float track)
{ 
	static ClusterTrackEnergyFillingHelper filler;

	m_ntuple->Fill(cluster, track);

	//return pandora::STATUS_CODE_FAILURE;
	return pandora::STATUS_CODE_SUCCESS;
}
