  /// \file ClusterHelper.h
/*
 *
 * ClusterHelper.h header template automatically generated by a class generator
 * Creation date : ven. avr. 10 2015
 *
 * This file is part of ArborContent libraries.
 * 
 * ArborContent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 * 
 * ArborContent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ArborContent.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * @author Remi Ete
 * @copyright CNRS , IPNL
 */


#ifndef CLUSTERHELPER_H
#define CLUSTERHELPER_H

#include "Pandora/PandoraInternal.h"
#include "Pandora/PandoraInputTypes.h"
#include "ArborApi/ArborInputTypes.h"
#include "Helpers/ClusterFitHelper.h"

namespace arbor_content
{

/**
 *  @brief  ClusterPca class
 */
class ClusterPca
{
public:
	/**
	 *  @brief  Constructor
	 *
	 *  @param  pCluster the input cluster address to perform pca on
	 */
	ClusterPca(const pandora::Cluster *const pCluster);

	/**
	 *  @brief  Get the eigen value for a given component. Components are sorted by descending order
	 *
	 *  @param  component the pca component
	 *
	 *  @return the eigen value for the given component
	 */
	float GetEigenValue(int component) const;

	/**
	 *  @brief  Get the eigen vector for a given component. Components are sorted by descending order
	 *
	 *  @param  component the pca component
	 *
	 *  @return the eigen vector for the given component
	 */
	const pandora::CartesianVector &GetEigenVector(int component) const;

	/**
	 *  @brief  Get the transverse ratio of the cluster
	 */
	float GetTransverseRatio() const;

	/**
	 *  @brief  Get the input cluster
	 */
	const pandora::Cluster *GetCluster() const;

private:
	/**
	 *  @brief  Component struct
	 */
	struct Component
	{
		/**
		 *  @brief  Contructor
		 */
		Component();

		float                         m_eigenValue;    ///< The eigen value
		pandora::CartesianVector      m_eigenVector;   ///< The eigen vector
	};

	Component                   m_components[3];      ///< The eigen values and vectors for each component
	float                       m_transverseRatio;    ///< The cluster transverse ratio
	const pandora::Cluster     *m_pCluster;           ///< The inout cluster
};

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

/** 
 *  @brief  ClusterHelper class
 */ 
class ClusterHelper 
{
public:
	/**
	 *  @brief  Get the cluster centroid
	 *
	 *  @param  pCluster the cluster to evaluate the centroid
	 *  @param  centroid the centroid to receive
	 */
	static pandora::StatusCode GetCentroid(const pandora::Cluster *const pCluster, pandora::CartesianVector &centroid);

	/**
	 *  @brief  Get the closest distance approach between all the hits of the cluster and the given position vector
	 *
	 *  @param  pCluster the cluster to evaluate the closest distance approach
	 *  @param  point the position vector
	 *  @param  closestDistance the closest distance approach between the point and the cluster, unit mm
	 */
	static pandora::StatusCode GetClosestDistanceApproach(const pandora::Cluster *const pCluster, const pandora::CartesianVector &point,
			float &closestDistance, bool onlyUseConnectedHit = true);

	/**
	 *  @brief  Get the distance between the cluster centroid and the given position vector
	 *
	 *  @param  pCluster the cluster to evaluate the closest distance approach
	 *  @param  point the position vector
	 *  @param  centroidDistance the distance between the point and the cluster centroid, unit mm
	 */
	static pandora::StatusCode GetCentroidDistance(const pandora::Cluster *const pCluster, const pandora::CartesianVector &point,
			float &centroidDistance);
  
	static float GetHadronicEnergyInECAL(const pandora::Cluster *const pCluster);
    static float GetEnergyRatio(const pandora::OrderedCaloHitList& orderedCaloHitList);
    static pandora::StatusCode GetEnergyRatio(const pandora::OrderedCaloHitList& orderedCaloHitList, float& energyRatio, unsigned int& nHits);

    static pandora::HitRegion GetRegion(const pandora::Cluster *const pCluster);
  
