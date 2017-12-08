/**
 *
 * This algorithm is to remove isolated hits by creating clusters
 * 
 */

#include "Pandora/AlgorithmHeaders.h"

#include "ArborClustering/IsoHitRemovalAlgorithm.h"
#include "ArborUtility/MeanShift.h"
#include "ArborUtility/ClusterShape.h"

using namespace pandora;

//FIXME
HitKDTree hitsKDTree;

namespace arbor_content
{

  IsoHitRemovalAlgorithm::IsoHitRemovalAlgorithm()
  {
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode IsoHitRemovalAlgorithm::Run()
  {
	//Test();
	
	/////////
	hitsKDTree.clear();
	BuildKDTree(hitsKDTree);
	//hitsKDTree.clear();

	/////////
	CaloHitList isoHitList;
    MCParticleToCaloHitListMap mcParticleToCaloHitListMap;

	GetIsoHits(isoHitList, mcParticleToCaloHitListMap);
	std::cout << "isolated hit size: " << isoHitList.size() << std::endl;

	/////////
	ClusterList clusterList;
    MCParticleToClusterMap mcParticleToClusterMap;
	GetClusters(clusterList, mcParticleToClusterMap);
	std::cout << "cluster size: " << clusterList.size() << std::endl;

#if 1
	/////////
    const ClusterList *pNewClusterList = NULL; std::string newClusterListName;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::CreateTemporaryListAndSetCurrent(*this, pNewClusterList, newClusterListName));

	std::vector<CaloHitList> clusterHitsCollection;
	MeanShiftClustering(isoHitList, clusterHitsCollection);

	for(int iList = 0; iList < clusterHitsCollection.size(); ++iList)
	{
		CreateCluster(&clusterHitsCollection.at(iList));
	}

#if 0
#if 0
    if(mcParticleToCaloHitListMap.empty() == false && mcParticleToClusterMap.empty() == false)
	{ 
		this->MCMergeCaloHits(mcParticleToCaloHitListMap, mcParticleToClusterMap);
	}
#else
	if(isoHitList.size() && clusterList.size())
	{
		MergeCaloHits(isoHitList, clusterList);
	}
#endif
#endif

	std::string m_outputClusterListName("ClustersFromIsoHits");
    if (!pNewClusterList->empty())
    {
      PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::SaveList<Cluster>(*this, m_outputClusterListName));
    }

#endif

	hitsKDTree.clear();

	//////


	//std::cout << "======== mcParticleToCaloHitListMap size: " << mcParticleToCaloHitListMap.size() << std::endl;
	//std::cout << "======== mcParticleToClusterMap size: " << mcParticleToClusterMap.size() << std::endl;

    return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------
  
  StatusCode IsoHitRemovalAlgorithm::MeanShiftClustering(CaloHitList& isoHitList, std::vector<CaloHitList>& clusterHitsCollection)
  {
	double shift_epsilon = 0.000001;

	MeanShift meanShift(m_kernelBandwidth, m_clusterEpsilon, shift_epsilon);

	vector<MSPoint> msCaloHitPoints;

	for(CaloHitList::iterator caloHitIter = isoHitList.begin(); caloHitIter != isoHitList.end(); ++caloHitIter)
	{
		CartesianVector hitPosition((*caloHitIter)->GetPositionVector());
		
		MSPoint msp(*caloHitIter);
		msp.push_back(hitPosition.GetX());
		msp.push_back(hitPosition.GetY());
		msp.push_back(hitPosition.GetZ());

		msCaloHitPoints.push_back(msp);
	}

	std::vector<MSCluster> msClusters = meanShift.cluster(msCaloHitPoints);

    std::cout << "  ======>>> meanshift cluster size: " << msClusters.size() << std::endl;	

	for(int iMSCluster = 0; iMSCluster < msClusters.size(); ++iMSCluster)
	{
		MSCluster& msCluster = msClusters.at(iMSCluster);
		std::vector<MSPoint>& caloHitPoints = msCluster.original_points;

#if 0
		std::cout << "|---> cluster: " << iMSCluster << std::endl;
#endif

		CaloHitList caloHitList;
		for(int iPoint = 0; iPoint < caloHitPoints.size(); ++iPoint)
		{
			MSPoint& hit = caloHitPoints.at(iPoint);
			const CaloHit* caloHit = hit.m_caloHit;
			CartesianVector hitPos(caloHit->GetPositionVector());

#if 0
			std::cout << "  ----> hit point: " << hit.at(0) << ", " << hit.at(1) << ", " << hit.at(2) 
				      << " --|-- " << hitPos.GetX() << ", " << hitPos.GetY() << ", " << hitPos.GetZ() << std::endl;
#endif

			caloHitList.insert(caloHit);
		}
	
		clusterHitsCollection.push_back(caloHitList);
	}

	return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode IsoHitRemovalAlgorithm::GetIsoHits(CaloHitList& isoHitList, MCParticleToCaloHitListMap& mcParticleToCaloHitListMap)
  {
    const CaloHitList *pCaloHitList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList));

