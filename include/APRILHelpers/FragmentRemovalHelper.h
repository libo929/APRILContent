/**
 * 
 *  @brief  Header file for the fragment removal helper class.
 * 
 *  $Log: $
 */
#ifndef APRIL_FRAGMENT_REMOVAL_HELPER_H
#define APRIL_FRAGMENT_REMOVAL_HELPER_H 1

#include "Pandora/PandoraInternal.h"
#include "Pandora/StatusCodes.h"

namespace april_content
{

/**
 *  @brief  ClusterContact class, describing the interactions and proximity between parent and daughter candidate clusters
 */
class ClusterContact
{
public:
    /**
     *  @brief  Parameters class
     */
    class Parameters
    {
    public:
        float           m_coneCosineHalfAngle1;         ///< Cosine half angle for first cone comparison in cluster contact object
        float           m_closeHitDistance1;            ///< First distance used to identify close hits in cluster contact object
        float           m_closeHitDistance2;            ///< Second distance used to identify close hits in cluster contact object
        float           m_minCosOpeningAngle;           ///< Min opening angle between two clusters to perform contact hit comparisons
        float           m_distanceThreshold;            ///< Number of calorimeter cell-widths used to identify cluster contact layers
    };

    /**
     *  @brief  Constructor
     * 
     *  @param  pandora the associated pandora instance
     *  @param  pDaughterCluster address of the daughter candidate cluster
     *  @param  pParentCluster address of the parent candidate cluster
     *  @param  parameters the cluster contact parameters
     */
    ClusterContact(const pandora::Pandora &pandora, const pandora::Cluster *const pDaughterCluster, const pandora::Cluster *const pParentCluster,
        const Parameters &parameters);

    /**
     *  @brief  Get the address of the daughter candidate cluster
     * 
     *  @return The address of the daughter candidate cluster
     */
    const pandora::Cluster *GetDaughterCluster() const;

    /**
     *  @brief  Get the address of the parent candidate cluster
     * 
     *  @return The address of the parent candidate cluster
     */
    const pandora::Cluster *GetParentCluster() const;

    /**
     *  @brief  Get the number of contact layers for parent and daughter clusters two clusters
     * 
     *  @return The number of contact layers
     */
    unsigned int GetNContactLayers() const;

    /**
     *  @brief  Get the ratio of the number of contact layers to the number of overlap layers
     * 
     *  @return The ratio of contact layers to overlap layers
     */
    float GetContactFraction() const;

    /**
     *  @brief  Distance between closest hits in parent and daughter clusters, units mm
     * 
     *  @return The distance between closest hits
     */
    float GetDistanceToClosestHit() const;

    /**
     *  @brief  Get the fraction of daughter hits that lie within sepcified distance 1 of parent cluster
     * 
     *  @return The daughter close hit fraction
     */
    float GetCloseHitFraction1() const;

    /**
     *  @brief  Get the fraction of daughter hits that lie within sepcified distance 2 of parent cluster
     * 
     *  @return The daughter close hit fraction
     */
    float GetCloseHitFraction2() const;

    /**
     *  @brief  Get the fraction of daughter hits that lie within specified cone 1 along parent direction
     * 
     *  @return The daughter cone fraction
     */
    float GetConeFraction1() const;

protected:
    /**
     *  @brief  Compare hits in daughter cluster with those in parent cluster to calculate minimum hit separation
     *          and close hit fractions. Calculate these properties in a single loop, for efficiency.
     * 
     *  @param  pDaughterCluster address of the daughter candidate cluster
     *  @param  pParentCluster address of the parent candidate cluster
     *  @param  parameters the cluster contact parameters
     */
    void HitDistanceComparison(const pandora::Cluster *const pDaughterCluster, const pandora::Cluster *const pParentCluster, const Parameters &parameters);

    const pandora::Cluster     *m_pDaughterCluster;         ///< Address of the daughter candidate cluster
    const pandora::Cluster     *m_pParentCluster;           ///< Address of the parent candidate cluster

    unsigned int                m_nContactLayers;           ///< The number of contact layers for parent and daughter clusters
    float                       m_contactFraction;          ///< The ratio of the number of contact layers to the number of overlap layers
    float                       m_coneFraction1;            ///< Fraction of daughter hits that lie within specified cone 1 along parent direction
    float                       m_closeHitFraction1;        ///< Fraction of daughter hits that lie within sepcified distance 1 of parent cluster
    float                       m_closeHitFraction2;        ///< Fraction of daughter hits that lie within sepcified distance 2 of parent cluster
    float                       m_distanceToClosestHit;     ///< Distance between closest hits in parent and daughter clusters, units mm
};

typedef std::vector<ClusterContact> ClusterContactVector;
typedef std::map<const pandora::Cluster *, ClusterContactVector> ClusterContactMap;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  FragmentRemovalHelper class
 */
class FragmentRemovalHelper
{
public:
    /**
     *  @brief  Get the fraction of calo hits in cluster I that lie within a specified distance of a calo hit in cluster J
     * 
     *  @param  pClusterI address of the cluster for which the fraction is calculated
     *  @param  pClusterJ address of the cluster used in the comparison
     *  @param  distanceThreshold the specified distance threshold
     * 
     *  @return The fraction of close calo hits
     */
    static float GetFractionOfCloseHits(const pandora::Cluster *const pClusterI, const pandora::Cluster *const pClusterJ, const float distanceThreshold);