	static float GetElectromagneticEnergyInECAL(const pandora::Cluster *const pCluster);
  
	static pandora::StatusCode GetClusterVolume(const pandora::Cluster *const pCluster, float& clusterVolume);
  
	static pandora::StatusCode FitStart(const pandora::Cluster *const pCluster, const unsigned int maxOccupiedLayers, pandora::ClusterFitResult &clusterFitResult);
  
	static pandora::StatusCode FitFullCluster(const pandora::Cluster *const pCluster, pandora::ClusterFitResult &clusterFitResult, bool useMainCluster = false, float eps = 50.);
  
	static pandora::StatusCode GetMainClusterHits(const pandora::Cluster *const pCluster, pandora::CaloHitList& mainClusterHits);
  
	static pandora::OrderedCaloHitList GetOrderedConnectedCaloHitList(const pandora::Cluster *const pCluster);
	
    static float GetClusterAxisStartingPointAngle(const pandora::Cluster *const pCluster);

	/**
	 *  @brief  Get the closest distance approach between all the hits of the clusters
	 *
	 *  @param  pCluster the first cluster
	 *  @param  pCluster the second cluster
	 *  @param  closestDistance the closest distance approach between the two clusters, unit mm
	 */
	static pandora::StatusCode GetClosestDistanceApproach(const pandora::Cluster *const pCluster1, const pandora::Cluster *const pCluster2,
			float &closestDistance, bool onlyUseConnectedHit = true);

	static pandora::StatusCode GetTrackClusterDistance(const pandora::TrackState *const pTrackState, const pandora::Cluster *const pCluster,
	const unsigned int maxSearchLayer, const float parallelDistanceCut, const float minTrackClusterCosAngle, float &trackClusterDistance);

	/**
	 *  @brief  Get the number of calo hit seeds in the cluster
	 *
	 *  @param  pCluster the cluster to get the number of calo hit seeds
	 *  @param  nSeeds the number of seeds to receive
	 */
	static pandora::StatusCode GetNCaloHitSeeds(const pandora::Cluster *const pCluster, unsigned int &nSeeds);

	/**
	 *  @brief  Whether the cluster is leaving the detector
	 *
	 *  @param  pandora the pandora instance to access internal content
	 *  @param  pCluster the cluster address
	 *  @param  nOuterLayersToExamine the number of outer layers of the cluster to examine
	 *  @param  maxDistanceToDetectorEdge the distance to the detector edge for a target calo hit
	 *  @param  minNHitsNearEdges the minimum number of calo hit near the detector edge to consider the cluster as leaving the detector
	 */

	 static bool IsClusterLeavingDetector(const pandora::Cluster *const pCluster, const unsigned int nOuterLayersToExamine = 4, 
			 const unsigned int nMipLikeOccupiedLayers = 4, const unsigned int nShowerLikeOccupiedLayers = 3, 
			 const float showerLikeEnergyInOuterLayers = 1.f);

	 static bool ContainsHitInOuterSamplingLayer(const pandora::Cluster *const pCluster);

	static bool IsClusterLeavingDetector(const pandora::Pandora &pandora, const pandora::Cluster *const pCluster, unsigned int nOuterLayersToExamine = 3,
			float maxDistanceToDetectorEdge = 50.f, unsigned int minNHitsNearEdges = 3);

	/**
	 *  @brief  Whether the cluster contains at least one hit of target type
	 *
	 *  @param  pCluster to check
	 *  @param  hitType the hit type to look for
	 */
	static bool ContainsHitType(const pandora::Cluster *const pCluster, const pandora::HitType hitType);

	/**
	 *  @brief  Get the mean surrounding energy of the cluster
	 *
	 *  @param  pCluster the input cluster
	 *  @param  meanSurroundingEnergy the mean surrounding energy to receive
	 */
	static pandora::StatusCode GetMeanSurroundingEnergy(const pandora::Cluster *const pCluster, float &meanSurroundingEnergy);

