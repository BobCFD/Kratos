// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:             BSD License
//                                       license: StructuralMechanicsApplication/license.txt
//
//  Main authors:    Manuel Messmer
//
//

// System includes

// External includes
#include <Eigen/Core>
#include <Eigen/Dense>

// Project includes
#include "custom_processes/perturb_geometry_base_process.h"
#include "utilities/builtin_timer.h"

#include "utilities/openmp_utils.h"



namespace Kratos
{

void PerturbGeometryBaseProcess::AssembleEigenvectors( ModelPart& rThisModelPart, const std::vector<double>& variables )
{
    KRATOS_TRY;
    BuiltinTimer assemble_eigenvectors_timer;
    DenseMatrixType& rPerturbationMatrix = *mpPerturbationMatrix;
    const int num_of_random_variables = variables.size();
    const int num_of_eigenvectors = rPerturbationMatrix.size2();
    const int num_of_nodes = rThisModelPart.NumberOfNodes();

    KRATOS_WARNING_IF("PerturbGeometryBaseProcess",
                    num_of_random_variables != num_of_random_variables)
        << "Number of random variables does not match number of eigenvectors: "
        << "Number of random variables: " << num_of_random_variables << ", "
        << "Number of eigenvectors: " << num_of_eigenvectors
        << std::endl;


    std::vector<double> random_field(num_of_nodes,0.0);

    #pragma omp parallel
    {
        #pragma omp for
        for( int i = 0; i < num_of_nodes; i++ ){
            for( int j = 0; j < num_of_random_variables; j++){
                random_field[i] += variables[j]* rPerturbationMatrix(i,j);
            }
        }
    }

    // Shift random field to zero mean
    double shifted_mean = 1.0 / num_of_nodes * std::accumulate(random_field.begin(), random_field.end(), 0.0);
    std::for_each( random_field.begin(), random_field.end(),
        [shifted_mean](double& element) {element -= shifted_mean; } );

    // Find maximum deviation
    double max_disp = *std::max_element(random_field.begin(), random_field.end());
    double min_disp = *std::min_element(random_field.begin(), random_field.end());
    double max_abs_disp = std::max( std::abs(max_disp), std::abs(min_disp) );

    // Scale random field to maximum deviation
    double multiplier = mMaximalDisplacement/max_abs_disp;
    std::for_each( random_field.begin(), random_field.end(),
        [multiplier](double& element) {element *= multiplier; } );

    // Apply random field to geometry
    #pragma omp parallel
    {
        array_1d<double, 3> normal;
        const auto it_node_original_begin = mrInitialModelPart.NodesBegin();
        const auto it_node_perturb_begin = rThisModelPart.NodesBegin();

        #pragma omp for private(normal)
        for( int i = 0; i < num_of_nodes; i++){
            auto it_node_original = it_node_original_begin + i;
            auto it_node_perturb = it_node_perturb_begin + i;

            // Get normal direction of original model
            normal =  it_node_original->FastGetSolutionStepValue(NORMAL);
            //it_node_perturb->GetInitialPosition().Coordinates() += normal*random_field[i];

            //For visualization
            it_node_perturb->FastGetSolutionStepValue(DISPLACEMENT_X) += normal(0)*random_field[i];
            it_node_perturb->FastGetSolutionStepValue(DISPLACEMENT_Y) += normal(1)*random_field[i];
            it_node_perturb->FastGetSolutionStepValue(DISPLACEMENT_Z) += normal(2)*random_field[i];
        }
    }
    KRATOS_INFO("PerturbGeometrySubgridProcess: Apply Random Field to Geometry Time")
        << assemble_eigenvectors_timer.ElapsedSeconds() << std::endl;

    KRATOS_CATCH("")
}


double PerturbGeometryBaseProcess::CorrelationFunction( ModelPart::NodeIterator itNode1, ModelPart::NodeIterator itNode2, double CorrelationLength)
{
    array_1d<double, 3> coorrdinate;
    coorrdinate = itNode1->GetInitialPosition().Coordinates() - itNode2->GetInitialPosition().Coordinates();

    double norm = sqrt( coorrdinate(0)*coorrdinate(0) + coorrdinate(1)*coorrdinate(1) + coorrdinate(2)*coorrdinate(2) );

    return( exp( - norm*norm / (CorrelationLength*CorrelationLength) ) );
}

} // namespace Kratos