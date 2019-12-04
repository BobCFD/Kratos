//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Suneth Warnakulasuriya (https://github.com/sunethwarna)
//

#if !defined(KRATOS_TIME_AVERAGING_PROCESS_H_INCLUDED)
#define KRATOS_TIME_AVERAGING_PROCESS_H_INCLUDED

// System includes
#include <string>
#include <vector>

// External includes

// Project includes
#include "containers/model.h"
#include "processes/process.h"

namespace Kratos
{
///@addtogroup RANSApplication
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

class KRATOS_API(RANS_APPLICATION) RansTimeAveragingProcess : public Process
{
public:
    ///@name Type Definitions
    ///@{

    using NodeType = ModelPart::NodeType;

    /// Pointer definition of RansTimeAveragingProcess
    KRATOS_CLASS_POINTER_DEFINITION(RansTimeAveragingProcess);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Constructor

    RansTimeAveragingProcess(Model& rModel, Parameters rParameters);

    /// Destructor.
    ~RansTimeAveragingProcess() override;

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    int Check() override;

    void ExecuteInitialize() override;

    void Execute() override;

    void ExecuteFinalizeSolutionStep() override;

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
    std::string Info() const override;

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override;

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override;

    ///@}
    ///@name Friends
    ///@{

    ///@}

protected:
    ///@name Protected static Member Variables
    ///@{

    ///@}
    ///@name Protected member Variables
    ///@{

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
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    Model& mrModel;
    Parameters mrParameters;

    std::string mModelPartName;
    std::string mIntegrationControlVariableName;

    std::vector<std::string> mVariableNamesList;

    int mEchoLevel;

    double mCurrentTime;

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{
    bool IsIntegrationStep() const;

    template <typename TDataType>
    void InitializeTimeAveragedQuantity(ModelPart::NodesContainerType& rNodes,
                                        const Variable<TDataType>& rVariable) const;

    template <typename TDataType>
    void CalculateTimeIntegratedQuantity(ModelPart::NodesContainerType& rNodes,
                                         const Variable<TDataType>& rVariable,
                                         const double DeltaTime) const;

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
    RansTimeAveragingProcess& operator=(RansTimeAveragingProcess const& rOther);

    /// Copy constructor.
    RansTimeAveragingProcess(RansTimeAveragingProcess const& rOther);

    ///@}

}; // Class RansTimeAveragingProcess

///@}

///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

/// output stream function
inline std::ostream& operator<<(std::ostream& rOStream, const RansTimeAveragingProcess& rThis);

///@}

///@} addtogroup block

} // namespace Kratos.

#endif // KRATOS_TIME_AVERAGING_PROCESS_H_INCLUDED defined
