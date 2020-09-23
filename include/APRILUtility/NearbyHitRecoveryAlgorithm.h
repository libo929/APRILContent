/**
 *  @file   APRILContent/include/APRILCheating/NearbyHitRecoveryAlgorithm.h
 * 
 *  @brief  Header file for the cheating cluster cleaning algorithm class.
 * 
 *  $Log: $
 */
#ifndef NEARBY_HIT_RECOVERY_ALGORITHM_H
#define NEARBY_HIT_RECOVERY_ALGORITHM_H 1

#include "Pandora/Algorithm.h"

#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"

namespace april_content
{
typedef std::map<const pandora::MCParticle* const, pandora::CaloHitList> MCPCaloHitListMap;
typedef std::map<const pandora::MCParticle* const, pandora::ClusterList> MCPClusterListMap;
typedef std::map<const pandora::Cluster* const,    pandora::CaloHitList> ClusterCaloHitListMap;

struct ClusterParamatersPlus
{
	// mean density
	// surrouding energy
	// COG
    ClusterParamatersPlus(const pandora::Cluster* pCluster, float density, float energy)
		:cluster(pCluster), meanDensity(density), surroudingEnergy(energy)
	{
	}
		
	const pandora::Cluster* cluster;
	float meanDensity;
	float surroudingEnergy;
};

/**
 *  @brief NearbyHitRecoveryAlgorithm class
 */
class NearbyHitRecoveryAlgorithm : public pandora::Algorithm
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

private:
	~NearbyHitRecoveryAlgorithm();
	pandora::StatusCode Initialize();
    pandora::StatusCode Run();

	//
	pandora::StatusCode GetNearbyClustersByDistance(const pandora::CaloHit* pCaloHit, pandora::ClusterList& clusterList);

	// mva
    pandora::StatusCode MakeSamples();
	pandora::StatusCode MVAMergeHitToCluster(ClusterCaloHitListMap& clusterCaloHitListMap);
    float DeterminMergingByMVA(const pandora::Cluster* cluster, const pandora::CaloHit* caloHit);

	// cut based
    pandora::StatusCode MakeClusterHitsAssociation(ClusterCaloHitListMap& clusterCaloHitListMap);
    pandora::StatusCode AddHitToCluster(ClusterCaloHitListMap& clusterCaloHitListMap);

    pandora::StatusCode ClusteringByTool(pandora::AlgorithmTool *pAlgorithmTool);

    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

	//
	unsigned int m_nNeighborHits;

	float m_maxHitsDistance;

    pandora::AlgorithmTool *m_pAlgorithmTool;

	// mva reader
    static TMVA::Reader* m_reader;

	float m_clusterEMEnergy;
	float m_clusterHadEnergy;
	float m_hitEMEnergy;
	float m_hitHadEnergy;
	float m_hitLayer;
	float m_nhits;
	float m_centroidDistance;
	float m_closestDistance;
	float m_meanDensity;
	float m_surroundingEnergy;  

	bool m_useMVA;

};

inline pandora::Algorithm *NearbyHitRecoveryAlgorithm::Factory::CreateAlgorithm() const
{
    return new NearbyHitRecoveryAlgorithm();
}

} // namespace APRIL_content

#endif 
