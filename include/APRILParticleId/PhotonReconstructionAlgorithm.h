/// \file PhotonReconstructionAlgorithm.h
/*
 *
 * PhotonReconstructionAlgorithm.h header template automatically generated by a class generator
 * Creation date : lun. oct. 24 2016
 *
 * This file is part of APRILContent libraries.
 * 
 * APRILContent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 * 
 * APRILContent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with APRILContent.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * @author Remi Ete
 * @copyright CNRS , IPNL
 */


#ifndef PHOTONRECONSTRUCTIONALGORITHM_H
#define PHOTONRECONSTRUCTIONALGORITHM_H

#include "Pandora/Algorithm.h"
#include "Pandora/PandoraInternal.h"

#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"

namespace april_content
{
  /**
   * @brief PhotonReconstructionAlgorithm class
   */
  class PhotonReconstructionAlgorithm : public pandora::Algorithm
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
    pandora::StatusCode SelectPhotonsAndRemoveOthers(const pandora::ClusterList *const pInputClusterList, 
			pandora::ClusterList &photonClusters,
			pandora::ClusterList &nonPhotonClusters);

  private:
    std::string     m_clusteringAlgorithmName;      ///< The name of the clustering algorithm to run
    std::string     m_inputCaloHitListName;         ///< The name of the input calo hit list, containing the hits to be clustered
    bool            m_restoreOriginalCaloHitList;   ///< Whether to restore the original calo hit list as the "current" list upon completion
    std::string     m_photonClusterListName;        ///< The name under which to save the new photon cluster list
    std::string     m_nonPhotonClusterListName;     ///< The name under which to save the new non-photon cluster list
    bool            m_replaceCurrentClusterList;    ///< Whether to subsequently use the new photon cluster list as the "current" list

	static TMVA::Reader*           m_reader;
	bool            m_useMVA;

  };

  //------------------------------------------------------------------------------------------------------------------------------------------

  inline pandora::Algorithm *PhotonReconstructionAlgorithm::Factory::CreateAlgorithm() const
  {
      return new PhotonReconstructionAlgorithm();
  }
} 

#endif  //  PHOTONRECONSTRUCTIONALGORITHM_H