	/**
	 *  @brief  Get the mean density of the cluster
	 *
	 *  @param  pCluster the input cluster
	 *  @param  meanDensity the mean density to receive
	 */
	static pandora::StatusCode GetMeanDensity(const pandora::Cluster *const pCluster, float &meanDensity);
    static float GetMeanDensity(const pandora::Cluster *const pCluster);

	/**
	 *  @brief  Evaluate the chi before and after to merge the two clusters
	 *
	 *  @param  pandora the pandora instance to access internal content
	 *  @param  pClusterToEnlarge the cluster to enlarge
	 *  @param  pClusterToMerge the cluster to merge
	 *  @param  oldChi the chi of the cluster to enlarge
	 *  @param  newChi the chi of the clusters after a potential merging
	 */
	static pandora::StatusCode GetChiClusterMerging(const pandora::Pandora &pandora, const pandora::Cluster *const pClusterToEnlarge, const pandora::Cluster *const pClusterToMerge, float& trackEnergy, float &oldChi, float &newChi);

	/**
	 *  @brief  Perform intelligent merging of clusters (daughter <-> parent relationship management)
	 *
	 *  @param  algorithm the algorithm that requests the merging
	 *  @param  clusterToClusterMap the mapping of clusters to merge (daughter -> parent)
	 */
	static pandora::StatusCode MergeClusters(const pandora::Algorithm &algorithm, ClusterToClusterMap &clusterToClusterMap);

	/**
	 *  @brief  Get the track cluster distance
	 *
	 *  @param  pandora the pandora instance to access internal content
	 *  @param  pCluster the input cluster
	 *  @param  pTrack the input track
	 *  @param  maxTransverseDistance the maximum distance between cluster hits and the track helix
	 *  @param  trackClusterDistance the track cluster distance to receive
	 */
	static pandora::StatusCode GetTrackClusterDistance(const pandora::Pandora &pandora, const pandora::Cluster *const pCluster, const pandora::Track *const pTrack, const float maxTransverseDistance, float &trackClusterDistance);

        /**
         *  @brief  Whether a linear fit to a cluster crosses a registered gap region. Only the region between the startlayer and
         *          endlayer is considered in the fit and in the comparison with registered gap regions.
         *
         *  @param  pandora the associated pandora instance
         *  @param  pCluster address of the cluster
         *  @param  startLayer the start layer (adjusted to maximum of specified layer and cluster inner layer)
         *  @param  endLayer the end layer (adjusted to minimum of specified layer and cluster outer layer)
         *  @param  pDetectorGap the crossing gap to receive, if any
         *  @param  nSamplingPoints number of points at which to sample the fit within the specified layer region
         *
         *  @return boolean
         */
        static bool DoesClusterCrossGapRegion(const pandora::Pandora &pandora, const pandora::Cluster *const pCluster, const unsigned int startLayer,
            const unsigned int endLayer, const pandora::DetectorGap *&pDetectorGap, const unsigned int nSamplingPoints = 50);

        /**
         *  @brief  Whether a linear fit crosses a registered gap region. The fit will be propagated through the specified distance
         *          from its closest approach to the startPosition. Within this propagation, the fit will be sampled a specified number
         *          of times and the resulting position compared with registered gap regions.
         *
         *  @param  pandora the associated pandora instance
         *  @param  clusterFitResult the cluster fit result
         *  @param  startPosition the propagation start position (adjusted to closest point on fit trajectory)
         *  @param  propagationDistance the propagation distance, which can be negative for propagation towards the ip
         *  @param  pDetectorGap the detector gap to receive, if any
         *  @param  nSamplingPoints number of points at which to sample the fit within its propagation
         *
         *  @return boolean
         */
        static bool DoesFitCrossGapRegion(const pandora::Pandora &pandora, const pandora::ClusterFitResult &clusterFitResult,
            const pandora::CartesianVector &startPosition, const float propagationDistance, const pandora::DetectorGap *&pDetectorGap, const unsigned int nSamplingPoints = 50);

