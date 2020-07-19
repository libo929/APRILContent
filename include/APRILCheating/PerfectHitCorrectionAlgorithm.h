/**
 *  @file   APRILContent/include/APRILCheating/PerfectHitCorrectionAlgorithm.h
 * 
 *  @brief  Header file for the perfect particle flow algorithm class.
 * 
 *  @author J. Marshall
 *  @comment Copied from https://github.com/PandoraPFA/LCContent
 *  $Log: $
 */
#ifndef APRILPERFECTHITCORRECTION_H
#define APRILPERFECTHITCORRECTION_H 1

#include "Api/PandoraContentApi.h"

#include "Pandora/Algorithm.h"

#include "APRILHelpers/HistogramHelper.h"

namespace april_content // changed namespace
{

/**
 *  @brief PerfectHitCorrectionAlgorithm class
 */
class PerfectHitCorrectionAlgorithm : public pandora::Algorithm
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
    PerfectHitCorrectionAlgorithm();

private:
    pandora::StatusCode Run();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    typedef PandoraContentApi::ParticleFlowObject::Parameters PfoParameters;

    /**
     *  @brief  Collection of calo hits
     * 
     *  @param  pPfoTarget address of the pfo target
     *  @param  pfoParameters the pfo parameters
     */
    void CaloHitCollection(const pandora::MCParticle *const pPfoTarget, PfoParameters &pfoParameters) const;

    /**
     *  @brief  Simple collection of calo hits, using only hits for which pfo target is the main mc particle
     * 
     *  @param  pPfoTarget address of the pfo target
     *  @param  pCaloHit address of the calo hit to consider
     *  @param  caloHitList to receive selected calo hits
     */
    void SimpleCaloHitCollection(const pandora::MCParticle *const pPfoTarget, const pandora::CaloHit *const pCaloHit, pandora::CaloHitList &caloHitList) const;

    /**
     *  @brief  Full collection of calo hits, using map of mc particles to hit weights; fragment calo hits where necessary
     * 
     *  @param  pPfoTarget address of the pfo target
     *  @param  pCaloHit address of the calo hit to consider
     *  @param  caloHitList to receive selected calo hits, which may include a list of fragments of the original input hit
     */
    void FullCaloHitCollection(const pandora::MCParticle *const pPfoTarget, const pandora::CaloHit *const pCaloHit, pandora::CaloHitList &caloHitList) const;

    /**
     *  @brief  Collection of tracks, using only tracks for which pfo target is the main mc particle
     * 
     *  @param  pPfoTarget address of the pfo target
     *  @param  pfoParameters the pfo parameters
     */
    void TrackCollection(const pandora::MCParticle *const pPfoTarget, PfoParameters &pfoParameters) const;

    /**
     *  @brief  Set the pfo parameters using tracks that have been collected together
     * 
     *  @param  pPfoTarget address of the pfo target
     *  @param  nTracksUsed to receive the number of tracks used for setting pfo properties
     *  @param  pfoParameters the pfo parameters
     */
    void SetPfoParametersFromTracks(const pandora::MCParticle *const pPfoTarget, int &nTracksUsed, PfoParameters &pfoParameters) const;

    /**
     *  @brief  Set the pfo parameters using clusters that have been collected together
     * 
     *  @param  pPfoTarget address of the pfo target
     *  @param  nTracksUsed the number of tracks used for setting pfo properties
     *  @param  pfoParameters the pfo parameters
     */
    void SetPfoParametersFromClusters(const pandora::MCParticle *const pPfoTarget, const int nTracksUsed, PfoParameters &pfoParameters) const;

    /**
     *  @brief  Get the energy-weighted centroid for a specified cluster, calculated over a particular pseudo layer range
     * 
     *  @param  pCluster address of the cluster to consider
     *  @param  pseudoLayer the inner pseudo layer of interest
     *  @param  pseudoLayer the outer pseudo layer of interest
     * 
     *  @return The energy-weighted centroid, returned by value
     */
    const pandora::CartesianVector GetEnergyWeightedCentroid(const pandora::Cluster *const pCluster, const unsigned int innerPseudoLayer,
        const unsigned int outerPseudoLayer) const;

    /**
     *  @brief  Print relevant pfo parameter debug information, if required
     * 
     *  @param  pPfoTarget address of the pfo target
     *  @param  nTracksUsed the number of tracks used for setting pfo properties
     *  @param  pfoParameters the pfo parameters
     */
    void PfoParameterDebugInformation(const pandora::MCParticle *const pPfoTarget, const int nTracksUsed, PfoParameters &pfoParameters) const;

    std::string     m_outputPfoListName;                    ///< The output pfo list name
    std::string     m_outputClusterListName;                ///< The output cluster list name
    bool            m_simpleCaloHitCollection;              ///< Whether to use simple calo hit collection mechanism, or full mechanism
    float           m_minWeightFraction;                    ///< The minimum mc particle calo hit weight for hit fragmentation
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *PerfectHitCorrectionAlgorithm::Factory::CreateAlgorithm() const
{
    return new PerfectHitCorrectionAlgorithm();
}

} // namespace april_content

#endif 
