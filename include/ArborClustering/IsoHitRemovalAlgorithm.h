/**
 *  @file   ArborContent/include/ArborCheating/IsoHitRemovalAlgorithm.h
 * 
 *  @brief  Header file for the cheating clustering algorithm class.
 * 
 *  $Log: $
 *  @author J. Marshall.
 *  Copied from https://github.com/PandoraPFA/LCContent sources
 */
#ifndef ISOHITREMOVALALGORITHM_H
#define ISOHITREMOVALALGORITHM_H 1

#include "Pandora/Algorithm.h"
#include "ArborUtility/KDTreeLinkerAlgoT.h"

typedef arbor_content::KDTreeLinkerAlgo<const pandora::CaloHit*, 4> HitKDTree;
typedef arbor_content::KDTreeNodeInfoT<const pandora::CaloHit*, 4> HitKDNode;
typedef map<const pandora::CaloHit*, const pandora::Cluster*> HitsToClustersMap;

namespace arbor_content
{

/**
 *  @brief IsoHitRemovalAlgorithm class
 */
class IsoHitRemovalAlgorithm : public pandora::Algorithm
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
    IsoHitRemovalAlgorithm();


protected:

private:
    pandora::StatusCode Run();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    typedef std::map<const pandora::MCParticle*, pandora::CaloHitList*> MCParticleToCaloHitListMap;
    typedef std::map<const pandora::MCParticle*, const pandora::Cluster*> MCParticleToClusterMap;

    /**
     *  @brief  Simple mc particle collection, using main mc particle associated with each calo hit
     * 
     *  @param  pCaloHit address of the calo hit
     *  @param  mcParticleToHitListMap the mc particle to hit list map
     */
    void SimpleMCParticleCaloHitListCollection(const pandora::CaloHit* const pCaloHit, 
			                                   MCParticleToCaloHitListMap &mcParticleToCaloHitListMap);

    void SimpleMCParticleClusterCollection(const pandora::Cluster* const pCluster, MCParticleToClusterMap &mcParticleToClusterMap);

    /**
     *  @brief  Full mc particle collection, using map of mc particles to hit weights; fragment calo hits where necessary
     * 
     *  @param  pCaloHit address of the calo hit
     *  @param  mcParticleToHitListMap the mc particle to hit list map
     */
    //void FullMCParticleCollection(const pandora::CaloHit *const pCaloHit, MCParticleToHitListMap &mcParticleToHitListMap) const;

    /**
     *  @brief  Add a calo hit to the mc particle to hit list map
     * 
     *  @param  pCaloHit address of the calo hit
     *  @param  pMCParticle address of the mc particle
     *  @param  mcParticleToHitListMap the mc particle to hit list map
     */

    void AddToCaloHitListMap(const pandora::CaloHit* const pCaloHitToAdd, 
		 const pandora::MCParticle *const pMCParticle, MCParticleToCaloHitListMap &mcParticleToCaloHitListMap) const;

    void AddToClusterMap(const pandora::Cluster* const pClusterToAdd, 
		 const pandora::MCParticle *const pMCParticle, MCParticleToClusterMap &mcParticleToClusterMap) const;

    /**
     *  @brief  Create clusters based on information in the mc particle to hit list map
     * 
     *  @param  mcParticleToHitListMap the mc particle to hit list map
     */


	void CreateCluster(const pandora::CaloHitList *const caloHitList) const;

	int GetMCParticle(const pandora::Cluster* pCluster) const;

	pandora::StatusCode Test();
	pandora::StatusCode GetIsoHits(pandora::CaloHitList& caloHitList, MCParticleToCaloHitListMap& mcParticleToCaloHitListMap);
    pandora::StatusCode GetClusters(pandora::ClusterList& clusterList, MCParticleToClusterMap& mcParticleToClusterMap);
  
	pandora::StatusCode MCMergeCaloHits(const MCParticleToCaloHitListMap &mcParticleToCaloHitListMap,
			                          const MCParticleToClusterMap &mcParticleToClusterMap) const;

	pandora::StatusCode MergeCaloHits(const pandora::CaloHitList& isoHitList, const pandora::ClusterList& clusterList);
  
	pandora::StatusCode BuildKDTree(HitKDTree& hits_kdtree);
  
	pandora::StatusCode SearchNearbyCaloHits(const pandora::CaloHit* pCaloHit, std::vector<const pandora::CaloHit*>& nearbyHits,
			                                 float wideX, float wideY, float wideZ, int layers);

    //bool                m_shouldUseOnlyECalHits;        ///< Whether to only use ecal hits in the clustering algorithm
    //bool                m_shouldUseIsolatedHits;        ///< Whether to use isolated hits in the clustering algorithm
    //bool                m_simpleMCParticleCollection;   ///< Whether to use simple mc particle collection mechanism, or full mechanism
    //float               m_minWeightFraction;            ///< The minimum mc particle calo hit weight for clustering consideration
	//int                 m_CaloHitMCGetterFailures;
	
	HitKDTree*            m_hitsKDTree;
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *IsoHitRemovalAlgorithm::Factory::CreateAlgorithm() const
{
    return new IsoHitRemovalAlgorithm();
}

}

#endif  // ISOHITREMOVALALGORITHM_H
