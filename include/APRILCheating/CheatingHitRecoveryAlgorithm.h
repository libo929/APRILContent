/**
 *  @file   APRILContent/include/APRILCheating/CheatingHitRecoveryAlgorithm.h
 * 
 *  @brief  Header file for the cheating cluster cleaning algorithm class.
 * 
 *  $Log: $
 */
#ifndef APRIL_CHEATING_HIT_RECOVERY_ALGORITHM_H
#define APRIL_CHEATING_HIT_RECOVERY_ALGORITHM_H 1

#include "Pandora/Algorithm.h"

namespace april_content
{
typedef std::map<const pandora::MCParticle* const, pandora::CaloHitList> MCPCaloHitListMap;
typedef std::map<const pandora::MCParticle* const, pandora::ClusterList> MCPClusterListMap;
typedef std::map<const pandora::Cluster* const,    pandora::CaloHitList> ClusterCaloHitListMap;

/**
 *  @brief CheatingHitRecoveryAlgorithm class
 */
class CheatingHitRecoveryAlgorithm : public pandora::Algorithm
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
    pandora::StatusCode Run();

    pandora::StatusCode MakeMCPClustersAssociation(MCPClusterListMap& mcpClusterListMap);
    pandora::StatusCode MakeMCPHitsAssociation(MCPCaloHitListMap& mcpCaloHitListMap);
    pandora::StatusCode AddHitToClusterByMCP(MCPClusterListMap& mcpClusterListMap, MCPCaloHitListMap& mcpCaloHitListMap);
    pandora::StatusCode AddHitsToNewClusters(MCPCaloHitListMap& mcpCaloHitListMap);

    pandora::StatusCode MakeClusterHitsAssociation(ClusterCaloHitListMap& clusterCaloHitListMap);
    pandora::StatusCode AddHitToCluster(ClusterCaloHitListMap& clusterCaloHitListMap);

	pandora::StatusCode MergeClusters();

    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

	bool m_shouldUseMCRecovery;
	bool m_shouldUseMCMerge;
    bool m_shouldUseRecovery;

	unsigned int m_nNeighborHits;

	float m_maxHitsDistance;

    std::string  m_mergedClusterListName;
};

inline pandora::Algorithm *CheatingHitRecoveryAlgorithm::Factory::CreateAlgorithm() const
{
    return new CheatingHitRecoveryAlgorithm();
}

} // namespace APRIL_content

#endif 
