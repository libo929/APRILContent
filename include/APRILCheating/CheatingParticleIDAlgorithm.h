/**
 *  @file   APRILContent/include/APRILCheating/CheatingParticleIDAlgorithm.h
 *
 *  @brief  Header file for the cheating particle id algorithm class.
 * 
 *  $Log: $
 */
#ifndef APRIL_CHEATING_PARTICLE_ID_ALGORITHM_H
#define APRIL_CHEATING_PARTICLE_ID_ALGORITHM_H 1

#include "Pandora/Algorithm.h"

namespace april_content
{

/**
 *  @brief  CheatingParticleIDAlgorithm class
 */
class CheatingParticleIDAlgorithm : public pandora::Algorithm
{
public:
    /**
     *  @brief  Factory class for instantiating algorithm
     */
    CheatingParticleIDAlgorithm();

    class Factory : public pandora::AlgorithmFactory
    {
    public:
        pandora::Algorithm *CreateAlgorithm() const;
    };

private:
    pandora::StatusCode Run();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    bool            m_useClusterOverTrackID;    ///< In case of PFO with tracks and clusters, take best mc particle from cluster
};

inline pandora::Algorithm *CheatingParticleIDAlgorithm::Factory::CreateAlgorithm() const
{
    return new CheatingParticleIDAlgorithm();
}

} // namespace april_content

#endif // #ifndef APRIL_CHEATING_PARTICLE_ID_ALGORITHM_H
