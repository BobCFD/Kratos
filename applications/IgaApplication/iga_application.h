/*
//  KRATOS  _____________
//         /  _/ ____/   |
//         / // / __/ /| |
//       _/ // /_/ / ___ |
//      /___/\____/_/  |_| Application
//
//  Main authors:   Thomas Oberbichler
*/

#if !defined(KRATOS_IGA_APPLICATION_H_INCLUDED)
#define  KRATOS_IGA_APPLICATION_H_INCLUDED

// System includes
#include <string>
#include <iostream>

// External includes

// Project includes
#include "includes/define.h"
#include "includes/kratos_application.h"
#include "includes/variables.h"

#include "custom_elements/iga_truss_element.h"
#include "custom_elements/iga_beam_element.h"
#include "custom_elements/iga_beam_ad_element.h"
#include "custom_elements/iga_beam_weak_dirichlet_condition.h"
#include "custom_elements/iga_beam_load_condition.h"
#include "custom_elements/iga_beam_ad_weak_coupling.h"
#include "custom_elements/iga_shell_3P_element.h"
#include "custom_elements/iga_shell_5P_element.h"
#include "custom_elements/shell_kl_discrete_element.h"

namespace Kratos {

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name Enum's
///@{

///@}
///@name Functions
///@{

///@}
///@name Kratos Classes
///@{

/// Short class definition.
/** Detail class definition.
*/
class KratosIgaApplication
    : public KratosApplication {
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of KratosIgaApplication
    KRATOS_CLASS_POINTER_DEFINITION(KratosIgaApplication);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    KratosIgaApplication();

    /// Destructor.
    ~KratosIgaApplication() override {}

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    virtual void Register();

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
    virtual std::string Info() const {
        return "KratosIgaApplication";
    }

    /// Print information about this object.
    virtual void PrintInfo(std::ostream& rOStream) const {
        rOStream << Info();
        PrintData(rOStream);
    }

    /// Print object's data.
    virtual void PrintData(std::ostream& rOStream) const {
        KRATOS_WATCH("in my application");
        KRATOS_WATCH(KratosComponents<VariableData>::GetComponents().size());

        rOStream << "Variables:" << std::endl;
        KratosComponents<VariableData>().PrintData(rOStream);
        rOStream << std::endl;
        rOStream << "Elements:" << std::endl;
        KratosComponents<Element>().PrintData(rOStream);
        rOStream << std::endl;
        rOStream << "Conditions:" << std::endl;
        KratosComponents<Condition>().PrintData(rOStream);
    }

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

    const IgaTrussElement mIgaTrussElement;
    const IgaBeamElement mIgaBeamElement;
    const IgaBeamADElement mIgaBeamADElement;
    const IgaBeamWeakDirichletCondition mIgaBeamWeakDirichletCondition;
    const IgaBeamADWeakCoupling mIgaBeamADWeakCoupling;
    const IgaShell3PElement mIgaShell3PElement;
    const IgaShell5PElement mIgaShell5PElement;
    const ShellKLDiscreteElement mShellKLDiscreteElement;

    const IgaBeamLoadCondition mIgaBeamLoadCondition;
    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    ///@}
    ///@name Private Access
    ///@{

    ///@}
    ///@name Private Inquiry
    ///@{

    ///@}
    ///@name Unaccessible methods
    ///@{

    /// Assignment operator.
    KratosIgaApplication& operator=(KratosIgaApplication const& rOther);

    /// Copy constructor.
    KratosIgaApplication(KratosIgaApplication const& rOther);

    ///@}

}; // class KratosIgaApplication

///@}

///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

///@}

} // namespace Kratos

#endif // !defined(KRATOS_IGA_APPLICATION_H_INCLUDED)
