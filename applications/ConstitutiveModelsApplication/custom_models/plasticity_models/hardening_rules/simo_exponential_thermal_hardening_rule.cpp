//
//   Project Name:        KratosConstitutiveModelsApplication $
//   Created by:          $Author:                JMCarbonell $
//   Last modified by:    $Co-Author:                         $
//   Date:                $Date:                   April 2017 $
//   Revision:            $Revision:                      0.0 $
//
//

// System includes

// External includes

// Project includes
#include "custom_models/plasticity_models/hardening_rules/simo_exponential_thermal_hardening_rule.hpp"

namespace Kratos
{

  //*******************************CONSTRUCTOR******************************************
  //************************************************************************************

  SimoExponentialThermalHardeningRule::SimoExponentialThermalHardeningRule()
    :SimoExponentialHardeningRule()
  {

  }


  //*******************************ASSIGMENT OPERATOR***********************************
  //************************************************************************************

  SimoExponentialThermalHardeningRule& SimoExponentialThermalHardeningRule::operator=(SimoExponentialThermalHardeningRule const& rOther)
  {
    SimoExponentialHardeningRule::operator=(rOther);
    return *this;
  }

  //*******************************COPY CONSTRUCTOR*************************************
  //************************************************************************************

  SimoExponentialThermalHardeningRule::SimoExponentialThermalHardeningRule(SimoExponentialThermalHardeningRule const& rOther)
    :SimoExponentialHardeningRule(rOther)
  {

  }


  //********************************CLONE***********************************************
  //************************************************************************************

  HardeningRule::Pointer SimoExponentialThermalHardeningRule::Clone() const
  {
    return ( HardeningRule::Pointer(new SimoExponentialThermalHardeningRule(*this)) );
  }


  //********************************DESTRUCTOR******************************************
  //************************************************************************************

  SimoExponentialThermalHardeningRule::~SimoExponentialThermalHardeningRule()
  {
  }

  /// Operations.

  //***************************CALCULATE HARDENING DERIVATIVE TEMPERATURE***************
  //************************************************************************************

  double& SimoExponentialThermalHardeningRule::CalculateDeltaThermalHardening(const PlasticDataType& rVariables, double& rDeltaThermalHardening)
  {
    KRATOS_TRY

    const ModelDataType& rModelData = rVariables.GetModelData();
      
    //get values   
    const double& rEquivalentPlasticStrain = rVariables.GetInternalVariables()[0];

    //linear hardening properties
    const Properties& rMaterialProperties  = rModelData.GetMaterialProperties();
    double  YieldStress                    = rMaterialProperties[YIELD_STRESS];
    double  KinematicHardeningConstant     = rMaterialProperties[KINEMATIC_HARDENING_MODULUS];
	
    //exponential saturation properties
    double  K_reference         =  rMaterialProperties[REFERENCE_HARDENING_MODULUS];
    double  K_infinity          =  rMaterialProperties[INFINITY_HARDENING_MODULUS];
    const double& Delta         =  rMaterialProperties[HARDENING_EXPONENT];

    //parameters for the thermal solution
    const double& ReferenceConductivity = rMaterialProperties[REFERENCE_CONDUCTIVITY];
    const double& HardnessConductivity  = rMaterialProperties[HARDNESS_CONDUCTIVITY];


    double ThermalFactor        = this->CalculateThermalReferenceEffect(rVariables,ThermalFactor);
    YieldStress                *= ThermalFactor;
    K_reference                *= ThermalFactor;

    ThermalFactor               = this->CalculateThermalCurrentEffect(rVariables,ThermalFactor);
    K_infinity                 *= ThermalFactor;
    KinematicHardeningConstant *= ThermalFactor;
    
    
    //Linear Hardening rule: (mTheta = 1)
    rDeltaThermalHardening  = ( YieldStress * ReferenceConductivity +  SimoExponentialHardeningRule::mTheta * KinematicHardeningConstant * HardnessConductivity * rEquivalentPlasticStrain );
	
    //Exponential Saturation:
    rDeltaThermalHardening += ( K_infinity * HardnessConductivity -K_reference * ReferenceConductivity ) * (1.0 - exp( (-1.0) * Delta * rEquivalentPlasticStrain ) );

    
    return rDeltaThermalHardening;

	
    KRATOS_CATCH(" ")
  }



  //***************************CALCULATE TEMPERATURE EVOLUTION PROPERTIES***************
  //************************************************************************************

  double& SimoExponentialThermalHardeningRule::CalculateThermalReferenceEffect(const PlasticDataType& rVariables, double& rThermalFactor)
  {
    KRATOS_TRY
      
    const ModelDataType& rModelData = rVariables.GetModelData();
        
    //get values
    const Properties& rMaterialProperties  = rModelData.GetMaterialProperties();
    const double& rTemperature             = rModelData.GetTemperature();
    
    //parameters for the thermal solution
    const double& ReferenceConductivity = rMaterialProperties[REFERENCE_CONDUCTIVITY];
    const double& ReferenceTemperature  = rMaterialProperties[REFERENCE_TEMPERATURE];       
    
    //thermal effect in the initial parameters
    rThermalFactor = ( 1.0 - ReferenceConductivity * (rTemperature - ReferenceTemperature) );   
  
    return rThermalFactor;
	
    KRATOS_CATCH(" ")
  }

  //***************************CALCULATE TEMPERATURE EVOLUTION PROPERTIES***************
  //************************************************************************************

  double& SimoExponentialThermalHardeningRule::CalculateThermalCurrentEffect(const PlasticDataType& rVariables, double& rThermalFactor)
  {
    KRATOS_TRY

    const ModelDataType& rModelData = rVariables.GetModelData();

    //get values
    const Properties& rMaterialProperties  = rModelData.GetMaterialProperties();
    const double& rTemperature             = rModelData.GetTemperature();
    
    //parameters for the thermal solution
    const double& HardnessConductivity  = rMaterialProperties[HARDNESS_CONDUCTIVITY];
    const double& ReferenceTemperature  = rMaterialProperties[REFERENCE_TEMPERATURE];       
    
    //thermal effect in the final parameters	
    rThermalFactor = ( 1.0 - HardnessConductivity * (rTemperature - ReferenceTemperature) );   

    return rThermalFactor;
 	
    KRATOS_CATCH(" ")
  }


}  // namespace Kratos.
