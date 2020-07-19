/// \file SingleClusterIdAlgorithm.h
/*
 * @author Bo Li(IPNL)
 *
 */

#ifndef SINGLECLUSTERIDALGORITHM_H 
#define SINGLECLUSTERIDALGORITHM_H

#include "Pandora/Algorithm.h"
#include "Pandora/PandoraInternal.h"

namespace april_content
{
  /**
   * @brief SingleClusterIdAlgorithm class
   */
  class SingleClusterIdAlgorithm : public pandora::Algorithm
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
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    /**
     *  @brief  Select photons from the input cluster list
     *
     *  @param  pInputClusterList the input cluster list
     *  @param  photonClusters the photons list to receive
     */

	pandora::StatusCode FillCluster(const pandora::ClusterList* pClusterList);
    pandora::StatusCode FillPhotonCluster(const pandora::ClusterList* pPhotonList);

  private:
    std::string     m_clusterListName;              ///< The name under which to save the new photon cluster list
    std::string     m_photonListName;              

	bool m_fillCluster;
	bool m_fillPhoton;

	float m_minEnergyRatio;
  };

  //------------------------------------------------------------------------------------------------------------------------------------------

  inline pandora::Algorithm *SingleClusterIdAlgorithm::Factory::CreateAlgorithm() const
  {
      return new SingleClusterIdAlgorithm();
  }
} 

#endif  //  SINGLECLUSTERIDALGORITHM_H
