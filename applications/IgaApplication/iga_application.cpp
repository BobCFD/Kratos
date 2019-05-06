/*
//  KRATOS  _____________
//         /  _/ ____/   |
//         / // / __/ /| |
//       _/ // /_/ / ___ |
//      /___/\____/_/  |_| Application
//
//  Main authors:   Thomas Oberbichler
*/

// System includes

// External includes

// Project includes
#include "iga_application.h"
#include "iga_application_variables.h"

namespace Kratos {

KratosIgaApplication::KratosIgaApplication()
    : KratosApplication("IgaApplication")
    , mIgaTrussElement(0, Element::GeometryType::Pointer(
        new Geometry<Node<3>>(Element::GeometryType::PointsArrayType(1))))
    , mIgaBeamElement(0, Element::GeometryType::Pointer(
        new Geometry<Node<3>>(Element::GeometryType::PointsArrayType(1))))
    , mIgaBeamADElement(0, Element::GeometryType::Pointer(
        new Geometry<Node<3>>(Element::GeometryType::PointsArrayType(1))))
    , mIgaBeamWeakBeddingCondition(0, Element::GeometryType::Pointer(
        new Geometry<Node<3>>(Element::GeometryType::PointsArrayType(1))))
    , mIgaBeamADWeakCoupling(0, Element::GeometryType::Pointer(
        new Geometry<Node<3>>(Element::GeometryType::PointsArrayType(1))))
    , mIgaBeamADPostprocess(0, Element::GeometryType::Pointer(
        new Geometry<Node<3>>(Element::GeometryType::PointsArrayType(1))))
    , mIgaShell3PElement(0, Element::GeometryType::Pointer(
        new Geometry<Node<3>>(Element::GeometryType::PointsArrayType(1))))
    , mIgaShell5PElement(0, Element::GeometryType::Pointer(
        new Geometry<Node<3>>(Element::GeometryType::PointsArrayType(1))))
    , mShellKLDiscreteElement(0, Element::GeometryType::Pointer(
        new Geometry<Node<3>>(Element::GeometryType::PointsArrayType(1))))

    , mIgaBeamLoadCondition(0, Condition::GeometryType::Pointer(
        new Geometry<Node<3>>(Condition::GeometryType::PointsArrayType(1))))    
{
}

void KratosIgaApplication::Register() {
    KratosApplication::Register();
    std::cout << "Initializing KratosIgaApplication... " << std::endl;

    KRATOS_REGISTER_ELEMENT("IgaTrussElement", mIgaTrussElement)
    KRATOS_REGISTER_ELEMENT("IgaBeamElement", mIgaBeamElement)
    KRATOS_REGISTER_ELEMENT("IgaBeamADElement", mIgaBeamADElement)
    KRATOS_REGISTER_ELEMENT("IgaBeamWeakBeddingCondition", mIgaBeamWeakBeddingCondition)
    KRATOS_REGISTER_ELEMENT("IgaBeamADWeakCoupling", mIgaBeamADWeakCoupling)
    KRATOS_REGISTER_ELEMENT("IgaBeamADPostprocess", mIgaBeamADPostprocess)
    KRATOS_REGISTER_ELEMENT("IgaShell3PElement", mIgaShell3PElement)
    KRATOS_REGISTER_ELEMENT("IgaShell5PElement", mIgaShell5PElement)
    KRATOS_REGISTER_ELEMENT("ShellKLDiscreteElement", mShellKLDiscreteElement)
    
    KRATOS_REGISTER_CONDITION("IgaBeamLoadCondition", mIgaBeamLoadCondition)
}

}  // namespace Kratos
