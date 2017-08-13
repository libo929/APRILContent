// file SingleClusterIdAlgorithm.cc
/* 
 * Author: Bo Li (IPNL)
 *
 */

#include "ArborParticleId/SingleClusterIdAlgorithm.h"

#include "Pandora/AlgorithmHeaders.h"
#include "ArborHelpers/ClusterHelper.h"

#include "ArborHelpers/ClusterPropertiesFillingHelper.h"

namespace arbor_content
{
  pandora::StatusCode SingleClusterIdAlgorithm::Run()
  {
	if(m_fillCluster && m_fillPhoton) 
	{ 
		return pandora::STATUS_CODE_FAILURE;
	}

	///////
    const pandora::ClusterList *pClusterList = NULL;
    PandoraContentApi::GetList(*this, m_clusterListName, pClusterList);

	if((NULL!=pClusterList) && m_fillCluster)
	{
		FillCluster(pClusterList);
	}

	///////
    const pandora::ClusterList *pPhotonList = NULL;
    PandoraContentApi::GetList(*this, m_photonListName, pPhotonList);

	if((NULL!=pPhotonList) && m_fillPhoton)
	{
		FillPhotonCluster(pPhotonList);
	}

    return pandora::STATUS_CODE_SUCCESS;
  }

  pandora::StatusCode SingleClusterIdAlgorithm::FillCluster(const pandora::ClusterList* pClusterList)
  {
	// hadronic cluster
	if(NULL != pClusterList) 
	{
	   std::cout << "# of cluster: " << pClusterList->size() << std::endl;

	   const pandora::Cluster* maxCluster(NULL);
	   float maxClusterEnergy(0.);
	   float totalEnergy(0.);

       for(pandora::ClusterList::const_iterator iter = pClusterList->begin(), endIter = pClusterList->end();
           endIter != iter ; ++iter)
       {
	   	  const pandora::Cluster* pCluster = *iter;

	   	  float clusterEnergy = pCluster->GetElectromagneticEnergy();
		  std::cout << "cluster energy: " << clusterEnergy << std::endl;

		  totalEnergy += clusterEnergy;

		  if(clusterEnergy>maxClusterEnergy) 
		  { 
			 maxClusterEnergy = clusterEnergy;
			 maxCluster = pCluster;
		  }
       }

	   if(maxClusterEnergy/totalEnergy > m_minEnergyRatio) 
	   {
		  std::cout << "fill a cluster with energy: " << maxCluster->GetElectromagneticEnergy() << std::endl;
	   	  ClusterPropertiesFillingHelper::FillProperties(maxCluster);
	   }
	}

    return pandora::STATUS_CODE_SUCCESS;
  }

  pandora::StatusCode SingleClusterIdAlgorithm::FillPhotonCluster(const pandora::ClusterList* pPhotonList)
  {
	// photon cluster
	if(NULL != pPhotonList)
	{
	   std::cout << "# of photon cluster: " << pPhotonList->size() << std::endl;

       for(pandora::ClusterList::const_iterator iter = pPhotonList->begin(), endIter = pPhotonList->end();
           endIter != iter ; ++iter)
       {
	   	const pandora::Cluster* pPhotonCluster = *iter;

	   	ClusterPropertiesFillingHelper::FillProperties(pPhotonCluster);
	   	std::cout << "photon cluster energy: " << pPhotonCluster->GetElectromagneticEnergy() << std::endl;
       }
	}

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode SingleClusterIdAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ClusterListName", m_clusterListName));

    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "PhotonListName", m_photonListName));

    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "FillCluster", m_fillCluster));

    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "FillPhoton", m_fillPhoton));

    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinEnergyRatio", m_minEnergyRatio));

    return pandora::STATUS_CODE_SUCCESS;
  }
}
