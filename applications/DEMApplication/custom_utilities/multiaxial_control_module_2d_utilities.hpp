//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Ignasi de Pouplana
//                   Guillermo Casas
//


#ifndef KRATOS_MULTIAXIAL_CONTROL_MODULE_2D_UTILITIES
#define KRATOS_MULTIAXIAL_CONTROL_MODULE_2D_UTILITIES

// /* External includes */

// System includes

// Project includes
#include "includes/variables.h"

/* System includes */
#include <limits>
#include <iostream>
#include <iomanip>
#include <map>

/* External includes */
#ifdef _OPENMP
#include <omp.h>
#endif

/* Project includes */
#include "geometries/geometry.h"
#include "includes/define.h"
#include "includes/model_part.h"

#include "includes/table.h"
#include "includes/kratos_parameters.h"

// Application includes
#include "custom_elements/spheric_continuum_particle.h"

#include "DEM_application_variables.h"


namespace Kratos
{
class MultiaxialControlModule2DUtilities
{
public:

KRATOS_CLASS_POINTER_DEFINITION(MultiaxialControlModule2DUtilities);

/// Defining a table with double argument and result type as table type.
typedef Table<double,double> TableType;

/// Default constructor.

MultiaxialControlModule2DUtilities(ModelPart& rDemModelPart,
                                ModelPart& rDemFemBoundaryModelPart,
                                Parameters& rParameters
                                ) :
                                mrDemModelPart(rDemModelPart),
                                mrDemFemBoundaryModelPart(rDemFemBoundaryModelPart)
{
    KRATOS_TRY

    Parameters default_parameters( R"(
        {
            "Parameters"    : {
                "control_module_time_step": 1.0e-5,
                "velocity_factor" : 1.0,
                "stress_increment_tolerance": 1.0e-3,
                "update_stiffness": true,
                "stiffness_alpha": 1.0,
                "start_time" : 0.0,
                "stress_averaging_time": 1.0e-5
            },
            "list_of_actuators" : []
        }  )" );

    // Now validate agains defaults -- this also ensures no type mismatch
    rParameters.ValidateAndAssignDefaults(default_parameters);

    // TODO: fill maps
    // std::map<std::string, std::vector<ModelPart*>> mFEMBoundariesSubModelParts; /// FEM SubModelParts associated to each boundary of every actuator
    // std::map<std::string, std::vector<ModelPart*>> mDEMBoundariesSubModelParts; /// DEM SubModelParts associated to each boundary of every actuator
    // std::map<std::string, unsigned int> mTargetStressTableIds; /// TargetStressTableIds associated to every actuator
    // std::map<std::string, double> mLimitVelocities;
    // std::map<std::string, double> mCompressionLengths;
    // std::map<std::string, std::vector<double>> mVectorsOfLastStresses;
    // Vector mVelocity;
    // Vector mReactionStressOld;
    // Matrix mStiffness;
    
    for (unsigned int i = 0; i < rParameters["list_of_actuators"].size(); i++) {
        for (unsigned int j = 0; j < rParameters["list_of_actuators"][i]["list_of_dem_boundaries"].size(); j++) {
            mDEMBoundariesSubModelParts[rParameters["list_of_actuators"][i]["Parameters"]["actuator_name"].GetString()] = 
                &( mrDemModelPart.GetSubModelPart(rParameters["list_of_actuators"][i]["list_of_dem_boundaries"][j]["model_part_name"].GetString()) );
        }
        for (unsigned int j = 0; j < rParameters["list_of_actuators"][i]["list_of_fem_boundaries"].size(); j++) {
            mFEMBoundariesSubModelParts[rParameters["list_of_actuators"][i]["Parameters"]["actuator_name"].GetString()] = 
                &( rDemFemBoundaryModelPart.GetSubModelPart(rParameters["list_of_actuators"][i]["list_of_fem_boundaries"][j]["model_part_name"].GetString()) );
        }
    }


    // Note: this utility is design to be used in the Z direction of a 2D case
    mTargetStressTableId = rParameters["target_stress_table_id"].GetInt();
    mVelocity = rParameters["initial_velocity"].GetDouble();
    mLimitVelocity = rParameters["limit_velocity"].GetDouble();
    mVelocityFactor = rParameters["velocity_factor"].GetDouble();
    mCompressionLength = rParameters["compression_length"].GetDouble();
    mStartTime = rParameters["start_time"].GetDouble();
    mStressIncrementTolerance = rParameters["stress_increment_tolerance"].GetDouble();
    mUpdateStiffness = rParameters["update_stiffness"].GetBool();
    mReactionStressOld = 0.0;
    mStiffness = rParameters["young_modulus"].GetDouble()/mCompressionLength; // mStiffness is actually a stiffness over an area
    mStressAveragingTime = rParameters["stress_averaging_time"].GetDouble();
    mVectorOfLastStresses.resize(0);

    mAlternateAxisLoading = rParameters["alternate_axis_loading"].GetBool();
    mZCounter = 3;

    mrDemModelPart.GetProcessInfo()[TARGET_STRESS_Z] = 0.0;

    KRATOS_CATCH("");
}

/// Destructor.

virtual ~MultiaxialControlModule2DUtilities(){}

//***************************************************************************************************************
//***************************************************************************************************************

// Before FEM and DEM solution
void ExecuteInitialize()
{
    KRATOS_TRY;

    mrDemModelPart.GetProcessInfo()[IMPOSED_Z_STRAIN_VALUE] = 0.0;

    // Fem elements have IMPOSED_Z_STRAIN_VALUE already initialized as 0.0

    KRATOS_CATCH("");
}

// Before FEM and DEM solution
void ExecuteInitializeSolutionStep()
{
    KRATOS_TRY;

    const double CurrentTime = mrDemModelPart.GetProcessInfo()[TIME];
    const double delta_time = mrDemModelPart.GetProcessInfo()[DELTA_TIME];
    const ProcessInfo& CurrentProcessInfo = mrDemModelPart.GetProcessInfo();
    int NElems = static_cast<int>(mrDemModelPart.Elements().size());
    ModelPart::ElementsContainerType::iterator elem_begin = mrDemModelPart.ElementsBegin();
    const int NNodes = static_cast<int>(mrDemModelPart.Nodes().size());
    ModelPart::NodesContainerType::iterator it_begin = mrDemModelPart.NodesBegin();
    TableType::Pointer pTargetStressTable = mrDemModelPart.pGetTable(mTargetStressTableId);

    double reaction_stress = CalculateReactionStress();
    reaction_stress = UpdateVectorOfHistoricalStressesAndComputeNewAverage(reaction_stress);

    // Check whether this is a loading step for the current axis
    IsTimeToApplyCM();

    if (mApplyCM == true) {

        // Update K if required
        if (mAlternateAxisLoading == false) {
            if(mUpdateStiffness == true) {
                mStiffness = EstimateStiffness(reaction_stress,delta_time);
            }
        }
        mReactionStressOld = reaction_stress;

        // Update velocity
        const double NextTargetStress = pTargetStressTable->GetValue(CurrentTime+delta_time);
        const double df_target = NextTargetStress - reaction_stress;
        double delta_velocity = df_target/(mStiffness * delta_time) - mVelocity;

        if(std::abs(df_target) < mStressIncrementTolerance) { delta_velocity = -mVelocity; }

        mVelocity += mVelocityFactor * delta_velocity;

        if(std::abs(mVelocity) > std::abs(mLimitVelocity)) {
            if(mVelocity >= 0.0) { mVelocity = std::abs(mLimitVelocity); }
            else { mVelocity = - std::abs(mLimitVelocity); }
        }

        // Update IMPOSED_Z_STRAIN_VALUE
        // DEM modelpart
        mrDemModelPart.GetProcessInfo()[IMPOSED_Z_STRAIN_VALUE] += mVelocity*delta_time/mCompressionLength;
        // FEM modelpart
        const double imposed_z_strain = mrDemModelPart.GetProcessInfo()[IMPOSED_Z_STRAIN_VALUE];
        #pragma omp parallel for
        for(int i = 0; i < NElems; i++)
        {
            ModelPart::ElementsContainerType::iterator itElem = elem_begin + i;
            Element::GeometryType& rGeom = itElem->GetGeometry();
            GeometryData::IntegrationMethod MyIntegrationMethod = itElem->GetIntegrationMethod();
            const Element::GeometryType::IntegrationPointsArrayType& IntegrationPoints = rGeom.IntegrationPoints(MyIntegrationMethod);
            unsigned int NumGPoints = IntegrationPoints.size();
            std::vector<double> imposed_z_strain_vector(NumGPoints);
            // Loop through GaussPoints
            for ( unsigned int GPoint = 0; GPoint < NumGPoints; GPoint++ )
            {
                imposed_z_strain_vector[GPoint] = imposed_z_strain;
            }
            itElem->SetValuesOnIntegrationPoints( IMPOSED_Z_STRAIN_VALUE, imposed_z_strain_vector, CurrentProcessInfo );
        }
        // Save calculated velocity and reaction for print (only at FEM nodes)
        #pragma omp parallel for
        for(int i = 0; i<NNodes; i++) {
            ModelPart::NodesContainerType::iterator it = it_begin + i;
            it->FastGetSolutionStepValue(TARGET_STRESS_Z) = pTargetStressTable->GetValue(CurrentTime);
            it->FastGetSolutionStepValue(REACTION_STRESS_Z) = reaction_stress;
            it->FastGetSolutionStepValue(LOADING_VELOCITY_Z) = mVelocity;
        }
    } else {
        // Save calculated velocity and reaction for print (only at FEM nodes)
        #pragma omp parallel for
        for(int i = 0; i<NNodes; i++) {
            ModelPart::NodesContainerType::iterator it = it_begin + i;
            it->FastGetSolutionStepValue(TARGET_STRESS_Z) = pTargetStressTable->GetValue(CurrentTime);
            it->FastGetSolutionStepValue(REACTION_STRESS_Z) = reaction_stress;
            it->FastGetSolutionStepValue(LOADING_VELOCITY_Z) = 0.0;
        }
    }

    mrDemModelPart.GetProcessInfo()[TARGET_STRESS_Z] = pTargetStressTable->GetValue(CurrentTime);

    KRATOS_CATCH("");
}

// After FEM and DEM solution
void ExecuteFinalizeSolutionStep()
{
    // Update K with latest ReactionStress after the axis has been loaded
    if (mApplyCM == true) {
        if (mAlternateAxisLoading == true) {
            const double delta_time = mrDemModelPart.GetProcessInfo()[DELTA_TIME];
            double ReactionStress = CalculateReactionStress();
            if(mUpdateStiffness == true) {
                mStiffness = EstimateStiffness(ReactionStress,delta_time);
            }
        }
    }
}

//***************************************************************************************************************
//***************************************************************************************************************

///@}
///@name Inquiry
///@{


///@}
///@name Input and output
///@{

/// Turn back information as a stemplate<class T, std::size_t dim> tring.

virtual std::string Info() const
{
    return "";
}

/// Print information about this object.

virtual void PrintInfo(std::ostream& rOStream) const
{
}

/// Print object's data.

virtual void PrintData(std::ostream& rOStream) const
{
}


///@}
///@name Friends
///@{

///@}

protected:
///@name Protected static Member r_variables
///@{

