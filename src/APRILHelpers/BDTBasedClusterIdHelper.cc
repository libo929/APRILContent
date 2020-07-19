#include "APRILHelpers/BDTBasedClusterIdHelper.h"
#include "APRILHelpers/ClusterHelper.h"
#include "APRILHelpers/ClusterPropertiesHelper.h"
#include "APRILObjects/CaloHit.h"

#include "TMatrixDSymEigen.h"

Reader* april_content::BDTBasedClusterIdHelper::m_reader = NULL;

float april_content::BDTBasedClusterIdHelper::m_minHitLayer          = 0.;
float april_content::BDTBasedClusterIdHelper::m_clusterVol           = 0.;
float april_content::BDTBasedClusterIdHelper::m_energyRatio          = 0.;
float april_content::BDTBasedClusterIdHelper::m_hitOutsideRatio      = 0.;
float april_content::BDTBasedClusterIdHelper::m_axisLengthRatio      = 0.;
float april_content::BDTBasedClusterIdHelper::m_shortAxisLengthRatio = 0.;

bool  april_content::BDTBasedClusterIdHelper::m_isMethodBooked       = false;


april_content::BDTBasedClusterIdHelper::~BDTBasedClusterIdHelper() 
{
	//std::cout << "the destructor of BDTBasedClusterIdHelper ..." << std::endl;
	if(m_reader != NULL ) delete m_reader;
}

april_content::BDTBasedClusterIdHelper::BDTBasedClusterIdHelper() 
{   
   // --- Create the Reader object
   m_reader = new TMVA::Reader( "!Color:!Silent" ); 

   // Create a set of variables and declare them to the m_reader
   // - the variable names MUST corresponds in name and type to those given in the weight file(s) used

   m_reader->AddVariable( "MinHitLayer"         , &m_minHitLayer          );
   m_reader->AddVariable( "ClusterVol"          , &m_clusterVol           );
   m_reader->AddVariable( "EnergyRatio"         , &m_energyRatio          );
   m_reader->AddVariable( "HitOutsideRatio"     , &m_hitOutsideRatio      );
   m_reader->AddVariable( "AxisLengthRatio"     , &m_axisLengthRatio      );
   m_reader->AddVariable( "ShortAxisLengthRatio", &m_shortAxisLengthRatio );

   // --- Book the MVA methods
   TString methodName = TString("BDTD method");
   TString weightfile = TString("weights/TMVAClassification_BDTD.weights.xml");

   //IMethod* mva = NULL;

   try 
   {
	   //mva = m_reader->BookMVA( methodName, weightfile ); 
	   m_reader->BookMVA( methodName, weightfile ); 
	   m_isMethodBooked = true;
   }
   catch (...)
   {
	   m_isMethodBooked = false;
   }
} 

pandora::StatusCode april_content::BDTBasedClusterIdHelper::BDTEvaluate(const pandora::Cluster* cluster, float& bdtEvaluation)
{ 
	if(m_isMethodBooked==false) return pandora::STATUS_CODE_FAILURE;

#if 1
	static BDTBasedClusterIdHelper clusterId;
#endif

	ClusterPropertiesHelper::GetClusterProperties(cluster, m_minHitLayer, m_clusterVol, m_energyRatio, 
		                 m_hitOutsideRatio, m_axisLengthRatio, m_shortAxisLengthRatio);

	//cluster->GetNCaloHits();

	//bdtEvaluation = 3.14;

#if 0
	std::cout << "minDepth: " << minDepth 
		 << ", cluVol: " << cluVol              
	     << ", enRatio: " << enRatio           
	     << ", axisRatio: " << axisRatio     
	     << ", outsideRatio: " << outsideRatio << std::endl;
#endif

	bdtEvaluation = m_reader->EvaluateMVA( "BDTD method" );

	//return pandora::STATUS_CODE_FAILURE;
	return pandora::STATUS_CODE_SUCCESS;
}
