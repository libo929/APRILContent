/* ClusterTrackEnergyFillingHelper.h
 *
 * Author: Bo Li (IPNL)
 * 
 */

#ifndef CLUSTERTRACKENERGYFILLINGHELPER_H
#define CLUSTERTRACKENERGYFILLINGHELPER_H

#include "Pandora/PandoraInternal.h"
#include "Pandora/StatusCodes.h"
#include "TNtuple.h"

namespace pandora { class Algorithm; class CaloHit; class Track; }

namespace april_content
{
	class ClusterTrackEnergyFillingHelper
	{
		public:
			static pandora::StatusCode FillEnergy(float cluster, float track);

		private:
			// constructor
			ClusterTrackEnergyFillingHelper();

			// destructor
			~ClusterTrackEnergyFillingHelper();
	
			// the tuple of cluster property
			static TNtuple* m_ntuple;

			/////
	        static float m_cluster; 
	        static float m_track; 
	};
}

#endif  // CLUSTERTRACKENERGYFILLINGHELPER_H
