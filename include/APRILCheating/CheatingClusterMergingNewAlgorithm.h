/**
 *  @file   APRILContent/include/APRILCheating/CheatingClusterMergingNewAlgorithm.h
 * 
 *  @brief  Header file for the cheating cluster cleaning algorithm class.
 * 
 *  $Log: $
 */
#ifndef APRIL_CHEATING_CLUSTER_MERGING_NEW_ALGORITHM_H
#define APRIL_CHEATING_CLUSTER_MERGING_NEW_ALGORITHM_H 1

#include "Pandora/Algorithm.h"

namespace april_content
{
typedef std::map<const pandora::MCParticle* const, pandora::CaloHitList> MCPCaloHitListMap;
typedef std::map<const pandora::MCParticle* const, pandora::ClusterList> MCPClusterListMap;
typedef std::map<const pandora::Cluster* const,    pandora::CaloHitList> ClusterCaloHitListMap;

/**
 *  @brief CheatingClusterMergingNewAlgorithm class
 */
class CheatingClusterMergingNewAlgorithm : public pandora::Algorithm
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

	pandora::StatusCode MergeClusters();

    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    std::string  m_mergedClusterListName;

	bool m_mergePhoton;
   	bool m_mergeCharged;
    bool m_mergeNeutral;
	bool m_mergeNeutralHadron;

	float m_minClusterEnergyToMerge;
};

inline pandora::Algorithm *CheatingClusterMergingNewAlgorithm::Factory::CreateAlgorithm() const
{
    return new CheatingClusterMergingNewAlgorithm();
}

} // namespace APRIL_content

#endif 