    ModelPart& mrDemModelPart;
    ModelPart& mrDemFemBoundaryModelPart;
    double mStartTime;
    double mStressAveragingTime;
    double mVelocityFactor;
    double mStressIncrementTolerance;
    bool mUpdateStiffness;
    double mCMTimeStep;
    double mStiffnessAlpha;
    std::map<std::string, std::vector<ModelPart*>> mFEMBoundariesSubModelParts; /// FEM SubModelParts associated to each boundary of every actuator
    std::map<std::string, std::vector<ModelPart*>> mDEMBoundariesSubModelParts; /// DEM SubModelParts associated to each boundary of every actuator
    std::map<std::string, unsigned int> mTargetStressTableIds; /// TargetStressTableIds associated to every actuator
    std::map<std::string, double> mLimitVelocities;
    std::map<std::string, double> mCompressionLengths;
    std::map<std::string, std::vector<double>> mVectorsOfLastStresses;
    Vector mVelocity;
    Vector mReactionStressOld;
    Matrix mStiffness;

///@}
///@name Protected member r_variables
///@{ template<class T, std::size_t dim>


///@}
///@name Protected Operators
///@{


///@}
///@name Protected Operations
///@{


///@}
///@name Protected  Access
///@{

///@}
///@name Protected Inquiry
///@{


///@}
///@name Protected LifeCycle
///@{


///@}

private:

///@name Static Member r_variables
///@{


///@}
///@name Member r_variables
///@{
///@}
///@name Private Operators
///@{

///@}
///@name Private Operations
///@{

double UpdateVectorOfHistoricalStressesAndComputeNewAverage(const double& last_reaction) {
    KRATOS_TRY;
    int length_of_vector = mVectorOfLastStresses.size();
    if (length_of_vector == 0) { //only the first time
        int number_of_steps_for_stress_averaging = (int) (mStressAveragingTime / mrDemModelPart.GetProcessInfo()[DELTA_TIME]);
        if(number_of_steps_for_stress_averaging < 1) number_of_steps_for_stress_averaging = 1;
        mVectorOfLastStresses.resize(number_of_steps_for_stress_averaging);
        KRATOS_INFO("DEM") << " 'number_of_steps_for_stress_averaging' is "<< number_of_steps_for_stress_averaging << std::endl;
    }

    length_of_vector = mVectorOfLastStresses.size();

    if(length_of_vector > 1) {
        for(int i=1; i<length_of_vector; i++) {
            mVectorOfLastStresses[i-1] = mVectorOfLastStresses[i];
        }
    }
    mVectorOfLastStresses[length_of_vector-1] = last_reaction;

    double average = 0.0;
    for(int i=0; i<length_of_vector; i++) {
        average += mVectorOfLastStresses[i];
    }
    average /= (double) length_of_vector;
    return average;

    KRATOS_CATCH("");
}

void IsTimeToApplyCM(){
    const double current_time = mrDemModelPart.GetProcessInfo()[TIME];
    mApplyCM = false;

    if(current_time >= mStartTime) {
        if (mAlternateAxisLoading == true) {
            const unsigned int step = mrDemModelPart.GetProcessInfo()[STEP];
            if(step == mZCounter){
                mApplyCM = true;
                mZCounter += 3;
            }
        } else {
            mApplyCM = true;
        }
    }
}

double CalculateReactionStress() {
    // DEM variables
    ModelPart::ElementsContainerType& rElements = mrDemModelPart.GetCommunicator().LocalMesh().Elements();
    // FEM variables
    const ProcessInfo& CurrentProcessInfo = mrDemModelPart.GetProcessInfo();
    int NElems = static_cast<int>(mrDemModelPart.Elements().size());
    ModelPart::ElementsContainerType::iterator elem_begin = mrDemModelPart.ElementsBegin();

    // Calculate face_area
    double face_area = 0.0;
    // DEM modelpart
    #pragma omp parallel for reduction(+:face_area)
    for (int i = 0; i < (int)rElements.size(); i++) {
        ModelPart::ElementsContainerType::ptr_iterator ptr_itElem = rElements.ptr_begin() + i;
        Element* p_element = ptr_itElem->get();
        SphericContinuumParticle* pDemElem = dynamic_cast<SphericContinuumParticle*>(p_element);
        const double radius = pDemElem->GetRadius();
        face_area += Globals::Pi*radius*radius;
    }
    // FEM modelpart
    #pragma omp parallel for reduction(+:face_area)
    for(int i = 0; i < NElems; i++) {
        ModelPart::ElementsContainerType::iterator itElem = elem_begin + i;
        face_area += itElem->GetGeometry().Area();
    }

    // Calculate ReactionStress
    double face_reaction = 0.0;
    // DEM modelpart
    #pragma omp parallel for reduction(+:face_reaction)
    for (int i = 0; i < (int)rElements.size(); i++) {
        ModelPart::ElementsContainerType::ptr_iterator ptr_itElem = rElements.ptr_begin() + i;
        Element* p_element = ptr_itElem->get();
        SphericContinuumParticle* pDemElem = dynamic_cast<SphericContinuumParticle*>(p_element);
        BoundedMatrix<double, 3, 3> stress_tensor = ZeroMatrix(3,3);
        noalias(stress_tensor) = (*(pDemElem->mSymmStressTensor));
        const double radius = pDemElem->GetRadius();
        face_reaction += stress_tensor(2,2) * Globals::Pi*radius*radius;
    }
    // FEM modelpart
    #pragma omp parallel for reduction(+:face_reaction)
    for(int i = 0; i < NElems; i++)
    {
        ModelPart::ElementsContainerType::iterator itElem = elem_begin + i;
        Element::GeometryType& rGeom = itElem->GetGeometry();
        GeometryData::IntegrationMethod MyIntegrationMethod = itElem->GetIntegrationMethod();
        const Element::GeometryType::IntegrationPointsArrayType& IntegrationPoints = rGeom.IntegrationPoints(MyIntegrationMethod);
        unsigned int NumGPoints = IntegrationPoints.size();
        std::vector<Vector> stress_vector(NumGPoints);
        itElem->CalculateOnIntegrationPoints( CAUCHY_STRESS_VECTOR, stress_vector, CurrentProcessInfo );
        const double area_over_gp = rGeom.Area()/NumGPoints;
        // Loop through GaussPoints
        for ( unsigned int GPoint = 0; GPoint < NumGPoints; GPoint++ )
        {
            face_reaction += stress_vector[GPoint][2] * area_over_gp;
        }
    }

    double reaction_stress;
    if (std::abs(face_area) > 1.0e-12) {
        reaction_stress = face_reaction / face_area;
    } else {
        reaction_stress = 0.0;
    }

    return reaction_stress;
}

double EstimateStiffness(const double& rReactionStress, const double& rDeltaTime) {
    double K_estimated = mStiffness;
    if(std::abs(mVelocity) > 1.0e-12 && std::abs(rReactionStress-mReactionStressOld) > mStressIncrementTolerance) {
        K_estimated = std::abs((rReactionStress-mReactionStressOld)/(mVelocity * rDeltaTime));
    }
    return K_estimated;
}

///@}
///@name Private  Access
///@{


///@}
///@name Private Inquiry
///@{


///@}
///@name Un accessible methods
///@{

/// Assignment operator.
MultiaxialControlModule2DUtilities & operator=(MultiaxialControlModule2DUtilities const& rOther);


///@}

}; // Class MultiaxialControlModule2DUtilities

}  // namespace Python.

#endif // KRATOS_MULTIAXIAL_CONTROL_MODULE_2D_UTILITIES