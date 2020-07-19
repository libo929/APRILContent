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
    const ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

	//std::cout << "-----> list : " << pClusterList->size() << std::endl;

    ClusterList localClusterList(pClusterList->begin(), pClusterList->end());
    MCParticleToClusterListMap mcParticleToClusterListMap;
	
	if(!localClusterList.empty())
	{
		for(ClusterList::const_iterator iter = localClusterList.begin(); iter != localClusterList.end(); ++iter)
		{
			const Cluster* const pCluster = *iter;
			SimpleMCParticleClusterListCollection(pCluster, mcParticleToClusterListMap);
		}
	}

	//std::cout << "-----> mcParticleToClusterListMap size: " << mcParticleToClusterListMap.size() << std::endl;

	///////////////////////////////////


	// collection of calo hit and cluster to remove relationship
	CaloHitToClusterMap caloHitToClusterRemoveMap;

	// collection of calo hit and cluster to make new relationship 
	CaloHitToClusterMap caloHitToClusterAddMap;

	// collection to store the calo hit of neutral particle 
    MCParticleToCaloHitListMap mcParticleToCaloHitListMap;

    for (ClusterList::const_iterator clusterIter = localClusterList.begin(), clusterIterEnd = localClusterList.end(); 
			clusterIter != clusterIterEnd; ++clusterIter)
    {
      try
      {
        const Cluster *const pCluster = *clusterIter;
		const TrackList& trackList = pCluster->GetAssociatedTrackList();
		bool isChargedCluster = trackList.size() != 0;

		const MCParticle *pClusterMCParticle = NULL;
#if 1
	    try
	    {
           pClusterMCParticle = MCParticleHelper::GetMainMCParticle(pCluster);
	    }
        catch (StatusCodeException &)
        {
        }
#endif

		const OrderedCaloHitList& orderedCaloHitList = pCluster->GetOrderedCaloHitList();

		//std::cout << "cluster energy: " << pCluster->GetHadronicEnergy() << ", orderedCaloHitList size: " << orderedCaloHitList.size() << std::endl;

		for(OrderedCaloHitList::const_iterator hitListIter = orderedCaloHitList.begin(); hitListIter != orderedCaloHitList.end(); 
				++hitListIter)
		{
			const CaloHitList* hitListInLayer = hitListIter->second;
			//std::cout << " === hit list size at layer " << hitListIter->first << ": " << hitListInLayer->size() << std::endl;

			for(CaloHitList::const_iterator hitIter = hitListInLayer->begin(); hitIter != hitListInLayer->end(); ++hitIter)
			{
				//std::cout << " --------->>>>> try a calo hit: " << *hitIter << std::endl;
				const CaloHit* pCaloHit = *hitIter;
				//std::cout << " ==== calo hit: " << pCaloHit << std::endl;

	            try
	            {
                   const MCParticle *const pMCParticle(MCParticleHelper::GetMainMCParticle(pCaloHit));

				   int mcpPID = pMCParticle->GetParticleId();
				   bool isNeutralHit = !(abs(mcpPID) == 211 || abs(mcpPID) == 321 || abs(mcpPID) == 2212);

				   if(pClusterMCParticle != NULL)
				   {
					   if(pClusterMCParticle->GetParticleId() == pMCParticle->GetParticleId()) continue;
				   }

			       // if the calo hit is neutral but its associated cluster is charged,
				   if(isNeutralHit && isChargedCluster)
				   {
				       //std::cout << "MCP: " << pMCParticle << ", PID: " << pMCParticle->GetParticleId() << std::endl;
			           // remove the calo hit from cluster and add it to the correct cluster by MCP
				       //std::cout << "cluster PDG: " << pClusterMCParticle->GetParticleId() << ", energy: " 
					   // << pClusterMCParticle->GetEnergy() << " ///// calo hit PDG: " << pMCParticle->GetParticleId() << std::endl;

					   caloHitToClusterRemoveMap.insert(CaloHitToClusterMap::value_type(pCaloHit, pCluster));

					   const MCParticleToClusterListMap::const_iterator clusterListIter = mcParticleToClusterListMap.find(pMCParticle);

					   if(clusterListIter != mcParticleToClusterListMap.end())
					   {
					       // if there is a correct cluster to add the hit
						   const ClusterList* cluList = clusterListIter->second;

						   if(!cluList->empty()) 
						   {
						      const Cluster* pCorrectCluster = *(cluList->begin());
					          caloHitToClusterAddMap.insert(CaloHitToClusterMap::value_type(pCaloHit, pCorrectCluster));
						   }
					   }
					   else
					   {
						   // if no correct cluster
						   //std::cout << " === AddToHitListMap " << std::endl;
                           AddToCaloHitListMap(pCaloHit, pMCParticle, mcParticleToCaloHitListMap);
						   //std::cout << " ======= AddToHitListMap " << std::endl;
					   }
				   }
	            }
                catch (StatusCodeException &)
                {
			    	continue;
                }
			}
		}

		// Remove calo hit and cluster relationship
		RemoveClusterCaloHitAssociations(caloHitToClusterRemoveMap);
		
		// Add calo hit and cluster relationship
		AddClusterCaloHitAssociations(caloHitToClusterAddMap);

		// Create neutral clusters from calo hit list
#if 1
		const ClusterList *pNewClusterList = NULL; std::string newClusterListName;
        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::CreateTemporaryListAndSetCurrent(*this, pNewClusterList, newClusterListName));
#endif

		CreateNeutralClusters(mcParticleToCaloHitListMap);

#if 1
	    std::string m_outputClusterListName("RecoveredNeutralClusters");
        if (!pNewClusterList->empty())
        {
           PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::SaveList<Cluster>(*this, m_outputClusterListName));
        }