    CaloHitList localCaloHitList(pCaloHitList->begin(), pCaloHitList->end());

    for (CaloHitList::const_iterator caloHitIter = localCaloHitList.begin(), caloHitIterEnd = localCaloHitList.end(); 
			caloHitIter != caloHitIterEnd; ++caloHitIter)
    {
      try
      {
        const CaloHit *const pCaloHit = *caloHitIter;

        if (!PandoraContentApi::IsAvailable(*this, pCaloHit))
          continue;

		//energyMissingHits += pCaloHit->GetHadronicEnergy();
		isoHitList.insert(pCaloHit);

        this->SimpleMCParticleCaloHitListCollection(pCaloHit, mcParticleToCaloHitListMap);
      }
      catch (StatusCodeException &)
      {
      }
    }

    return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode IsoHitRemovalAlgorithm::GetClusters(ClusterList& clusterList, MCParticleToClusterMap& mcParticleToClusterMap)
  {
    const ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

    ClusterList localClusterList(pClusterList->begin(), pClusterList->end());

    for (ClusterList::const_iterator clusterIter = localClusterList.begin(), clusterIterEnd = localClusterList.end(); 
			clusterIter != clusterIterEnd; ++clusterIter)
	{
		const Cluster *const pCluster = *clusterIter;

		clusterList.insert(pCluster);
        this->SimpleMCParticleClusterCollection(pCluster, mcParticleToClusterMap);
	}

    return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode IsoHitRemovalAlgorithm::BuildKDTree(HitKDTree& hits_kdtree)
  {
     const ClusterList *pClusterList = NULL;
     PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

     std::vector<HitKDNode> hit_nodes;
     CaloHitList hit_list, clusterHits;

     // build the kd-tree of hits from the input clusters and save the map of hits to clusters
     for (const Cluster *const pCluster : *pClusterList)
     {   
         pCluster->GetOrderedCaloHitList().GetCaloHitList(clusterHits);

         for (const CaloHit *const pCaloHit : clusterHits)
         {   
             hit_list.insert(pCaloHit);
         }   

         clusterHits.clear();
     }   

     KDTreeTesseract hitsBoundingRegion = fill_and_bound_4d_kd_tree(this, hit_list, hit_nodes, true);
     hits_kdtree.build(hit_nodes, hitsBoundingRegion);
     hit_nodes.clear();

	 return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode IsoHitRemovalAlgorithm::SearchNearbyCaloHits(const CaloHit* pCaloHit, 
              std::vector<const CaloHit*>& nearbyHits, float wideX, float wideY, float wideZ, int layers)
  {
     std::vector<HitKDNode> found_hits; 

     CartesianVector hitPosition(pCaloHit->GetPositionVector());
	 //std::cout << "hit center: " << hitPosition << std::endl;

     KDTreeTesseract searchRegionHits = build_4d_kd_search_region(hitPosition, wideX, wideY, wideZ, layers); 
     hitsKDTree.search(searchRegionHits, found_hits);

     for (const auto &hit : found_hits)                                                                       
     {
         auto caloHit = hit.data;
#if 0
	     CartesianVector pos(caloHit->GetPositionVector());
	     CartesianVector distance = pos - hitPosition;

	     std::cout << "   -----> hit layer: " << caloHit->GetLayer() << "  pos: " << pos 
	    	       << ", distance: " << distance.GetMagnitude() << std::endl;
#endif

		 nearbyHits.push_back(caloHit);
     }

     found_hits.clear();  

	 return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode IsoHitRemovalAlgorithm::Test()
  {
     const ClusterList *pClusterList = NULL;
     PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

     HitKDTree hits_kdtree;
     std::vector<HitKDNode> hit_nodes;
     CaloHitList hit_list, clusterHits;
	 HitsToClustersMap hits_to_clusters;

     // build the kd-tree of hits from the input clusters and save the map of hits to clusters
     for (const Cluster *const pCluster : *pClusterList)
     {   
         pCluster->GetOrderedCaloHitList().GetCaloHitList(clusterHits);

         for (const CaloHit *const pCaloHit : clusterHits)
         {   
             hit_list.insert(pCaloHit);
             hits_to_clusters.emplace(pCaloHit, pCluster);
         }   

         clusterHits.clear();
     }   

     KDTreeTesseract hitsBoundingRegion = fill_and_bound_4d_kd_tree(this, hit_list, hit_nodes, true);
     //hit_list.clear();
     hits_kdtree.build(hit_nodes,hitsBoundingRegion);
     hit_nodes.clear();

	 for(CaloHitList::const_iterator iter = hit_list.begin(); iter != hit_list.end(); ++iter)
	 {
         //ClusterList nearby_clusters;                                                                                              
         std::vector<HitKDNode> found_hits; 

		 const CaloHit* pCaloHit = *iter;
         CartesianVector hitPosition(pCaloHit->GetPositionVector());
		 std::cout << "hit center: " << hitPosition << std::endl;

         float m_parallelDistanceCut(200.);
         float iPseudoLayer(2);
    
         KDTreeTesseract searchRegionHits = build_4d_kd_search_region(hitPosition, m_parallelDistanceCut, 
      		   m_parallelDistanceCut, m_parallelDistanceCut, iPseudoLayer);
         hits_kdtree.search(searchRegionHits, found_hits);

         for (const auto &hit : found_hits)                                                                       
         {                                                                                                        
             auto caloHit = hit.data;                                               
			 CartesianVector pos(caloHit->GetPositionVector());
			 CartesianVector distance = pos - hitPosition;

			 std::cout << "   -----> hit layer: " << caloHit->GetLayer() << "  pos: " << pos 
				       << ", distance: " << distance.GetMagnitude() << std::endl;
         }                                                                                                        

         found_hits.clear();  
	 }
    
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

  int IsoHitRemovalAlgorithm::GetMCParticle(const Cluster* pCluster) const
  {
	const MCParticle * pMCParticle = NULL;

	try
	{
		pMCParticle = MCParticleHelper::GetMainMCParticle(pCluster);
	}
    catch (StatusCodeException &)
    {
    }

	int mcpPID = pMCParticle->GetParticleId();
	//bool isNeutralCluster = !(abs(mcpPID) == 211 || abs(mcpPID) == 321 || abs(mcpPID) == 2212);

	return mcpPID;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode IsoHitRemovalAlgorithm::MCMergeCaloHits(const MCParticleToCaloHitListMap &mcParticleToCaloHitListMap,
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

		if(iter == mcParticleToClusterMap.end())
		{
			// create new cluster for isolated hit
			CreateCluster(pCaloHitList);
		}
		else
		{
		    const Cluster* const pCluster = iter->second;
	
			std::cout << " |------>  cluster to add hits: " << GetMCParticle(pCluster) << ":: " << pCaloHitList->size() << std::endl;

		    for(CaloHitList::const_iterator caloHitIter = pCaloHitList->begin(); caloHitIter != pCaloHitList->end(); ++caloHitIter)
		    {
		    	const CaloHit* const caloHit = *caloHitIter;

		    	PandoraContentApi::AddToCluster(*this, pCluster, caloHit);
		    }
		}
	  }

      delete pCaloHitList;
    }

	//std::cout << "   ==== hit merged: " << nHitMerged << ", hit passed: " << nPassed << std::endl;

	return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode IsoHitRemovalAlgorithm::MergeCaloHits(const CaloHitList& isoHitList, const ClusterList& clusterList)
  {
    for (CaloHitList::const_iterator hitIter = isoHitList.begin(); hitIter != isoHitList.end(); ++hitIter)
    {
      const CaloHit* const pCaloHit = *hitIter;

	  std::vector<const CaloHit*> nearbyHits;
	  
	  float wideX = 100.;
	  float wideY = 100.;
	  float wideZ = 100.;

	  int   layers = 3;

	  SearchNearbyCaloHits(pCaloHit, nearbyHits, wideX, wideY, wideZ, layers);
    }

	clusterList.size();
	//SearchNearbyClusters(pCaloHit, nearbyClusters);

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
	//

    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::Cluster::Create(*this, parameters, pCluster));

	if(pCluster==NULL) 
	{
		std::cout << " --- cluster not created... " << std::endl;
		return;
	}

	float clusterEnergy = pCluster->GetHadronicEnergy();
	std::cout << "cluster energy: " << clusterEnergy << std::endl;

	// FIXME
	if(clusterEnergy<0.01 && pCaloHitList->size() <=1 )
	{
		std::cout << " |=====>  a cluster with energy: " << pCluster->GetHadronicEnergy() 
			      << ", hit size: " << pCaloHitList->size() << ", not created ..." << std::endl;

		PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::Delete(*this, pCluster));
	}
	else 
	{
		int pid = 0;

	    try
	    {
           const MCParticle *const pMCParticle(MCParticleHelper::GetMainMCParticle(pCluster));
		   pid = pMCParticle->GetParticleId();
	    }
        catch (StatusCodeException &)
        {
        }

#if 1
		std::cout << " |------>  created a cluster with PID: " << pid << ", energy: " << pCluster->GetHadronicEnergy() 
			      << ", hit size: " << pCaloHitList->size() << std::endl;
#endif

		if(pCaloHitList->size()>=5)
		{
			ClusterShape cs(pCluster);
			double csFactor = cs.CalcClusterShapeFactor();
			std::cout << "   |------> shape factor: " << csFactor << std::endl;

			// cluster shape factor < 0.2 : track-like
			// cluster shape factor > 0.2 : blob
		}
	}
  }

  //------------------------------------------------------------------------------------------------------------------------------------------
  
  StatusCode IsoHitRemovalAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
  {
	m_kernelBandwidth = 100.;

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "KernelBandwidth", m_kernelBandwidth));

	m_clusterEpsilon = 30.;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ClusterEpsilon", m_clusterEpsilon));

    return STATUS_CODE_SUCCESS;
  }

}
