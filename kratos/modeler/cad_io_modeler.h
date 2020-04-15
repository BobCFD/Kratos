//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//


#if !defined(KRATOS_IGA_IO_MODELER_H_INCLUDED )
#define  KRATOS_IGA_IO_MODELER_H_INCLUDED


// System includes

// External includes

// Project includes
#include "modeler.h"
#include "input_output/cad_json_input.h"


namespace Kratos
{

///@name Kratos Classes
///@{

/// Short class definition.
/** Detail class definition.
*/
class KRATOS_API(KRATOS_CORE) CadIoModeler
    : public Modeler
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of Modeler
    KRATOS_CLASS_POINTER_DEFINITION(CadIoModeler);

    typedef std::size_t SizeType;
    typedef std::size_t IndexType;

    typedef Node<3> NodeType;
    typedef Geometry<NodeType> GeometryType;
    typedef typename GeometryType::GeometriesArrayType GeometriesArrayType;

    typedef typename Properties::Pointer PropertiesPointerType;

    typedef typename ModelPart::ElementsContainerType ElementsContainerType;
    typedef typename ModelPart::ConditionsContainerType ConditionsContainerType;

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    CadIoModeler(const Parameters ModelerParameters = Parameters())
        : Modeler(ModelerParameters)
        , mEchoLevel(
            ModelerParameters.Has("EchoLevel")
            ? ModelerParameters["EchoLevel"].GetInt()
            : 0)
    {
    }

    /// Destructor.
    virtual ~CadIoModeler() = default;

    /// Creates the Modeler Pointer
    Modeler::Pointer Create(const Parameters ModelParameters) const override
    {
        return Kratos::make_shared<CadIoModeler>(ModelParameters);
    }

    ///@}
    ///@name Stages
    ///@{

    void ImportGeometryModel(
        Model& rModel) const override;

    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override
    {
        return "CadIoModeler";
    }

    /// Print information about this object.
    void PrintInfo(std::ostream & rOStream) const override
    {
        rOStream << Info();
    }

    /// Print object's data.
    void PrintData(std::ostream & rOStream) const override
    {
    }

    ///@}

private:
    ///@name Iga functionalities
    ///@{

    SizeType mEchoLevel;

    ///@}

}; // Class CadIoModeler

///@}
///@name Input and output
///@{

/// input stream function
inline std::istream& operator >> (
    std::istream& rIStream,
    CadIoModeler& rThis);

/// output stream function
inline std::ostream& operator << (
    std::ostream& rOStream,
    const CadIoModeler& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}

///@}

}  // namespace Kratos.

#endif // KRATOS_IGA_IO_MODELER_H_INCLUDED  defined