#endif

		// Clear 
	    ClearCaloHitListMap(mcParticleToCaloHitListMap);

		// IsoHit ...
		//const CaloHitList& isolatedCaloHitList = pCluster->GetIsolatedCaloHitList();
      }
      catch (StatusCodeException &)
      {
		  continue;
      }
    }

    return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------
  
  void PerfectNeutralHitRecoveryAlgorithm::RemoveClusterCaloHitAssociations(CaloHitToClusterMap& caloHitToClusterRemoveMap) const
  {
	//std::cout << "Remove: " << caloHitToClusterRemoveMap.size() << std::endl;
	for(CaloHitToClusterMap::const_iterator iter = caloHitToClusterRemoveMap.begin(); iter != caloHitToClusterRemoveMap.end();
			++iter)
	{
		const CaloHit* pCaloHit = iter->first;
		const Cluster* pCluster = iter->second;
		PandoraContentApi::RemoveFromCluster(*this, pCluster, pCaloHit);
		//std::cout << "cluster: " << pCluster << ", calohit: " << pCaloHit << std::endl;
	}

	caloHitToClusterRemoveMap.clear();
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void PerfectNeutralHitRecoveryAlgorithm::AddClusterCaloHitAssociations(CaloHitToClusterMap& caloHitToClusterAddMap) const
  {
	for(CaloHitToClusterMap::const_iterator iter = caloHitToClusterAddMap.begin(); iter != caloHitToClusterAddMap.end();
			++iter)
	{
		const CaloHit* pCaloHit = iter->first;
		const Cluster* pCluster = iter->second;

		//PandoraContentApi::AddIsolatedToCluster(*this, pCluster, pCaloHit);
		PandoraContentApi::AddToCluster(*this, pCluster, pCaloHit);
	    //std::cout << " ==========AddClusterCaloHitAssociation: cluster: " << pCluster << " E: " << pCluster->GetHadronicEnergy() 
	   	//<< std::endl;
	}


	caloHitToClusterAddMap.clear();
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void PerfectNeutralHitRecoveryAlgorithm::CreateNeutralClusters(MCParticleToCaloHitListMap& mcParticleToCaloHitListMap) const
  {
	 for(MCParticleToCaloHitListMap::const_iterator iter = mcParticleToCaloHitListMap.begin(); 
			 iter != mcParticleToCaloHitListMap.end(); ++iter)
	 {
		const Cluster *pCluster = NULL;
		const CaloHitList* pCaloHitList = iter->second;

        PandoraContentApi::Cluster::Parameters parameters;
        parameters.m_caloHitList = *pCaloHitList;

        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::Cluster::Create(*this, parameters, pCluster));
		//std::cout << " ========PerfectNeutralHitRecoveryAlgorithm::CreateNeutralCluster ..." << std::endl;
	 }
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void PerfectNeutralHitRecoveryAlgorithm::SimpleMCParticleClusterListCollection(const Cluster *const pCluster, 
		  MCParticleToClusterListMap &mcParticleToClusterListMap) const
  {
	try
	{
       const MCParticle *const pMCParticle(MCParticleHelper::GetMainMCParticle(pCluster));
       this->AddToClusterListMap(pCluster, pMCParticle, mcParticleToClusterListMap);
	}
    catch (StatusCodeException &)
    {
    }
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void PerfectNeutralHitRecoveryAlgorithm::AddToClusterListMap(const Cluster *const pClusterToAdd, const MCParticle *const pMCParticle,
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

  void PerfectNeutralHitRecoveryAlgorithm::ClearCaloHitListMap(MCParticleToCaloHitListMap &mcParticleToCaloHitListMap) const
  {
	// Do we need this ?
    for(MCParticleToCaloHitListMap::iterator iter = mcParticleToCaloHitListMap.begin(); iter != mcParticleToCaloHitListMap.end(); ++iter)
	{
		CaloHitList *const pCaloHitList = iter->second;
		delete pCaloHitList;
	}

	mcParticleToCaloHitListMap.clear();
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode PerfectNeutralHitRecoveryAlgorithm::ReadSettings(const TiXmlHandle )
  {
    return STATUS_CODE_SUCCESS;
  }

}
