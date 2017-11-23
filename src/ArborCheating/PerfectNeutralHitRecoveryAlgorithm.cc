/**
 *  @file   ArborContent/src/ArborCheating/PerfectNeutralHitRecoveryAlgorithm.cc
 * 
 *  @brief  Implementation of the cheating clustering algorithm class
 * 
 *  $Log: $
 *  @author J. Marshall.
 *  Copied from https://github.com/PandoraPFA/LCContent sources
 */

#include "Pandora/AlgorithmHeaders.h"

#include "ArborCheating/PerfectNeutralHitRecoveryAlgorithm.h"

using namespace pandora;

namespace arbor_content
{

  PerfectNeutralHitRecoveryAlgorithm::PerfectNeutralHitRecoveryAlgorithm()
  {
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode PerfectNeutralHitRecoveryAlgorithm::Run()
  {
    const CaloHitList *pCaloHitList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList));

    CaloHitList localCaloHitList(pCaloHitList->begin(), pCaloHitList->end());
    MCParticleToCaloHitListMap mcParticleToCaloHitListMap;
	int nMissingHits = 0;
	float energyMissingHits = 0.;

    for (CaloHitList::const_iterator caloHitIter = localCaloHitList.begin(), caloHitIterEnd = localCaloHitList.end(); 
			caloHitIter != caloHitIterEnd; ++caloHitIter)
    {
      try
      {
        const CaloHit *const pCaloHit = *caloHitIter;

        if (!PandoraContentApi::IsAvailable(*this, pCaloHit))
          continue;

		energyMissingHits += pCaloHit->GetHadronicEnergy();
		++nMissingHits;

        this->SimpleMCParticleCaloHitListCollection(pCaloHit, mcParticleToCaloHitListMap);
      }
      catch (StatusCodeException &)
      {
      }
    }

	// map of MCP and cluster
    const ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

    ClusterList localClusterList(pClusterList->begin(), pClusterList->end());
    MCParticleToClusterMap mcParticleToClusterMap;

    for (ClusterList::const_iterator clusterIter = localClusterList.begin(), clusterIterEnd = localClusterList.end(); 
			clusterIter != clusterIterEnd; ++clusterIter)
	{
		const Cluster *const pCluster = *clusterIter;
		//std::cout << "cluster: " << pCluster << std::endl;
        this->SimpleMCParticleClusterCollection(pCluster, mcParticleToClusterMap);
	}


    if(mcParticleToCaloHitListMap.empty() == false && mcParticleToClusterMap.empty() == false)
	{ 
		this->MergeCaloHits(mcParticleToCaloHitListMap, mcParticleToClusterMap);
	}

	std::cout << "======== mcParticleToCaloHitListMap size: " << mcParticleToCaloHitListMap.size() << std::endl;
	std::cout << "======== mcParticleToClusterMap size: " << mcParticleToClusterMap.size() << std::endl;
	std::cout << "============= missing hits : " << nMissingHits << ", energy: " <<  energyMissingHits << std::endl;

    return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void PerfectNeutralHitRecoveryAlgorithm::SimpleMCParticleCaloHitListCollection(const CaloHit *const pCaloHit, 
		  MCParticleToCaloHitListMap &mcParticleToCaloHitListMap) const
  {
	try
	{
       const MCParticle *const pMCParticle(MCParticleHelper::GetMainMCParticle(pCaloHit));
       this->AddToCaloHitListMap(pCaloHit, pMCParticle, mcParticleToCaloHitListMap);
	}
    catch (StatusCodeException &)
    {
    }
  }

  //------------------------------------------------------------------------------------------------------------------------------------------
  
  void PerfectNeutralHitRecoveryAlgorithm::SimpleMCParticleClusterCollection(const Cluster *const pCluster, 
		  MCParticleToClusterMap &mcParticleToClusterMap) const
  {
	try
	{
       const MCParticle *const pMCParticle(MCParticleHelper::GetMainMCParticle(pCluster));
       this->AddToClusterMap(pCluster, pMCParticle, mcParticleToClusterMap);
	}
    catch (StatusCodeException &)
    {
    }
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void PerfectNeutralHitRecoveryAlgorithm::AddToCaloHitListMap(const CaloHit *const pCaloHitToAdd, const MCParticle *const pMCParticle,
      MCParticleToCaloHitListMap &mcParticleToCaloHitListMap) const
  {
    MCParticleToCaloHitListMap::iterator iter(mcParticleToCaloHitListMap.find(pMCParticle));

    if (mcParticleToCaloHitListMap.end() == iter)
    {
      CaloHitList *const pCaloHitList = new CaloHitList();
      pCaloHitList->insert(pCaloHitToAdd);
      (void) mcParticleToCaloHitListMap.insert(MCParticleToCaloHitListMap::value_type(pMCParticle, pCaloHitList));
    }
    else
    {
      iter->second->insert(pCaloHitToAdd);
    }
  }

  //------------------------------------------------------------------------------------------------------------------------------------------
  
  void PerfectNeutralHitRecoveryAlgorithm::AddToClusterMap(const Cluster *const pClusterToAdd, const MCParticle *const pMCParticle,
      MCParticleToClusterMap &mcParticleToClusterMap) const
  {
	//std::cout << "     - mcp: " << pMCParticle << std::endl;
    if (mcParticleToClusterMap.find(pMCParticle) == mcParticleToClusterMap.end() )
    {
      mcParticleToClusterMap.insert(MCParticleToClusterMap::value_type(pMCParticle, pClusterToAdd));
	  //std::cout << "      added cluster: " << pClusterToAdd << ", mcp: " << pMCParticle << std::endl;
    }
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode PerfectNeutralHitRecoveryAlgorithm::MergeCaloHits(const MCParticleToCaloHitListMap &mcParticleToCaloHitListMap,
		                                                  const MCParticleToClusterMap &mcParticleToClusterMap) const
  {
    for (MCParticleToCaloHitListMap::const_iterator mapIter = mcParticleToCaloHitListMap.begin(), 
	    mapIterEnd = mcParticleToCaloHitListMap.end(); mapIter != mapIterEnd; ++mapIter)
    {
      const MCParticle *const pMCParticle = mapIter->first;
      CaloHitList *const pCaloHitList = mapIter->second;

      if (pCaloHitList->size()>0)
      {
		// get the cluster which will merge hits by MCP
		MCParticleToClusterMap::const_iterator iter(mcParticleToClusterMap.find(pMCParticle));
		if(iter == mcParticleToClusterMap.end()) continue;

		const Cluster* const pCluster = iter->second;

		for(CaloHitList::const_iterator caloHitIter = pCaloHitList->begin(); caloHitIter != pCaloHitList->end(); ++caloHitIter)
		{
			const CaloHit* const caloHit = *caloHitIter;

			// only for neutral particle ?

			//std::cout << "++++++ Add hit to cluster: " << pCluster << ", " << caloHit << std::endl;
			//std::cout << pCluster->GetHadronicEnergy() << std::endl;
			PandoraContentApi::AddIsolatedToCluster(*this, pCluster, caloHit);
			//std::cout << "  adding done..." << std::endl;
		}
	  }

      delete pCaloHitList;
    }

	return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode PerfectNeutralHitRecoveryAlgorithm::ReadSettings(const TiXmlHandle )
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
