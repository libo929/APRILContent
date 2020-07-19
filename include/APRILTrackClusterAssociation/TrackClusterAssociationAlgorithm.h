/**
 *  @file   LCContent/include/LCTrackClusterAssociation/TrackClusterAssociationAlgorithm.h
 * 
 *  @brief  Header file for the track-cluster association algorithm class.
 * 
 *  $Log: $
 */
#ifndef APRIL_TRACK_CLUSTER_ASSOCIATION_ALGORITHM_H
#define APRIL_TRACK_CLUSTER_ASSOCIATION_ALGORITHM_H 1

#include "Pandora/Algorithm.h"

#include <unordered_map>

namespace april_content
{

template<typename, unsigned int> class KDTreeLinkerAlgo;
template<typename, unsigned int> class KDTreeNodeInfoT;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  TrackClusterAssociationAlgorithm class
 */
class TrackClusterAssociationAlgorithm : public pandora::Algorithm
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

    typedef KDTreeLinkerAlgo<const pandora::CaloHit*, 4> HitKDTree;
    typedef KDTreeNodeInfoT<const pandora::CaloHit*, 4> HitKDNode;
    typedef std::unordered_multimap<std::pair<const pandora::Track*, unsigned int>, const pandora::CaloHit*> TracksToHitsInPseudoLayerMap;
    typedef std::unordered_map<const pandora::CaloHit*, const pandora::Cluster*> HitsToClustersMap;

    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    float           m_lowEnergyCut;                     ///< Energy cut (GeV). Algorithm prefers to associate tracks to high-energy clusters
    float           m_maxTrackClusterDistance;          ///< Max distance between track and cluster to allow association

    unsigned int    m_maxSearchLayer;                   ///< Max pseudo layer to examine when calculating track-cluster distance
    float           m_parallelDistanceCut;              ///< Max allowed projection of track-hit separation along track direction
    float           m_minTrackClusterCosAngle;          ///< Min cos(angle) between track and cluster initial direction
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *TrackClusterAssociationAlgorithm::Factory::CreateAlgorithm() const
{
    return new TrackClusterAssociationAlgorithm();
}

}

#endif 
