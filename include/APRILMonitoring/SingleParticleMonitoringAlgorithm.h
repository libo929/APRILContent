/// \file SingleParticleMonitoringAlgorithm.h
/*
 *
 * SingleParticleMonitoringAlgorithm.h header template automatically generated by a class generator
 * Creation date : jeu. oct. 6 2016
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


#ifndef SINGLEPARTICLEMONITORINGALGORITHM_H
#define SINGLEPARTICLEMONITORINGALGORITHM_H

#include "Pandora/Algorithm.h"
#include "Pandora/PandoraInputTypes.h"

namespace april_content {

  /**
   * @brief SingleParticleMonitoringAlgorithm class
   */
  class SingleParticleMonitoringAlgorithm : public pandora::Algorithm
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
    ~SingleParticleMonitoringAlgorithm();
    pandora::StatusCode Run();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

  private:
    std::string                m_treeName;
    std::string                m_fileName;
    float                      m_gapDistanceFine;
    float                      m_gapDistanceCoarse;
    bool                       m_shouldUseIsolatedGapHits;
  };


  //------------------------------------------------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------------------------------------------------

  inline pandora::Algorithm *SingleParticleMonitoringAlgorithm::Factory::CreateAlgorithm() const
  {
      return new SingleParticleMonitoringAlgorithm();
  }

} 

#endif  //  SINGLEPARTICLEMONITORINGALGORITHM_H