        /**
         *  @brief  Get the distance to detector gap
         *
         *  @param  positionVector the position in space to test
         *  @param  pDetectorGap the detector gap to consider
         *  @param  distanceToGap the distance to detector gap to receive
         */
        static pandora::StatusCode GetDistanceToDetectorGap(const pandora::CartesianVector &positionVector, const pandora::DetectorGap *const pDetectorGap, float &distanceToGap);

        /**
         *  @brief  Get the calo hit list belonging to the cluster, close to all registered detector gaps
         *
         *  @param  pandora the pandora instance to access pandora content
         *  @param  pCluster the input cluster address
         *  @param  maxDistance the maximum distance between the gap and a calo hit
         *  @param  caloHitList the list of calo hit near a detector gap
         *  @param  shouldUseIsolatedHits whether to consider isolated calo hits from the input cluster
         */
        static pandora::StatusCode GetCaloHitsNearDetectorGaps(const pandora::Pandora &pandora, const pandora::Cluster *const pCluster, const float maxDistanceFine,
            const float maxDistanceCoarse, pandora::CaloHitList &caloHitList, bool shouldUseIsolatedHits);

        /**
         *  @brief  Get the calo hit list belonging to the cluster, close to all registered detector gaps
         *
         *  @param  pandora the pandora instance to access pandora content
         *  @param  pCluster the input cluster address
         *  @param  pDetectorGap the detector gap to consider
         *  @param  maxDistance the maximum distance between the gap and a calo hit
         *  @param  caloHitList the list of calo hit near a detector gap
         *  @param  shouldUseIsolatedHits whether to consider isolated calo hits from the input cluster
         */
        static pandora::StatusCode GetCaloHitsNearDetectorGap(const pandora::Pandora &pandora, const pandora::Cluster *const pCluster, const pandora::DetectorGap *const pDetectorGap, const float maxDistanceFine,
            const float maxDistanceCoarse, pandora::CaloHitList &caloHitList, bool shouldUseIsolatedHits);

        /**
         *
         */
        static pandora::StatusCode CleanAndDeleteCluster(const pandora::Algorithm &algorithm, const pandora::Cluster *const pCluster);

        /**
         *
         */
        static pandora::StatusCode CleanAndDeleteClusters(const pandora::Algorithm &algorithm, const pandora::ClusterList &clusterList);
  
		static bool CanMergeCluster(const pandora::Pandora &pandora, const pandora::Cluster *const pCluster, const float minMipFraction, 
				const float maxAllHitsFitRms);
  
		static float GetAverageTime(const pandora::Cluster *const pCluster, bool onlyForECALHit = false);
        static float GetMeanHitPerLayer(const pandora::Cluster *const pCluster);

        static pandora::StatusCode GetMainClusterHits(const pandora::Cluster *const pCluster, pandora::CaloHitList& mainClusterHits, float eps);

        static pandora::StatusCode GetRMS(const pandora::Cluster *const pCluster, pandora::CartesianVector cog, pandora::CartesianVector axis, float& rms1, float& rms2);
		static pandora::StatusCode GetRMS(const pandora::CaloHitList& clusterHits, pandora::CartesianVector cog, pandora::CartesianVector axis, float& rms1, float& rms2);
  
		static unsigned int GetClusterConnectorNumber(const pandora::Cluster *const pCluster);
};

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

inline float ClusterPca::GetEigenValue(int component) const
{
	if(component < 0 || component > 2)
		throw pandora::StatusCodeException(pandora::STATUS_CODE_INVALID_PARAMETER);

	return m_components[component].m_eigenValue;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline const pandora::CartesianVector &ClusterPca::GetEigenVector(int component) const
{
	if(component < 0 || component > 2)
		throw pandora::StatusCodeException(pandora::STATUS_CODE_INVALID_PARAMETER);

	return m_components[component].m_eigenVector;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline float ClusterPca::GetTransverseRatio() const
{
	return m_transverseRatio;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline const pandora::Cluster *ClusterPca::GetCluster() const
{
	return m_pCluster;
}

} 

#endif  //  CLUSTERHELPER_H
