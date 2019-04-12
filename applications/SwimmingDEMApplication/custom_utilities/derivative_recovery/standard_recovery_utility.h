//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Guillermo Casas (gcasas@cimne.upc.edu)
//
//

#ifndef KRATOS_STANDARD_RECOVERY_UTILITY_H
#define KRATOS_STANDARD_RECOVERY_UTILITY_H

// System includes
#include <string>
#include <iostream>

// External includes

// Project includes
#include "includes/define.h"
#include "derivative_recovery_utility.h"
#include "includes/kratos_parameters.h"

// Application includes


namespace Kratos
{
///@addtogroup SwimmingDEMApplication
///@{

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{

class KRATOS_API(SWIMMING_DEM_APPLICATION) StandardRecoveryUtility : public DerivativeRecoveryUtility
{
public:
    ///@name Type Definitions
    ///@{
    using ScalarVariableType = DerivativeRecoveryUtility::ScalarVariableType;
    using ComponentVariableType = DerivativeRecoveryUtility::ComponentVariableType;
    using VectorVariableType = DerivativeRecoveryUtility::VectorVariableType;

    /// Pointer definition of StandardRecoveryUtility
    KRATOS_CLASS_POINTER_DEFINITION(StandardRecoveryUtility);

    ///@}
    ///@name Life Cycle
    ///@{
    /// Constructor with Kratos parameters.
    StandardRecoveryUtility(
        ModelPart& rModelPart,
        Parameters rParameters,
        RecoveryVariablesContainer& rVariablesContainer);

    /// Constructor with Kratos model
    StandardRecoveryUtility(
        Model& rModel,
        Parameters rParameters,
        RecoveryVariablesContainer& rVariablesContainer);

    /// Destructor.
    ~StandardRecoveryUtility() override {}

    ///@}
    ///@name Operators
    ///@{

    void Initialize() override;

    ///@}
    ///@name Operations
    ///@{

    ///@}
    ///@name Access
    ///@{

    ///@}
    ///@name Inquiry
    ///@{

    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override
    {
        std::stringstream buffer;
        buffer << "StandardRecoveryUtility" ;
        return buffer.str();
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override {rOStream << "StandardRecoveryUtility";}


    ///@}
    ///@name Friends
    ///@{

    ///@}

protected:

void AddPartialTimeDerivative(const ScalarVariableType& rScalarVariable, const ScalarVariableType& rTimeDerivativeVariable) override;

void AddPartialTimeDerivative(const ComponentVariableType& rScalarComponent, const ScalarVariableType& rTimeDerivativeVariable) override;

void AddPartialTimeDerivative(const VectorVariableType& rVectorVariable, const VectorVariableType& rTimeDerivativeVariable) override;

void CalculateGradient(const ScalarVariableType& rScalarVariable, const VectorVariableType& rGradientVariable) override;

void CalculateGradient(const ComponentVariableType& rScalarComponent, const VectorVariableType& rGradientVariable) override;

// void CalculateGradient(const VectorVariableType& rVectorVariable,
//                        const VectorVariableType& rComponent0GradientVariable,
//                        const VectorVariableType& rComponent1GradientVariable,
//                        const VectorVariableType& rComponent2GradientVariable) override;

void CalculateDivergence(const VectorVariableType& rVectorVariable, const ScalarVariableType& rDivergenceVariable) override;

void CalculateLaplacian(const ScalarVariableType& rScalarVariable, const ScalarVariableType& rLaplacianVariable) override;

void CalculateLaplacian(const ComponentVariableType& rScalarComponent, const ScalarVariableType& rLaplacianVariable) override;

void CalculateLaplacian(const VectorVariableType& rVectorComponent, const VectorVariableType& rLaplacianVariable) override;

void CalculateMaterialDerivative(const ScalarVariableType& rScalarVariable, const ScalarVariableType& rMaterialDerivativeVariable) override;

void CalculateMaterialDerivative(const ComponentVariableType& rScalarComponent, const ScalarVariableType& rMaterialDerivativeVariable) override;

void CalculateMaterialDerivative(const VectorVariableType& rScalarComponent, const VectorVariableType& rMaterialDerivativeVariable) override;

void CalculateRotational(const VectorVariableType rVectorVariable, const VectorVariableType& rRotationalVariable) override;

void CheckDefaultsAndSettings(Parameters rParameters) override;

private:
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{
    template<class TScalarVariable>
    void AddScalarPartialTimeDerivative(const TScalarVariable& rScalarVariable, const ScalarVariableType& rGradientVariable);

    template<class TScalarVariable>
    void CalculateScalarGradient(const TScalarVariable& rScalarVariable, const VectorVariableType& rGradientVariable);

    template<class TScalarVariable>
    void CalculateScalarLaplacian(const TScalarVariable& rScalarVariable, const ScalarVariableType& rLaplacianVariable);

    template<class TScalarVariable>
    void CalculateScalarMaterialDerivative(const TScalarVariable& rScalarVariable, const ScalarVariableType& rMaterialDerivativeVariable);
    ///@}
    ///@name Private  Access
    ///@{


    ///@}
    ///@name Private Inquiry
    ///@{


    ///@}
    ///@name Un accessible methods
    ///@{

    /// Default constructor.
    StandardRecoveryUtility() = delete;

    /// Assignment operator.
    StandardRecoveryUtility& operator=(StandardRecoveryUtility const& rOther) = delete;

    /// Copy constructor.
    StandardRecoveryUtility(StandardRecoveryUtility const& rOther) = delete;

    ///@}

}; // Class StandardRecoveryUtility

///@}
///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

///@}

///@} addtogroup block

};  // namespace Kratos.

#endif // KRATOS_STANDARD_RECOVERY_UTILITY_H
