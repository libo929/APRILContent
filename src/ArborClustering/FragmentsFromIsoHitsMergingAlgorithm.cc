/**
 *
 * This algorithm is to merge clusters created from isolated hits
 * 
 */

#include "Pandora/AlgorithmHeaders.h"

#include "ArborClustering/FragmentsFromIsoHitsMergingAlgorithm.h"
#include "ArborUtility/ClusterShape.h"
#include "ArborHelpers/ClusterHelper.h"

//FIXME
HitKDTree caloHitsKDTree;

namespace arbor_content
{

  FragmentsFromIsoHitsMergingAlgorithm::FragmentsFromIsoHitsMergingAlgorithm()
  {
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode FragmentsFromIsoHitsMergingAlgorithm::Run()
  {
	caloHitsKDTree.clear();
	BuildKDTree(caloHitsKDTree);

	/////////
	pandora::ClusterList mainClusterList;
	std::string mainClusterName("PfoCreation");
	GetClusters(mainClusterList, mainClusterName);
	//std::cout << "main cluster size: " << mainClusterList.size() << std::endl;

	pandora::ClusterList clustersFromIsoHits;
	std::string fragmentsName("ClustersFromIsoHits");
	GetClusters(clustersFromIsoHits, fragmentsName);

	MergeFragments(mainClusterList, clustersFromIsoHits);

	caloHitsKDTree.clear();

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode FragmentsFromIsoHitsMergingAlgorithm::MergeFragments(pandora::ClusterList& mainClusterList, pandora::ClusterList& clustersFromIsoHits)
  {
	  std::map<pandora::Cluster*, TVector3> mainclusterCentroidMap;
	  std::map<pandora::Cluster*, TVector3> fragmentCentroidMap;

	  for(pandora::ClusterList::iterator iter = mainClusterList.begin(); iter != mainClusterList.end(); ++iter)
	  {
		  const pandora::Cluster* pCluster = *iter;
		  pandora::CartesianVector centroid(0.f, 0.f, 0.f);

		  PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ClusterHelper::GetCentroid(pCluster, centroid));
	  }
	  
	  return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode FragmentsFromIsoHitsMergingAlgorithm::GetClusters(pandora::ClusterList& clusterList, const std::string& listName)
  {
    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, listName, pClusterList));

    pandora::ClusterList localClusterList(pClusterList->begin(), pClusterList->end());

    for (pandora::ClusterList::const_iterator clusterIter = localClusterList.begin(), clusterIterEnd = localClusterList.end(); 
			clusterIter != clusterIterEnd; ++clusterIter)
	{
		const pandora::Cluster *const pCluster = *clusterIter;
		clusterList.insert(pCluster);
	}

	std::cout << "cluster list " << listName << " size: " << clusterList.size() << std::endl;

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode FragmentsFromIsoHitsMergingAlgorithm::BuildKDTree(HitKDTree& hits_kdtree)
  {
     const pandora::ClusterList *pClusterList = NULL;
     PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

     std::vector<HitKDNode> hit_nodes;
	 pandora::CaloHitList hit_list, clusterHits;

     // build the kd-tree of hits from the input clusters and save the map of hits to clusters
     for (const pandora::Cluster *const pCluster : *pClusterList)
     {   
         pCluster->GetOrderedCaloHitList().GetCaloHitList(clusterHits);

         for (const pandora::CaloHit *const pCaloHit : clusterHits)
         {   
             hit_list.insert(pCaloHit);
         }   

         clusterHits.clear();
     }   

     KDTreeTesseract hitsBoundingRegion = fill_and_bound_4d_kd_tree(this, hit_list, hit_nodes, true);
     hits_kdtree.build(hit_nodes, hitsBoundingRegion);
     hit_nodes.clear();

	 return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode FragmentsFromIsoHitsMergingAlgorithm::SearchNearbyCaloHits(const pandora::CaloHit* pCaloHit, 
              std::vector<const pandora::CaloHit*>& nearbyHits, float wideX, float wideY, float wideZ, int layers)
  {
     std::vector<HitKDNode> found_hits; 

	 pandora::CartesianVector hitPosition(pCaloHit->GetPositionVector());
	 //std::cout << "hit center: " << hitPosition << std::endl;

     KDTreeTesseract searchRegionHits = build_4d_kd_search_region(hitPosition, wideX, wideY, wideZ, layers); 
     caloHitsKDTree.search(searchRegionHits, found_hits);

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

	 return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode FragmentsFromIsoHitsMergingAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
	m_kernelBandwidth = 100.;

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "KernelBandwidth", m_kernelBandwidth));

	m_clusterEpsilon = 30.;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ClusterEpsilon", m_clusterEpsilon));

    return pandora::STATUS_CODE_SUCCESS;
  }

}
