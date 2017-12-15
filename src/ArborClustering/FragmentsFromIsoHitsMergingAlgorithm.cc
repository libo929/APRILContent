/**
 *
 * This algorithm is to merge clusters created from isolated hits
 * 
 */

#include "Pandora/AlgorithmHeaders.h"

#include "ArborClustering/FragmentsFromIsoHitsMergingAlgorithm.h"
#include "ArborUtility/ClusterShape.h"
#include "ArborHelpers/ClusterHelper.h"
#include "ArborHelpers/ClusterPropertiesHelper.h"

#include "TMath.h"

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
	  std::map<const pandora::Cluster*, TVector3> mainclusterCentroidMap;
	  std::map<const pandora::Cluster*, TVector3> fragmentCentroidMap;
	  std::map<const pandora::Cluster*, pandora::ClusterList> clusterNearbyFragments;

	  for(pandora::ClusterList::iterator iter = clustersFromIsoHits.begin(); iter != clustersFromIsoHits.end(); ++iter)
	  {
		  const pandora::Cluster* pFragment = *iter;
		  pandora::CartesianVector centroid(0.f, 0.f, 0.f);

		  PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ClusterHelper::GetCentroid(pFragment, centroid));

		  TVector3 centroidVec(centroid.GetX(), centroid.GetY(), centroid.GetZ());

		  fragmentCentroidMap.insert( std::pair<const pandora::Cluster*, TVector3>(pFragment, centroidVec) );
		  std::cout << "cluster: " << pFragment << "------> " << centroidVec.X() << ", " << centroidVec.Y() << ", " << centroidVec.Z() << std::endl;

			 // fragment's PCA
			 TVector3 fragmentAxis = GetClustersAxis(pFragment);
			 fragmentAxis.Unit();
			 pandora::CartesianVector axis(fragmentAxis.X(), fragmentAxis.Y(), fragmentAxis.Z());

			 TVector3 fragmentCentroid = fragmentCentroidMap.find(pFragment)->second;

			 double openAngle = fragmentCentroid.Angle(fragmentAxis);

			 openAngle = std::min(openAngle, TMath::Pi()-openAngle);

			 std::cout << "-------> nearby fragments : " << pFragment->GetHadronicEnergy() << ", axis: " 
				       << fragmentAxis.X() << ", " << fragmentAxis.Y() << ", " << fragmentAxis.Z() << ", angle: " << openAngle << std::endl;

			 pFragment->SetCentroid(centroid);
			 pFragment->SetAxis(axis);
	  }

	  for(pandora::ClusterList::iterator clusterIter = mainClusterList.begin(); clusterIter != mainClusterList.end(); ++clusterIter)
	  {
		  const pandora::Cluster* pCluster = *clusterIter;
		  pandora::CartesianVector centroid(0.f, 0.f, 0.f);

		  PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ClusterHelper::GetCentroid(pCluster, centroid));

		  TVector3 clusterCentroid(centroid.GetX(), centroid.GetY(), centroid.GetZ());

		  mainclusterCentroidMap.insert( std::pair<const pandora::Cluster*, TVector3>(pCluster, clusterCentroid) );
		  std::cout << "cluster: " << pCluster << "---> " << clusterCentroid.X() << ", " << clusterCentroid.Y() << ", " << clusterCentroid.Z() << std::endl;
		
		  ///////  
		  pandora::ClusterList nearbyFragments;

		  for(auto fragIter = fragmentCentroidMap.begin(); fragIter != fragmentCentroidMap.end(); ++fragIter)
		  {
			  const pandora::Cluster* pFragment = fragIter->first;
			  TVector3 fragmentCentroid = fragIter->second;

			  TVector3 cluFragDirection = clusterCentroid - fragmentCentroid;

			  if(cluFragDirection.Mag() < 500.)
		
			  {
				  nearbyFragments.insert(pFragment);
			  }
		  }

		  clusterNearbyFragments.insert(std::pair<const pandora::Cluster*, pandora::ClusterList>(pCluster, nearbyFragments));
	  }

	  for(auto iter = clusterNearbyFragments.begin(); iter != clusterNearbyFragments.end(); ++iter)
	  {
		 const pandora::Cluster* pCluster = iter->first;
		 pandora::ClusterList fragments = iter->second; 

		 std::cout << "=====> cluster : " << pCluster->GetHadronicEnergy() << std::endl;

		 for(pandora::ClusterList::iterator fragIter = fragments.begin(); fragIter != fragments.end(); ++fragIter)
		 {
			 const pandora::Cluster* pFragment = *fragIter;
		 
		 }
	  }

	  return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------
 
  TVector3 FragmentsFromIsoHitsMergingAlgorithm::GetClustersAxis(const pandora::Cluster* pCluster)
  {
	  float minHitLayer, clusterVol, energyRatio, hitOutsideRatio, axisLengthRatio, shortAxisLengthRatio;
	  TVector3 axis;

      ClusterPropertiesHelper::CalcClusterProperties(pCluster, minHitLayer, clusterVol, energyRatio, 
		                               hitOutsideRatio, axisLengthRatio, shortAxisLengthRatio, axis);

	  return axis;
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
