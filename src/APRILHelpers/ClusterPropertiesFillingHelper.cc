#include "APRILHelpers/ClusterPropertiesFillingHelper.h"
#include "APRILHelpers/ClusterHelper.h"
#include "APRILHelpers/ClusterPropertiesHelper.h"
#include "APRILObjects/CaloHit.h"

TNtuple* april_content::ClusterPropertiesFillingHelper::m_ntuple = NULL;

float april_content::ClusterPropertiesFillingHelper::m_minHitLayer          = 0.;
float april_content::ClusterPropertiesFillingHelper::m_clusterVol           = 0.;
float april_content::ClusterPropertiesFillingHelper::m_energyRatio          = 0.;
float april_content::ClusterPropertiesFillingHelper::m_hitOutsideRatio      = 0.;
float april_content::ClusterPropertiesFillingHelper::m_axisLengthRatio      = 0.;
float april_content::ClusterPropertiesFillingHelper::m_shortAxisLengthRatio = 0.;

april_content::ClusterPropertiesFillingHelper::~ClusterPropertiesFillingHelper() 
{
	if(m_ntuple != NULL ) delete m_ntuple;
}

april_content::ClusterPropertiesFillingHelper::ClusterPropertiesFillingHelper() 
{   
   m_ntuple = new TNtuple("Cluster","APRILCluster", 
		                  "MinHitLayer:ClusterVol:EnergyRatio:HitOutsideRatio:AxisLengthRatio:ShortAxisLengthRatio");
} 

pandora::StatusCode april_content::ClusterPropertiesFillingHelper::FillProperties(const pandora::Cluster* cluster)
{ 
	static ClusterPropertiesFillingHelper filler;

	ClusterPropertiesHelper::GetClusterProperties(cluster, m_minHitLayer, m_clusterVol, m_energyRatio, 
		                 m_hitOutsideRatio, m_axisLengthRatio, m_shortAxisLengthRatio);

	m_ntuple->Fill(m_minHitLayer, m_clusterVol, m_energyRatio, m_hitOutsideRatio, m_axisLengthRatio, m_shortAxisLengthRatio);

	//return pandora::STATUS_CODE_FAILURE;
	return pandora::STATUS_CODE_SUCCESS;
}
