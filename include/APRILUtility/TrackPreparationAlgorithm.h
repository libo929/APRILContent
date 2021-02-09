  /// \file TrackPreparationAlgorithm.h
/*
 *
 * TrackPreparationAlgorithm.h header template automatically generated by a class generator
 * Creation date : sam. mars 21 2015
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
 * @author John Marshall
 */


#ifndef APRIL_TRACK_PREPARATION_ALGORITHM_H
#define APRIL_TRACK_PREPARATION_ALGORITHM_H 1

#include "Pandora/Algorithm.h"

namespace april_content
{

/**
 *  @brief  TrackPreparationAlgorithm class
 */
class TrackPreparationAlgorithm : public pandora::Algorithm
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
    TrackPreparationAlgorithm();

private:
    pandora::StatusCode Run();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    /**
     *  @brief  Create list of parent tracks to be used in construction of charged pfos
     * 
     *  @param  inputTrackList the input track list
     *  @param  pfoTrackList to receive the list of parent tracks to be used in construction of charged pfos
     */
    pandora::StatusCode CreatePfoTrackList(const pandora::TrackList &inputTrackList, pandora::TrackList &pfoTrackList) const;

    /**
     *  @brief  Whether a track, or any of its daughters or siblings has an associated cluster
     * 
     *  @param  pTrack address of the track
     *  @param  readSiblingInfo whether to read sibling track information (set to false to avoid multiple counting)
     *
     *  @return boolean
     */
    bool HasAssociatedClusters(const pandora::Track *const pTrack, const bool readSiblingInfo = true) const;

    pandora::StringVector   m_candidateListNames;           ///< The list of track list names to use
    std::string             m_mergedCandidateListName;      ///< The name under which to save the full, merged, list of candidate tracks

    bool                    m_shouldMakeAssociations;       ///< Whether to re-make track-cluster associations for candidate tracks
    pandora::StringVector   m_associationAlgorithms;        ///< The ordered list of track-cluster associations algorithm to run

    bool                    m_shouldMakePfoTrackList;       ///< Whether to make pfo track list, containing parent tracks of charged pfos
    std::string             m_pfoTrackListName;             ///< The name of the pfo track list, containing parent tracks of charged pfos
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *TrackPreparationAlgorithm::Factory::CreateAlgorithm() const
{
    return new TrackPreparationAlgorithm();
}

}

#endif // #ifndef APRIL_TRACK_PREPARATION_ALGORITHM_H