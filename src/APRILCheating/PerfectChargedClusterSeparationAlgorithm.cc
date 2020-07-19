/**
 *  @file   APRILContent/src/APRILCheating/PerfectChargedClusterSeparationAlgorithm.cc
 * 
 *  @brief  Implementation of the cheating clustering algorithm class
 * 
 *  $Log: $
 *  @author J. Marshall.
 *  Copied from https://github.com/PandoraPFA/LCContent sources
 */

#include "Pandora/AlgorithmHeaders.h"

#include "APRILCheating/PerfectChargedClusterSeparationAlgorithm.h"

using namespace pandora;

namespace april_content
{

  PerfectChargedClusterSeparationAlgorithm::PerfectChargedClusterSeparationAlgorithm()
  {
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode PerfectChargedClusterSeparationAlgorithm::Run()
  {
    const ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

	//std::cout << "-----> list : " << pClusterList->size() << std::endl;

    ClusterList clusterList(pClusterList->begin(), pClusterList->end());
	
	// collection of calo hit and cluster to make new relationship 
    MCParticleToCaloHitListMap mcParticleToCaloHitListMap;

	if(!clusterList.empty())
	{
		ClusterSeparation(clusterList, mcParticleToCaloHitListMap);
	}


	CreateClusters(clusterList, mcParticleToCaloHitListMap);


    return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------
  
  StatusCode PerfectChargedClusterSeparationAlgorithm::ClusterSeparation(ClusterList& localClusterList, MCParticleToCaloHitListMap& mcParticleToCaloHitListMap)
  {
	// collection to store the calo hit of neutral particle 

    for (ClusterList::const_iterator clusterIter = localClusterList.begin(), clusterIterEnd = localClusterList.end(); 
			clusterIter != clusterIterEnd; ++clusterIter)
    {
      try
      {
        const Cluster *const pCluster = *clusterIter;
		//std::cout << "--> In cluster: " << pCluster << std::endl;

#if 0
		const MCParticle *pClusterMCParticle = NULL;

	    try
	    {
           pClusterMCParticle = MCParticleHelper::GetMainMCParticle(pCluster);
	    }
        catch (StatusCodeException &)
        {
        }
#endif

		const OrderedCaloHitList& orderedCaloHitList = pCluster->GetOrderedCaloHitList();

        pandora::CaloHitList hitList;
    	orderedCaloHitList.FillCaloHitList(hitList);

		const pandora::MCParticle* firstMCP = NULL;

    	for(pandora::CaloHitList::const_iterator hitIter = hitList.begin(); hitIter != hitList.end(); ++hitIter)
    	{
    		const pandora::CaloHit* pCaloHit = *hitIter;
    
    		try
    		{
    			const pandora::MCParticle *const pMCParticle(pandora::MCParticleHelper::GetMainMCParticle(pCaloHit));
    			//int mcpCharge = pandora::PdgTable::GetParticleCharge(pMCParticle->GetParticleId());
    
    		    //std::cout << "hit MCP: " << pMCParticle << ", PID: " << pMCParticle->GetParticleId()  << 
    			//	", charge: " << mcpCharge << std::endl;

			   if(firstMCP==NULL)
			   {
			   	 firstMCP = pMCParticle;
			   }

			   if(firstMCP!=NULL && firstMCP != pMCParticle)
			   {
				   //std::cout << "firstMCP: " << firstMCP << ", thisMCP: " << pMCParticle << std::endl;

				   AddToClusterMap(pCaloHit, pMCParticle, mcParticleToCaloHitListMap);
		           PandoraContentApi::RemoveFromCluster(*this, pCluster, pCaloHit);
			   }
    		}
    		catch (pandora::StatusCodeException &)
    		{
    		}
    	}
      }
      catch (StatusCodeException &)
      {
		  continue;
      }
    }

    return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------
  void PerfectChargedClusterSeparationAlgorithm::AddToClusterMap(const pandora::CaloHit* pCaloHit,  const pandora::MCParticle* pMCParticle, MCParticleToCaloHitListMap& mcParticleToCaloHitListMap)
  {
	  MCParticleToCaloHitListMap::iterator caloHitListIter = mcParticleToCaloHitListMap.find(pMCParticle);

	  if(caloHitListIter == mcParticleToCaloHitListMap.end())
	  {
		  pandora::CaloHitList caloHits;
		  caloHits.push_back(pCaloHit);
		  mcParticleToCaloHitListMap.insert(MCParticleToCaloHitListMap::value_type(pMCParticle, caloHits));
	  }
	  else
	  {
		  pandora::CaloHitList& caloHits = caloHitListIter->second;
		  caloHits.push_back(pCaloHit);
	  }
  }
  
  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode PerfectChargedClusterSeparationAlgorithm::CreateClusters(ClusterList& clusterList, MCParticleToCaloHitListMap& mcParticleToCaloHitListMap) const
  {
	const ClusterList *pNewClusterList = NULL; std::string clusterListName;
    PandoraContentApi::CreateTemporaryListAndSetCurrent(*this, pNewClusterList, clusterListName);

	 //----> create separated clusters
	 pandora::ClusterList newClusterList;
	 for(MCParticleToCaloHitListMap::const_iterator iter = mcParticleToCaloHitListMap.begin(); 
			 iter != mcParticleToCaloHitListMap.end(); ++iter)
	 {
		const Cluster *pCluster = NULL;
		const CaloHitList& pCaloHitList = iter->second;

        PandoraContentApi::Cluster::Parameters parameters;
        parameters.m_caloHitList = pCaloHitList;

        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::Cluster::Create(*this, parameters, pCluster));

#if 0
		const MCParticle *pClusterMCParticle = NULL;

	    try
	    {
           pClusterMCParticle = MCParticleHelper::GetMainMCParticle(pCluster);
	    }
        catch (StatusCodeException &)
        {
        }

		std::cout << "========PerfectClusterSeparationAlgorithm: create a cluster: " << pCluster 
			      << ", MCP: " << pClusterMCParticle << std::endl;
#endif

		newClusterList.push_back(pCluster);
	 }

     std::string newClusterListName("NewSeparatedClusters");

     if (!newClusterList.empty())
     {
	    //std::cout << "Create new cluster: NewSeparatedClusters: " << newClusterList.size() << std::endl;
        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::SaveList<Cluster>(*this, newClusterListName));
     }

	 if(!clusterList.empty())
	 {
		 //std::cout << "original cluster size: " << clusterList.size() << std::endl;
	 }

     PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_INITIALIZED, !=, 
				PandoraContentApi::SaveList(*this, newClusterListName, "prePfoCreation2", newClusterList));

	 PandoraContentApi::ReplaceCurrentList<Cluster>(*this, "prePfoCreation2");

     const ClusterList *pClusterList = NULL;
     PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

	  //----> create a map for all existing clusters
	  ClusterList localClusterList(pClusterList->begin(), pClusterList->end());
	  //std::cout << "localClusterList size: " << localClusterList.size() << std::endl;

	  //std::cout << "newClusterList size: " << newClusterList.size() << std::endl;
	  //localClusterList.merge(newClusterList);
	  //std::cout << "after merging, localClusterList size: " << localClusterList.size() << std::endl;
	
	  /// collection of MCP and cluster list
      MCParticleToClusterListMap mcParticleToClusterListMap;

      for (ClusterList::const_iterator clusterIter = localClusterList.begin(), clusterIterEnd = localClusterList.end(); 
	  		clusterIter != clusterIterEnd; ++clusterIter)
      {
          const Cluster *const pCluster = *clusterIter;
	  	  //std::cout << "--> In cluster: " << pCluster << std::endl;

		  const MCParticle *pClusterMCParticle = NULL;

	      try
	      {
             pClusterMCParticle = MCParticleHelper::GetMainMCParticle(pCluster);
	      }
          catch (StatusCodeException &)
          {
          }
	  
		  MCParticleToClusterListMap::iterator clusterListIter = mcParticleToClusterListMap.find(pClusterMCParticle);

		  if(clusterListIter == mcParticleToClusterListMap.end())
		  {
			  ClusterList clusters;
			  clusters.push_back(pCluster);
			  mcParticleToClusterListMap.insert(MCParticleToClusterListMap::value_type(pClusterMCParticle, clusters));
		  }
		  else
		  {
			  ClusterList& clusters = clusterListIter->second;
			  clusters.push_back(pCluster);
		  }
	  }


    // merge cluster with identical MCP
    for (MCParticleToClusterListMap::iterator iter = mcParticleToClusterListMap.begin(), iterEnd = mcParticleToClusterListMap.end(); iter != iterEnd; ++iter)
	{
      ClusterList &clusters = iter->second;

	  if(clusters.size()<=1) continue;

	  const pandora::Cluster *const firstCluster = *(clusters.begin());

	  ClusterList::const_iterator itCluster = clusters.begin();
	  ++itCluster;

      for (ClusterList::const_iterator itClusterEnd = clusters.end(); itCluster != itClusterEnd; ++itCluster)
	  {
		  const pandora::Cluster* cluster = *itCluster;
		  PandoraContentApi::MergeAndDeleteClusters(*this, firstCluster, cluster);
		  //std::cout << "----> merged two clusters: " << firstCluster << " <--- " << cluster << std::endl;
	  }
	}
    
	PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));
	ClusterList finalClusterList(pClusterList->begin(), pClusterList->end());

	//std::cout << "final cluster size: " << finalClusterList.size() << std::endl;

    return STATUS_CODE_SUCCESS;

  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode PerfectChargedClusterSeparationAlgorithm::ReadSettings(const TiXmlHandle )
  {
    return STATUS_CODE_SUCCESS;
  }

}
