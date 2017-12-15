/**
 *
 *  @file   ArborContent/include/ArborClustering/FragmentsFromIsoHitsMergingAlgorithm.h
 * 
 */
#ifndef FRAGMENTSFROMISOHITSMERGINGALGORITHM_H
#define FRAGMENTSFROMISOHITSMERGINGALGORITHM_H 1

#include "Pandora/Algorithm.h"
#include "ArborUtility/KDTreeLinkerAlgoT.h"

#include "TVector3.h"

typedef arbor_content::KDTreeLinkerAlgo<const pandora::CaloHit*, 4> HitKDTree;
typedef arbor_content::KDTreeNodeInfoT<const pandora::CaloHit*, 4> HitKDNode;
typedef map<const pandora::CaloHit*, const pandora::Cluster*> HitsToClustersMap;

namespace arbor_content
{

/**
 *  @brief FragmentsFromIsoHitsMergingAlgorithm class
 */

class FragmentsFromIsoHitsMergingAlgorithm : public pandora::Algorithm
{
public:
    /**
     *  @brief  Factory class for instantiating algorithm
     */
    class Factory : public pandora::AlgorithmFactory
    {
    public:
        pandora::Algorithm *CreateAlgorithm() const;
    };

    /**
     *  @brief Default constructor
     */
    FragmentsFromIsoHitsMergingAlgorithm();


private:
    pandora::StatusCode Run();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    typedef std::map<const pandora::MCParticle*, pandora::CaloHitList*> MCParticleToCaloHitListMap;
    typedef std::map<const pandora::MCParticle*, const pandora::Cluster*> MCParticleToClusterMap;

	void CreateCluster(const pandora::CaloHitList *const caloHitList) const;

	pandora::StatusCode GetClusters(pandora::ClusterList& clusterList, const std::string& listName);
  
	pandora::StatusCode BuildKDTree(HitKDTree& hits_kdtree);

	TVector3 GetClustersAxis(const pandora::Cluster* pCluster);
  
	pandora::StatusCode SearchNearbyCaloHits(const pandora::CaloHit* pCaloHit, 
			std::vector<const pandora::CaloHit*>& nearbyHits, float wideX, float wideY, float wideZ, int layers);

    pandora::StatusCode MergeFragments(pandora::ClusterList& mainClusterList, pandora::ClusterList& clustersFromIsoHits);

private:
	float                 m_kernelBandwidth;
	float                 m_clusterEpsilon;
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *FragmentsFromIsoHitsMergingAlgorithm::Factory::CreateAlgorithm() const
{
    return new FragmentsFromIsoHitsMergingAlgorithm();
}

}

#endif  // FRAGMENTSFROMISOHITSMERGINGALGORITHM_H
