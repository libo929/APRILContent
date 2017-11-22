/**
 *  @file   ArborContent/src/ArborCheating/PerfectIsoHitRemovalAlgorithm.cc
 * 
 *  @brief  Implementation of the cheating clustering algorithm class
 * 
 *  $Log: $
 *  @author J. Marshall.
 *  Copied from https://github.com/PandoraPFA/LCContent sources
 */

#include "Pandora/AlgorithmHeaders.h"

#include "ArborCheating/PerfectIsoHitRemovalAlgorithm.h"

using namespace pandora;

namespace arbor_content
{

  PerfectIsoHitRemovalAlgorithm::PerfectIsoHitRemovalAlgorithm()
  {
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode PerfectIsoHitRemovalAlgorithm::Run()
  {
    const ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

    ClusterList localClusterList(pClusterList->begin(), pClusterList->end());
    MCParticleToClusterListMap mcParticleToClusterListMap;

    for (ClusterList::const_iterator clusterIter = localClusterList.begin(), clusterIterEnd = localClusterList.end(); 
			clusterIter != clusterIterEnd; ++clusterIter)
    {
      try
      {
        const Cluster *const pCluster = *clusterIter;

        if (!PandoraContentApi::IsAvailable(*this, pCluster))
          continue;

        this->SimpleMCParticleCollection(pCluster, mcParticleToClusterListMap);
      }
      catch (StatusCodeException &)
      {
      }
    }

	std::cout << "mcParticleToClusterListMap size: " << mcParticleToClusterListMap.size() << std::endl;
    this->MergeClusters(mcParticleToClusterListMap);

    return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void PerfectIsoHitRemovalAlgorithm::SimpleMCParticleCollection(const Cluster *const pCluster, 
		  MCParticleToClusterListMap &mcParticleToClusterListMap) const
  {
    const MCParticle *const pMCParticle(MCParticleHelper::GetMainMCParticle(pCluster));
    this->AddToClusterListMap(pCluster, pMCParticle, mcParticleToClusterListMap);
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void PerfectIsoHitRemovalAlgorithm::AddToClusterListMap(const Cluster *const pClusterToAdd, const MCParticle *const pMCParticle,
      MCParticleToClusterListMap &mcParticleToClusterListMap) const
  {
    MCParticleToClusterListMap::iterator iter(mcParticleToClusterListMap.find(pMCParticle));

    if (mcParticleToClusterListMap.end() == iter)
    {
      ClusterList *const pClusterList = new ClusterList();
      pClusterList->insert(pClusterToAdd);
      (void) mcParticleToClusterListMap.insert(MCParticleToClusterListMap::value_type(pMCParticle, pClusterList));
    }
    else
    {
      iter->second->insert(pClusterToAdd);
    }
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode PerfectIsoHitRemovalAlgorithm::MergeClusters(const MCParticleToClusterListMap &mcParticleToClusterListMap) const
  {
    for (MCParticleToClusterListMap::const_iterator mapIter = mcParticleToClusterListMap.begin(), 
	    mapIterEnd = mcParticleToClusterListMap.end(); mapIter != mapIterEnd; ++mapIter)
    {
      //const MCParticle *const pMCParticle = mapIter->first;
      ClusterList *const pClusterList = mapIter->second;

      if (pClusterList->size()>1)
      {
		int nChargedCluster = 0;
        const Cluster *mainCluster = NULL;

		for(ClusterList::const_iterator clusterIter = pClusterList->begin(); clusterIter != pClusterList->end(); ++clusterIter)
		{
			const Cluster* cluster = *clusterIter;

			const TrackList& associatedTracks = cluster->GetAssociatedTrackList();
			if(!associatedTracks.empty()) 
			{
				++nChargedCluster;
				mainCluster = cluster;
			}
		}

		// only deal with the case of one charged cluster (main cluster) while other are neutral
		if(nChargedCluster!=1) continue;

		for(ClusterList::const_iterator clusterIter = pClusterList->begin(); clusterIter != pClusterList->end(); ++clusterIter)
		{
			const Cluster* fragment = *clusterIter;
			if(fragment != mainCluster) PandoraContentApi::MergeAndDeleteClusters(*this, mainCluster, fragment);
		}
	  }

      delete pClusterList;
    }

#if 0
	std::string cluName("perfectClusters");
    if (!pClusterList->empty())
    {
      PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::SaveList<Cluster>(*this, cluName));
      PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::ReplaceCurrentList<Cluster>(*this, cluName));
    }
#endif

	return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode PerfectIsoHitRemovalAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
  {
#if 0
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadVectorOfValues(xmlHandle,
        "ParticleIdList", m_particleIdList));

    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
        "ShouldUseOnlyECalHits", m_shouldUseOnlyECalHits));

    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
        "ShouldUseIsolatedHits", m_shouldUseIsolatedHits));

    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
        "SimpleMCParticleCollection", m_simpleMCParticleCollection));

    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
        "MinWeightFraction", m_minWeightFraction));
#endif

    return STATUS_CODE_SUCCESS;
  }

}
