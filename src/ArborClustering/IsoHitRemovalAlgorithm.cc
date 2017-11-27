/**
 *  @file   ArborContent/src/ArborCheating/IsoHitRemovalAlgorithm.cc
 * 
 *  @brief  Implementation of the cheating clustering algorithm class
 * 
 *  $Log: $
 *  @author J. Marshall.
 *  Copied from https://github.com/PandoraPFA/LCContent sources
 */

#include "Pandora/AlgorithmHeaders.h"

#include "ArborCheating/IsoHitRemovalAlgorithm.h"

using namespace pandora;

namespace arbor_content
{

  IsoHitRemovalAlgorithm::IsoHitRemovalAlgorithm()
  {
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode IsoHitRemovalAlgorithm::Run()
  {
	m_CaloHitMCGetterFailures = 0;

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

	///////////////
#if 1
    const ClusterList *pNewClusterList = NULL; std::string newClusterListName;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::CreateTemporaryListAndSetCurrent(*this, pNewClusterList, newClusterListName));
#endif

    if(mcParticleToCaloHitListMap.empty() == false && mcParticleToClusterMap.empty() == false)
	{ 
		this->MergeCaloHits(mcParticleToCaloHitListMap, mcParticleToClusterMap);
	}

#if 1
	std::string m_outputClusterListName("ClustersFromIsoHits");
    if (!pNewClusterList->empty())
    {
      PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::SaveList<Cluster>(*this, m_outputClusterListName));
    }
#endif

	std::cout << "======== mcParticleToCaloHitListMap size: " << mcParticleToCaloHitListMap.size() << std::endl;
	std::cout << "======== mcParticleToClusterMap size: " << mcParticleToClusterMap.size() << std::endl;
	std::cout << "============= missing hits : " << nMissingHits << ", energy: " <<  energyMissingHits << std::endl;
	std::cout << "============= CaloHitMCGetterFailures: " << m_CaloHitMCGetterFailures << std::endl;

    return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void IsoHitRemovalAlgorithm::SimpleMCParticleCaloHitListCollection(const CaloHit *const pCaloHit, 
		  MCParticleToCaloHitListMap &mcParticleToCaloHitListMap)
  {
	try
	{
       const MCParticle *const pMCParticle(MCParticleHelper::GetMainMCParticle(pCaloHit));
       this->AddToCaloHitListMap(pCaloHit, pMCParticle, mcParticleToCaloHitListMap);
	}
    catch (StatusCodeException &)
    {
		//++m_CaloHitMCGetterFailures;
    }
  }

  //------------------------------------------------------------------------------------------------------------------------------------------
  
  void IsoHitRemovalAlgorithm::SimpleMCParticleClusterCollection(const Cluster *const pCluster, 
		  MCParticleToClusterMap &mcParticleToClusterMap)
  {
	try
	{
       const MCParticle *const pMCParticle(MCParticleHelper::GetMainMCParticle(pCluster));
       this->AddToClusterMap(pCluster, pMCParticle, mcParticleToClusterMap);
	}
    catch (StatusCodeException &)
    {
		//++m_CaloHitMCGetterFailures;
    }
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void IsoHitRemovalAlgorithm::AddToCaloHitListMap(const CaloHit *const pCaloHitToAdd, const MCParticle *const pMCParticle,
      MCParticleToCaloHitListMap &mcParticleToCaloHitListMap) const
  {
    MCParticleToCaloHitListMap::iterator iter(mcParticleToCaloHitListMap.find(pMCParticle));

    if (!PandoraContentApi::IsAvailable(*this, pCaloHitToAdd)) return;

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
  
  void IsoHitRemovalAlgorithm::AddToClusterMap(const Cluster *const pClusterToAdd, const MCParticle *const pMCParticle,
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

  StatusCode IsoHitRemovalAlgorithm::MergeCaloHits(const MCParticleToCaloHitListMap &mcParticleToCaloHitListMap,
		                                                  const MCParticleToClusterMap &mcParticleToClusterMap) const
  {
	int nHitMerged = 0;
	int nPassed = 0;

    for (MCParticleToCaloHitListMap::const_iterator mapIter = mcParticleToCaloHitListMap.begin(), 
	    mapIterEnd = mcParticleToCaloHitListMap.end(); mapIter != mapIterEnd; ++mapIter)
    {
      const MCParticle *const pMCParticle = mapIter->first;
      CaloHitList *const pCaloHitList = mapIter->second;

      if (pCaloHitList->size()>0)
      {
		// get the cluster which will merge hits by MCP

		MCParticleToClusterMap::const_iterator iter(mcParticleToClusterMap.find(pMCParticle));

		if(iter == mcParticleToClusterMap.end()) 
		{
			// create new cluster for isolated hit
			CreateCluster(pCaloHitList);
		}
		else
		{
		    const Cluster* const pCluster = iter->second;

		    for(CaloHitList::const_iterator caloHitIter = pCaloHitList->begin(); caloHitIter != pCaloHitList->end(); ++caloHitIter)
		    {
		    	const CaloHit* const caloHit = *caloHitIter;

		    	// only for neutral particle ?

		    	//std::cout << "++++++ Add hit to cluster: " << pCluster << ", " << caloHit << std::endl;
		    	//std::cout << pCluster->GetHadronicEnergy() << std::endl;
		    	//PandoraContentApi::AddIsolatedToCluster(*this, pCluster, caloHit);
		    	PandoraContentApi::AddToCluster(*this, pCluster, caloHit);
		    	//std::cout << "  adding done..." << std::endl;
		    	++nHitMerged;
		    }
		}
	  }

      delete pCaloHitList;
    }

	//std::cout << "   ==== hit merged: " << nHitMerged << ", hit passed: " << nPassed << std::endl;

	return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void IsoHitRemovalAlgorithm::CreateCluster(const CaloHitList *const caloHitList) const
  {
	const Cluster *pCluster = NULL;
	const CaloHitList* pCaloHitList = caloHitList;

	if(pCaloHitList->size()==0) return;

    PandoraContentApi::Cluster::Parameters parameters;
    parameters.m_caloHitList = *pCaloHitList;

	//std::cout << " need to CreateCluster" << std::endl;


    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::Cluster::Create(*this, parameters, pCluster));
    //PandoraContentApi::Cluster::Create(*this, parameters, pCluster);
  }

  //------------------------------------------------------------------------------------------------------------------------------------------
  
  StatusCode IsoHitRemovalAlgorithm::ReadSettings(const TiXmlHandle )
  {
    return STATUS_CODE_SUCCESS;
  }

}
