  /// \file NearbyTrackPhotonRemovalAlg.h
/*
 *
 * NearbyTrackPhotonRemovalAlg.h header template automatically generated by a class generator
 * Creation date : lun. juin 27 2016
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


#ifndef NEARBYTRACKPHOTONREMOVALALG_H
#define NEARBYTRACKPHOTONREMOVALALG_H

#include "Pandora/Algorithm.h"
#include "Pandora/PandoraInternal.h"

namespace april_content
{

/** 
 * @brief NearbyTrackPhotonRemovalAlg class
 */ 
class NearbyTrackPhotonRemovalAlg : public pandora::Algorithm
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
	 *  @brief  Get the contents for the algorithm
	 *
	 *  @param  clusterVector the cluster vector to receive
	 *  @param  trackVector the track vector to receive
	 */
	pandora::StatusCode GetContents(pandora::ClusterVector &clusterVector, pandora::TrackVector &trackVector) const;

	/**
	 *  @brief  Remove photon hits nearby tracks and get all photon fragments with too less hits
	 *
	 *  @param  clusterVector the input photon cluster vector
	 *  @param  trackVector the input track vector
	 *  @param  photonFragmentVector the list of photon fragments to receive
	 */
	pandora::StatusCode RemoveNearbyPhotonHits(const pandora::ClusterVector &clusterVector, const pandora::TrackVector &trackVector,
			pandora::ClusterVector &photonFragmentVector) const;

	/**
	 *  @brief  Delete the provided list of photon fragments
	 *
	 *  @param  photonFragmentVector the list of photon fragments to remove
	 */
	pandora::StatusCode RemovePhotonFragments(const pandora::ClusterVector &photonFragmentVector) const;

private:
	std::string                 m_photonClusterListName;
	float                       m_maxTrackHitDistance;
	float                       m_maxOmegaTrack;
	float                       m_minRemainingPhotonEnergy;
	unsigned int                m_minRemainingPhotonNHits;
};

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *NearbyTrackPhotonRemovalAlg::Factory::CreateAlgorithm() const
{
    return new NearbyTrackPhotonRemovalAlg();
}

} 

#endif  //  NEARBYTRACKPHOTONREMOVALALG_H
