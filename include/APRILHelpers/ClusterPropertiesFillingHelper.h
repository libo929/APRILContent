/* ClusterPropertiesFillingHelper.h
 *
 * Author: Bo Li (IPNL)
 * 
 */

#ifndef CLUSTERPROPERTIESFILLINGHELPER_H
#define CLUSTERPROPERTIESFILLINGHELPER_H

#include "Pandora/PandoraInternal.h"
#include "Pandora/StatusCodes.h"
#include "Objects/Cluster.h"
#include "TNtuple.h"

namespace pandora { class Algorithm; class CaloHit; class Track; }

namespace april_content
{
	class ClusterPropertiesFillingHelper
	{
		public:
			static pandora::StatusCode FillProperties(const pandora::Cluster* cluster);

		private:
			// constructor
			ClusterPropertiesFillingHelper();

			// destructor
			~ClusterPropertiesFillingHelper();
	
			// the tuple of cluster property
			static TNtuple* m_ntuple;

			/////
	        static float m_minHitLayer; 
	        static float m_clusterVol; 
	        static float m_energyRatio;
	        static float m_hitOutsideRatio;
	        static float m_axisLengthRatio;
	        static float m_shortAxisLengthRatio;
	};
}

#endif  // CLUSTERPROPERTIESFILLINGHELPER_H
