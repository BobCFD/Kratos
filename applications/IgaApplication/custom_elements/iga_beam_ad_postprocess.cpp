/*
//  KRATOS  _____________
//         /  _/ ____/   |
//         / // / __/ /| |
//       _/ // /_/ / ___ |
//      /___/\____/_/  |_| Application
//
//  Main authors:   Thomas Oberbichler
//                  Lukas Rauch
*/

// System includes
#include <iostream>
#include <iomanip>
#include "includes/define.h"

#include "includes/variables.h"
#include <chrono>
// External includes

// Project includes
#include "iga_beam_ad_postprocess.h"
#include "iga_application_variables.h"

namespace Kratos {

Element::Pointer IgaBeamADPostprocess::Create(
    IndexType NewId,
    NodesArrayType const& ThisNodes,
    PropertiesType::Pointer pProperties) const
{
    auto geometry = GetGeometry().Create(ThisNodes);

    return Kratos::make_shared<IgaBeamADPostprocess>(NewId, geometry,
        pProperties);
}

void IgaBeamADPostprocess::GetDofList(
    DofsVectorType& rElementalDofList,
    ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    rElementalDofList.resize(NumberOfDofs());

    for (std::size_t i = 0; i < NumberOfNodes(); i++) {
        SetElementDof(rElementalDofList, i, 0, DISPLACEMENT_X);
        SetElementDof(rElementalDofList, i, 1, DISPLACEMENT_Y);
        SetElementDof(rElementalDofList, i, 2, DISPLACEMENT_Z);
        SetElementDof(rElementalDofList, i, 3, DISPLACEMENT_ROTATION);
    }

    KRATOS_CATCH("")
}

void IgaBeamADPostprocess::EquationIdVector(
    EquationIdVectorType& rResult,
    ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    rResult.resize(NumberOfDofs());

    for (std::size_t i = 0; i < NumberOfNodes(); i++) {
        SetElementEquationId(rResult, i, 0, DISPLACEMENT_X);
        SetElementEquationId(rResult, i, 1, DISPLACEMENT_Y);
        SetElementEquationId(rResult, i, 2, DISPLACEMENT_Z);
        SetElementEquationId(rResult, i, 3, DISPLACEMENT_ROTATION);
    }

    KRATOS_CATCH("")
}

void IgaBeamADPostprocess::Initialize()
{
    
}

void IgaBeamADPostprocess::CalculateAll(
    MatrixType& rLeftHandSideMatrix,
    VectorType& rRightHandSideVector,
    ProcessInfo& rCurrentProcessInfo,
    const bool ComputeLeftHandSide,
    const bool ComputeRightHandSide)
{
    using namespace BeamUtilities;
    using std::sqrt;
    using std::pow;

    using Vector3d = BeamUtilities::Vector<3>;
    using Matrix3d = BeamUtilities::Matrix<3, 3>;

    KRATOS_TRY;
    
    // get shape functions

    BeamUtilities::Matrix<3, Dynamic> shape_functions(3, NumberOfNodes());
    shape_functions.row(0) = MapVector(GetValue(SHAPE_FUNCTION_VALUES));
    shape_functions.row(1) = MapVector(GetValue(SHAPE_FUNCTION_LOCAL_DER_1));
    shape_functions.row(2) = MapVector(GetValue(SHAPE_FUNCTION_LOCAL_DER_2));

    std::ofstream write;
    write.open("TimestepAD.txt", std::ofstream::app);
    auto start = std::chrono::steady_clock::now();

    // get properties

    const auto& properties = GetProperties();

    const double young_modulus = properties[YOUNG_MODULUS];
    const double shear_modulus = properties[SHEAR_MODULUS];
    const double area = properties[CROSS_AREA];
    const double moment_of_inertia_x = properties[MOMENT_OF_INERTIA_T];
    const double moment_of_inertia_y = properties[MOMENT_OF_INERTIA_Y];
    const double moment_of_inertia_z = properties[MOMENT_OF_INERTIA_Z];
    const double prestress = properties[PRESTRESS_CAUCHY];
    const double possion = properties[PRESTRESS_CAUCHY];

    // material

    const double ea  = young_modulus * area;
    const double gi1 = shear_modulus * moment_of_inertia_x;
    const double ei2 = young_modulus * moment_of_inertia_y;
    const double ei3 = young_modulus * moment_of_inertia_z;

    const Vector3d A1   = MapVector(GetValue(BASE_A1));
    const Vector3d A1_1 = MapVector(GetValue(BASE_A1_1));

    const double A11 = A1.dot(A1);
    const double A = sqrt(A11);

    const Vector3d T = A1 / A;
    const Vector3d T_1 = A1_1 / A - A1.dot(A1_1) * A1 / pow(A, 3);

    const Vector3d A2   = MapVector(GetValue(BASE_A2));
    const Vector3d A2_1 = MapVector(GetValue(BASE_A2_1));

    const Vector3d A3   = MapVector(GetValue(BASE_A3));
    const Vector3d A3_1 = MapVector(GetValue(BASE_A3_1));

    const double B2 = A2_1.dot(A1);
    const double B3 = A3_1.dot(A1);

    const double C12 = A3_1.dot(A2);
    const double C13 = A2_1.dot(A3);

    const double Tm = 1 / A11;
    const double Ts = 1 / A;

    // actual configuration

    const auto phi = ComputeActValue(DISPLACEMENT_ROTATION, 0, shape_functions, GetGeometry());
    const auto phi_1 = ComputeActValue(DISPLACEMENT_ROTATION, 1, shape_functions, GetGeometry());    

    const auto a1 = ComputeActBaseVector(1, shape_functions, GetGeometry());
    const auto a1_1 = ComputeActBaseVector(2, shape_functions, GetGeometry());

    const auto a11 = a1.dot(a1);
    const auto a = sqrt(a11);

    const auto t = a1 / a;
    const auto t_1 = a1_1 / a - a1.dot(a1_1) * a1 / pow(a, 3);

    const auto rod = ComputeRod<HyperDual>(t, phi);
    const auto rod_1 = ComputeRod_1<HyperDual>(t, t_1, phi, phi_1);

    const auto lam = ComputeLam<HyperDual>(T, t);
    const auto lam_1 = ComputeLam_1<HyperDual>(T, T_1, t, t_1);

    const auto rod_lam = rod * lam;
    const auto rod_1_lam = rod_1 * lam;
    const auto rod_lam_1 = rod * lam_1;

    const auto a2 = rod_lam * A2.transpose();
    const auto a2_1 = rod_lam * A2_1.transpose() + rod_lam_1 * A2.transpose() + rod_1_lam * A2.transpose() ;

    const auto a3 = rod_lam * A3.transpose();
    const auto a3_1 = rod_lam * A3_1.transpose() + rod_lam_1 * A3.transpose() + rod_1_lam * A3.transpose();

    const auto b2 = a2_1.dot(a1);
    const auto b3 = a3_1.dot(a1);

    const auto c12 = a3_1.dot(a2);
    const auto c13 = a2_1.dot(a3);

    // inner energy

    const auto eps11 = Tm * (a11 - A11) / 2;
    const auto kap2  = Tm * (b2  - B2 );
    const auto kap3  = Tm * (b3  - B3 );
    const auto kap12 = Ts * (c12 - C12);
    const auto kap13 = Ts * (c13 - C13);

    auto end = std::chrono::steady_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    write << time.count() <<  "\n";
    write.close();

    const auto dP = 0.5 * (pow(eps11, 2) * ea ); 

    MapMatrix(rLeftHandSideMatrix) = dP.h() *0;
    MapVector(rRightHandSideVector) = -dP.g() *0;


    // Postprozessing 
    // const auto gauss_point = GetValue(GAUSS_POINT);

    const auto d_t = a1.dot(T);     // Anteil T in a1 Richtung
    const auto d_n = a1.dot(A2);     // Anteil N in a1 Richtung
    const auto d_v = a1.dot(A3);     // Anteil V in a1 Richtung

    const auto alpha_2  = HyperJet::atan(d_n / d_t);                  // Winkel zwischen a1 und A1 um n
    const auto alpha_3  = HyperJet::atan(d_v / d_t);                  // Winkel zwischen a1 und A1 um v

    std::ofstream write_f;
    write_f.open("kratos_postprocess.txt", std::ofstream::app);
    const double moment_kappa_2 = kap2.f() * ei3; 
    const double moment_kappa_3 = kap3.f() * ei2;
    const double normal_force   = (eps11.f() * ea  + moment_kappa_2 * kap2.f() + moment_kappa_3 * kap3.f());
    const double moment_torsion = 0.5 * (kap12.f() - kap13.f()) * gi1; 


    // write output 
    write_f << std::setw(4)  << Id()
            // << std::setw(20) << gauss_point[0] 
            // << std::setw(20) << gauss_point[1] 
            // << std::setw(20) << gauss_point[2] 
            << std::setw(30) << normal_force 
            << std::setw(30) << moment_kappa_3 
            << std::setw(30) << moment_kappa_2  
            << std::setw(30) << moment_torsion  
            << std::setw(30) << alpha_2.f()  
            << std::setw(30) << alpha_3.f()  
            <<  "\n";
    write_f.close();


    std::ofstream bm_m3;
    bm_m3.precision(13);
    bm_m3.open("benchmark_m3.txt");
    bm_m3 << moment_kappa_3;
    bm_m3.close();

    std::ofstream bm_mt;
    bm_mt.precision(13);
    bm_mt.open("benchmark_mt.txt");
    bm_mt << moment_torsion;
    bm_mt.close();


    KRATOS_CATCH("")
}

void IgaBeamADPostprocess::PrintInfo(std::ostream& rOStream) const
{
    rOStream << "\"IgaBeamADPostprocess\" #" << Id();
}

} // namespace Kratos