/* BDTBasedParticleIdAlgorithm.h
 *
 * Author: Bo Li (IPNL)
 * 
 */

#ifndef BDTBASEDCLUSTERID_H
#define BDTBASEDCLUSTERID_H


#include "Pandora/PandoraInternal.h"
#include "Pandora/StatusCodes.h"

#include "Objects/Cluster.h"

#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"

#include "TVector3.h"
#include "TNtuple.h"

using namespace TMVA;

namespace pandora { class Algorithm; class CaloHit; class Track; }

namespace april_content
{
	class BDTBasedClusterIdHelper
	{   
		public: 
			static pandora::StatusCode BDTEvaluate(const pandora::Cluster* cluster, float& bdtEvaluation); 
		
		private:
			// constructor
			BDTBasedClusterIdHelper();

			// destructor
			~BDTBasedClusterIdHelper();
	
			// the reader of BDT
			static Reader* m_reader;

			// cluster properties for BDT
	        static float m_minHitLayer; 
	        static float m_clusterVol; 
	        static float m_energyRatio;
	        static float m_hitOutsideRatio;
	        static float m_axisLengthRatio;
	        static float m_shortAxisLengthRatio;
			
			static bool  m_isMethodBooked;
	}; 
}

#endif  // BDTBASEDCLUSTERID_H