    /**
     *  @brief  Get the fraction of calo hits in cluster I that lie within a cone along the direction of cluster J.
     *          If cluster J has an associated track, it's projected position and direction at the calorimeter will be used to define cone,
     *          otherwise cone will be radial and its axis will pass through centroid of cluster J at its showerstart layer.
     * 
     *  @param  pandora the associated pandora instance
     *  @param  pClusterI address of the cluster for which the fraction is calculated
     *  @param  pClusterJ address of the cluster used in the comparison
     *  @param  coneCosineHalfAngle the cone cosine half angle
     * 
     *  @return The fraction of calo hits in the cone
     */
    static float GetFractionOfHitsInCone(const pandora::Pandora &pandora, const pandora::Cluster *const pClusterI, const pandora::Cluster *const pClusterJ,
        const float coneCosineHalfAngle);

    /**
     *  @brief  Get the fraction of calo hits in a cluster that lie within a cone along the direction of a specified track
     * 
     *  @param  pCluster address of the cluster for which the fraction is calculated
     *  @param  pTrack address of the cluster used in the comparison
     *  @param  coneCosineHalfAngle the cone cosine half angle
     * 
     *  @return The fraction of calo hits in the cone
     */
    static float GetFractionOfHitsInCone(const pandora::Cluster *const pCluster, const pandora::Track *const pTrack, const float coneCosineHalfAngle);

    /**
     *  @brief  Get the fraction of calo hits in a cluster that lie within a specified cone
     * 
     *  @param  pCluster address of the cluster for which the fraction is calculated
     *  @param  coneApex position vector specifying cone apex
     *  @param  coneDirection unit vector specifying cone direction
     *  @param  coneCosineHalfAngle the cone cosine half angle
     * 
     *  @return The fraction of calo hits in the cone
     */
    static float GetFractionOfHitsInCone(const pandora::Cluster *const pCluster, const pandora::CartesianVector &coneApex,
        const pandora::CartesianVector &coneDirection, const float coneCosineHalfAngle);

    /**
     *  @brief  Get the number of pseudo layers crossed by helix in specified range of z coordinates
     * 
     *  @param  pandora the associated pandora instance
     *  @param  helix the helix
     *  @param  zStart start z coordinate
     *  @param  zEnd end z coordinate
     *  @param  nSamplingPoints number of points at which to sample the helix in the z interval
     * 
     *  @return The number of pseudo layers crossed
     */
    static unsigned int GetNLayersCrossed(const pandora::Pandora &pandora, const pandora::Helix &helix, const float zStart,
        const float zEnd, const unsigned int nSamplingPoints = 100);

    /**
     *  @brief  Get the distance between hits in a cluster and a helix, typically the result of a fit to a track
     * 
     *  @param  pCluster address of the cluster
     *  @param  helix the helix
     *  @param  startLayer the first pseudo layer of the cluster to examine
     *  @param  endLayer the last pseudo layer of the cluster to examine
     *  @param  maxOccupiedLayers the maximum number of occupied cluster pseudo layers to examine
     *  @param  closestDistanceToHit to receive the closest distance between the helix and a hit in the specified range of the cluster
     *  @param  meanDistanceToHits to receive the mean distance between the helix and hits in the specified range of the cluster
     */
    static pandora::StatusCode GetClusterHelixDistance(const pandora::Cluster *const pCluster, const pandora::Helix &helix,
        const unsigned int startLayer, const unsigned int endLayer, const unsigned int maxOccupiedLayers, float &closestDistanceToHit,
        float &meanDistanceToHits);

    /**
     *  @brief  Get the number of contact layers for two clusters and also the ratio of the number of contact layers to overlap layers
     * 
     *  @param  pClusterI address of the first cluster
     *  @param  pClusterJ address of the second cluster
     *  @param  distanceThreshold number of calorimeter cell-widths used to determine whether layers are in contact
     *  @param  nContactLayers to receive the number of contact layers
     *  @param  contactFraction to receive the ratio of the number of contact layers to number of overlap layers
     */
    static pandora::StatusCode GetClusterContactDetails(const pandora::Cluster *const pClusterI, const pandora::Cluster *const pClusterJ,
        const float distanceThreshold, unsigned int &nContactLayers, float &contactFraction);

    /**
     *  @brief  Get the electromagnetic energy-weighted mean common layer separation for a pair of provided clusters
     *          Note: energy-weighting uses only energies from cluster j
     * 
     *  @param  pClusterI address of first cluster
     *  @param  pClusterJ address of second cluster
     * 
     *  @return the energy-weighted mean common layer separation
     */
    static float GetEMEnergyWeightedLayerSeparation(const pandora::Cluster *const pClusterI, const pandora::Cluster *const pClusterJ);

    /**
     *  @brief  Get the electromagnetic energy-weighted mean cluster position for a provided cluster
     * 
     *  @param  pCluster address of the cluster
     * 
     *  @return the electromagnetic energy-weighted mean cluster position
     */
    static pandora::CartesianVector GetEMEnergyWeightedPosition(const pandora::Cluster *const pCluster);
};

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

inline const pandora::Cluster *ClusterContact::GetDaughterCluster() const
{
    return m_pDaughterCluster;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline const pandora::Cluster *ClusterContact::GetParentCluster() const
{
    return m_pParentCluster;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline unsigned int ClusterContact::GetNContactLayers() const
{
    return m_nContactLayers;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline float ClusterContact::GetContactFraction() const
{
    return m_contactFraction;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline float ClusterContact::GetDistanceToClosestHit() const
{
    return m_distanceToClosestHit;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline float ClusterContact::GetCloseHitFraction1() const
{
    return m_closeHitFraction1;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline float ClusterContact::GetCloseHitFraction2() const
{
    return m_closeHitFraction2;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline float ClusterContact::GetConeFraction1() const
{
    return m_coneFraction1;
}

} 

#endif 
