/**
 * 
 *  @brief  Header file for the forced clustering algorithm class.
 * 
 *  $Log: $
 */
#ifndef APRIL_FORCED_CLUSTERING_ALGORITHM_H
#define APRIL_FORCED_CLUSTERING_ALGORITHM_H 1

#include "Pandora/Algorithm.h"

#include "Objects/CartesianVector.h"

namespace april_content
{

/**
 *  @brief  ForcedClusteringAlgorithm class
 */
class ForcedClusteringAlgorithm : public pandora::Algorithm
{
public:
    /**
     *  @brief Default constructor
     */

	class Factory : public pandora::AlgorithmFactory
	{
	public:
		pandora::Algorithm *CreateAlgorithm() const;
	};

    ForcedClusteringAlgorithm();

private:
    pandora::StatusCode Run();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    /**
     *  @brief  Remove any empty clusters at the end of the algorithm
     */
    pandora::StatusCode RemoveEmptyClusters() const;

    /**
     *  @brief  TrackDistanceInfo class
     */
    class TrackDistanceInfo
    {
    public:
        /**
         *  @brief  Constructor
         * 
         *  @param  pCaloHit address of calo hit
         *  @param  pCluster address of cluster seeded by track
         *  @param  trackEnergy energy of track, measured at dca
         *  @param  trackDistance distance from calo hit to track
         */
        TrackDistanceInfo(const pandora::CaloHit *const pCaloHit, const pandora::Cluster *const pCluster, const pandora::Track *const pTrack, float trackEnergy, float trackDistance);

        /**
         *  @brief  Get the address of the calo hit
         *
         *  @return The address of the calo hit
         */
        const pandora::CaloHit *GetCaloHit() const;

        /**
         *  @brief  Get the address of the cluster seeded by track
         *
         *  @return The address of the cluster seeded by track
         */
        const pandora::Cluster *GetCluster() const;

        const pandora::Track *GetTrack() const;

        /**
         *  @brief  Get the energy of the track, measured at dca
         *
         *  @return The energy of the track, measured at dca
         */
        float GetTrackEnergy() const;

        /**
         *  @brief  Get the distance from calo hit to track
         *
         *  @return The distance from calo hit to track
         */
        float GetTrackDistance() const;

    private:
        const pandora::CaloHit     *m_pCaloHit;                                 ///< Address of calo hit
        const pandora::Cluster     *m_pCluster;                                 ///< Address of cluster seeded by track
		const pandora::Track       *m_pTrack;
        float                       m_trackEnergy;                              ///< Energy of track, measured at dca
        float                       m_trackDistance;                            ///< Distance from calo hit to track
    };

    typedef std::vector<TrackDistanceInfo> TrackDistanceInfoVector;

    /**
     *  @brief  Sort TrackDistanceInfo objects by increasing distance from track
     * 
     *  @param  lhs the first calo hit distance pair
     *  @param  rhs the second calo hit distance pair
     */
    static bool SortByDistanceToTrack(const TrackDistanceInfo &lhs, const TrackDistanceInfo &rhs);

    bool                            m_shouldRunStandardClusteringAlgorithm;     ///< Whether to run standard clustering algorithm to deal with remnants
    std::string                     m_standardClusteringAlgorithmName;          ///< The name of standard clustering algorithm to run
        
    bool                            m_shouldClusterIsolatedHits;                ///< Whether to directly include isolated hits in newly formed clusters
    bool                            m_shouldAssociateIsolatedHits;              ///< Whether to associate isolated hits to newly formed clusters
    std::string                     m_isolatedHitAssociationAlgorithmName;      ///< The name of isolated hit association algorithm
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline bool ForcedClusteringAlgorithm::SortByDistanceToTrack(const TrackDistanceInfo &lhs, const TrackDistanceInfo &rhs)
{
    return (lhs.GetTrackDistance() < rhs.GetTrackDistance());
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

inline ForcedClusteringAlgorithm::TrackDistanceInfo::TrackDistanceInfo(const pandora::CaloHit *const pCaloHit, const pandora::Cluster *const pCluster, const pandora::Track *const pTrack,
        float trackEnergy, float trackDistance) :
    m_pCaloHit(pCaloHit),
    m_pCluster(pCluster),
    m_pTrack(pTrack),
    m_trackEnergy(trackEnergy),
    m_trackDistance(trackDistance)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline const pandora::CaloHit *ForcedClusteringAlgorithm::TrackDistanceInfo::GetCaloHit() const
{
    return m_pCaloHit;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline const pandora::Cluster *ForcedClusteringAlgorithm::TrackDistanceInfo::GetCluster() const
{
    return m_pCluster;
}

inline const pandora::Track *ForcedClusteringAlgorithm::TrackDistanceInfo::GetTrack() const
{
    return m_pTrack;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline float ForcedClusteringAlgorithm::TrackDistanceInfo::GetTrackEnergy() const
{
    return m_trackEnergy;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline float ForcedClusteringAlgorithm::TrackDistanceInfo::GetTrackDistance() const
{
    return m_trackDistance;
}

inline pandora::Algorithm *ForcedClusteringAlgorithm::Factory::CreateAlgorithm() const
{
    return new ForcedClusteringAlgorithm();
}

} // namespace april_content

#endif // #ifndef APRIL_FORCED_CLUSTERING_ALGORITHM_H
