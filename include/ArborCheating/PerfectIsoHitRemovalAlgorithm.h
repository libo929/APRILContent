/**
 *  @file   ArborContent/include/ArborCheating/PerfectIsoHitRemovalAlgorithm.h
 * 
 *  @brief  Header file for the cheating clustering algorithm class.
 * 
 *  $Log: $
 *  @author J. Marshall.
 *  Copied from https://github.com/PandoraPFA/LCContent sources
 */
#ifndef PERFECTISOHITREMOVALALGORITHM_H
#define PERFECTISOHITREMOVALALGORITHM_H 1

#include "Pandora/Algorithm.h"

namespace arbor_content
{

/**
 *  @brief PerfectIsoHitRemovalAlgorithm class
 */
class PerfectIsoHitRemovalAlgorithm : public pandora::Algorithm
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
    PerfectIsoHitRemovalAlgorithm();

protected:
    //virtual bool SelectMCParticlesForClustering(const pandora::MCParticle *const pMCParticle) const;

private:
    pandora::StatusCode Run();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    typedef std::map<const pandora::MCParticle*, pandora::ClusterList*> MCParticleToClusterListMap;

    /**
     *  @brief  Simple mc particle collection, using main mc particle associated with each calo hit
     * 
     *  @param  pCaloHit address of the calo hit
     *  @param  mcParticleToHitListMap the mc particle to hit list map
     */
    void SimpleMCParticleCollection(const pandora::Cluster* const pCaloHit, MCParticleToClusterListMap &mcParticleToClusterListMap) const;

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
    void AddToClusterListMap(const pandora::Cluster *const pClusterToAdd, const pandora::MCParticle *const pMCParticle, MCParticleToClusterListMap &mcParticleToClusterListMap) const;

    /**
     *  @brief  Create clusters based on information in the mc particle to hit list map
     * 
     *  @param  mcParticleToHitListMap the mc particle to hit list map
     */
	pandora::StatusCode MergeClusters(const MCParticleToClusterListMap &mcParticleToClusterListMap) const;

    //pandora::IntVector  m_particleIdList;               ///< list of particle ids of MCPFOs to be selected
    //bool                m_shouldUseOnlyECalHits;        ///< Whether to only use ecal hits in the clustering algorithm
    //bool                m_shouldUseIsolatedHits;        ///< Whether to use isolated hits in the clustering algorithm
    //bool                m_simpleMCParticleCollection;   ///< Whether to use simple mc particle collection mechanism, or full mechanism
    //float               m_minWeightFraction;            ///< The minimum mc particle calo hit weight for clustering consideration
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *PerfectIsoHitRemovalAlgorithm::Factory::CreateAlgorithm() const
{
    return new PerfectIsoHitRemovalAlgorithm();
}

}

#endif  // PERFECTFRAGMENTREMOVALALGORITHM_H 
