/**
 *  @file   APRILContent/include/APRILCheating/PerfectChargedClusterSeparationAlgorithm.h
 * 
 *  @brief  Header file for the cheating clustering algorithm class.
 * 
 *  $Log: $
 *  @author J. Marshall.
 *  Copied from https://github.com/PandoraPFA/LCContent sources
 */
#ifndef PERFECTCHARGEDCLUSTERSEPARATIONALGORITHM_H
#define PERFECTCHARGEDCLUSTERSEPARATIONALGORITHM_H 1

#include "Pandora/Algorithm.h"

namespace april_content
{

/**
 *  @brief PerfectChargedClusterSeparationAlgorithm class
 */
class PerfectChargedClusterSeparationAlgorithm : public pandora::Algorithm
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
    PerfectChargedClusterSeparationAlgorithm();

protected:

private:
    pandora::StatusCode Run();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    //typedef std::map<const pandora::MCParticle*, pandora::CaloHitList*> MCParticleToCaloHitListMap;
    //typedef std::map<const pandora::MCParticle*, const pandora::Cluster*> MCParticleToClusterMap;
    //typedef std::map<const pandora::CaloHit*, const pandora::Cluster*> CaloHitToClusterMap;
    typedef std::map<const pandora::MCParticle*, pandora::ClusterList> MCParticleToClusterListMap;
    typedef std::map<const pandora::MCParticle*, pandora::CaloHitList> MCParticleToCaloHitListMap;

    //void AddToCaloHitListMap(const pandora::CaloHit* const pCaloHitToAdd, 
	//	 const pandora::MCParticle *const pMCParticle, MCParticleToCaloHitListMap &mcParticleToCaloHitListMap) const;

    //void AddToClusterListMap(const pandora::Cluster* const pClusterToAdd, 
	//	 const pandora::MCParticle *const pMCParticle, MCParticleToClusterListMap &mcParticleToClusterListMap) const;
  
	//void ClearCaloHitListMap(MCParticleToCaloHitListMap &mcParticleToCaloHitListMap) const;
  
	//void CreateNeutralClusters(MCParticleToCaloHitListMap& mcParticleToCaloHitListMap) const;
  
	//void RemoveClusterCaloHitAssociations(CaloHitToClusterMap& caloHitToClusterRemoveMap) const;

	//void AddClusterCaloHitAssociations(CaloHitToClusterMap& caloHitToClusterAddMap) const;

	pandora::StatusCode ClusterSeparation(pandora::ClusterList& localClusterList, MCParticleToCaloHitListMap& mcParticleToCaloHitListMap);

    void AddToClusterMap(const pandora::CaloHit* pCaloHit,  const pandora::MCParticle* pMCParticle, 
			            MCParticleToCaloHitListMap& mcParticleToCaloHitListMap);
  
	pandora::StatusCode CreateClusters(pandora::ClusterList& clusterList, MCParticleToCaloHitListMap& mcParticleToCaloHitListMap) const;

};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *PerfectChargedClusterSeparationAlgorithm::Factory::CreateAlgorithm() const
{
    return new PerfectChargedClusterSeparationAlgorithm();
}

}

#endif  // PERFECTCHARGEDCLUSTERSEPARATIONALGORITHM_H 
