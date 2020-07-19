/**
 *  @file   APRILContent/include/APRILCheating/PerfectChargedHitRecoveryAlgorithm.h
 * 
 *  @brief  Header file for the cheating clustering algorithm class.
 * 
 *  $Log: $
 *  @author J. Marshall.
 *  Copied from https://github.com/PandoraPFA/LCContent sources
 */
#ifndef PERFECTCHARGEDHITRECOVERYALGORITHM_H
#define PERFECTCHARGEDHITRECOVERYALGORITHM_H 1

#include "Pandora/Algorithm.h"

namespace april_content
{

/**
 *  @brief PerfectChargedHitRecoveryAlgorithm class
 */
class PerfectChargedHitRecoveryAlgorithm : public pandora::Algorithm
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
    PerfectChargedHitRecoveryAlgorithm();

protected:

private:
    pandora::StatusCode Run();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    typedef std::map<const pandora::MCParticle*, pandora::CaloHitList*> MCParticleToCaloHitListMap;
    //typedef std::map<const pandora::MCParticle*, const pandora::Cluster*> MCParticleToClusterMap;
    typedef std::map<const pandora::MCParticle*, pandora::ClusterList*> MCParticleToClusterListMap;
    typedef std::map<const pandora::CaloHit*, const pandora::Cluster*> CaloHitToClusterMap;

    /**
     *  @brief  Simple mc particle collection, using main mc particle associated with each calo hit
     * 
     *  @param  pCaloHit address of the calo hit
     *  @param  mcParticleToHitListMap the mc particle to hit list map
     */
    void SimpleMCParticleCaloHitListCollection(const pandora::CaloHit* const pCaloHit, 
			                                   MCParticleToCaloHitListMap &mcParticleToCaloHitListMap) const;

    //void SimpleMCParticleClusterCollection(const pandora::Cluster* const pCluster, MCParticleToClusterMap &mcParticleToClusterMap) const;
  
	void SimpleMCParticleClusterListCollection(const pandora::Cluster *const pCluster, 
			MCParticleToClusterListMap &mcParticleToClusterListMap) const;

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

    void AddToClusterListMap(const pandora::Cluster* const pClusterToAdd, 
		 const pandora::MCParticle *const pMCParticle, MCParticleToClusterListMap &mcParticleToClusterListMap) const;
  
	void ClearCaloHitListMap(MCParticleToCaloHitListMap &mcParticleToCaloHitListMap) const;
  
	void CreateChargedClusters(MCParticleToCaloHitListMap& mcParticleToCaloHitListMap) const;
  
	void RemoveClusterCaloHitAssociations(CaloHitToClusterMap& caloHitToClusterRemoveMap) const;

	void AddClusterCaloHitAssociations(CaloHitToClusterMap& caloHitToClusterAddMap) const;

    /**
     *  @brief  Create clusters based on information in the mc particle to hit list map
     * 
     *  @param  mcParticleToHitListMap the mc particle to hit list map
     */

	//pandora::StatusCode MergeCaloHits(const MCParticleToCaloHitListMap &mcParticleToCaloHitListMap,
	//		                          const MCParticleToClusterMap &mcParticleToClusterMap) const;

};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *PerfectChargedHitRecoveryAlgorithm::Factory::CreateAlgorithm() const
{
    return new PerfectChargedHitRecoveryAlgorithm();
}

}

#endif  // PERFECTCHARGEDHITRECOVERYALGORITHM_H
