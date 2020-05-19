# Import Kratos core and apps
import KratosMultiphysics as KM

# Additional imports
from KratosMultiphysics.ShapeOptimizationApplication import optimizer_factory
from KratosMultiphysics.ShapeOptimizationApplication.analyzer_base import AnalyzerBaseClass
from KratosMultiphysics.KratosUnittest import TestCase
import KratosMultiphysics.kratos_utilities as kratos_utilities
import csv, os

# Read parameters
with open("parameters.json",'r') as parameter_file:
    parameters = KM.Parameters(parameter_file.read())

model = KM.Model()

# =======================================================================================================
# Define external analyzer
# =======================================================================================================

# The external analyzer provides a response to constrain the distance of a specific node to a given target
class CustomAnalyzer(AnalyzerBaseClass):
    # --------------------------------------------------------------------------------------------------
    def AnalyzeDesignAndReportToCommunicator(self, current_design, optimization_iteration, communicator):

        # Constraint 1
        constraint_node_id = 893
        if communicator.isRequestingValueOf("y_value_893"):
            value = current_design.Nodes[constraint_node_id].Y
            communicator.reportValue("y_value_893", value)

        if communicator.isRequestingGradientOf("y_value_893"):
            gradient = {}
            for node in current_design.Nodes:
                if node.Id == constraint_node_id:
                    gradient[node.Id] = [0.0,1.0,0.0]
                else:
                    gradient[node.Id] = [0.0,0.0,0.0]
            communicator.reportGradient("y_value_893", gradient)

        # Constraint 2
        constrained_node_id =1861
        target_x = 11.0
        target_y = 0.0
        target_z = 10.0
        if communicator.isRequestingValueOf("distance_1861"):
            constrained_node = current_design.GetNodes()[constrained_node_id]

            distance = [0,0,0]
            distance[0] = constrained_node.X0 - target_x
            distance[1] = constrained_node.Y0 - target_y
            distance[2] = constrained_node.Z0 - target_z
            value = distance[0]**2 + distance[1]**2 + distance[2]**2

            communicator.reportValue("distance_1861", value)

        if communicator.isRequestingGradientOf("distance_1861"):
            gradient = {}
            for node in current_design.Nodes:
                if node.Id == constrained_node_id:
                    grad = [0,0,0]
                    grad[0] = 2*(constrained_node.X0 - target_x)
                    grad[1] = 2*(constrained_node.Y0 - target_y)
                    grad[2] = 2*(constrained_node.Z0 - target_z)
                    gradient[node.Id] = [grad[0],grad[1],grad[2]]
                else:
                    gradient[node.Id] = [0.0,0.0,0.0]

            communicator.reportGradient("distance_1861", gradient)

# =======================================================================================================
# Perform optimization
# =======================================================================================================

# Create optimizer and perform optimization
optimizer = optimizer_factory.CreateOptimizer(parameters["optimization_settings"], model, CustomAnalyzer())
optimizer.Optimize()

# =======================================================================================================
# Test results and clean directory
# =======================================================================================================
output_directory = parameters["optimization_settings"]["output"]["output_directory"].GetString()
optimization_log_filename = parameters["optimization_settings"]["output"]["optimization_log_filename"].GetString() + ".csv"
optimization_model_part_name = parameters["optimization_settings"]["model_settings"]["model_part_name"].GetString()

# Testing
original_directory = os.getcwd()
os.chdir(output_directory)

with open(optimization_log_filename, 'r') as csvfile:
    reader = csv.reader(csvfile, delimiter=',')

    all_lines = []
    last_line = None
    for line in reader:
        if not line:
            continue
        else:
            all_lines.append(line)
            last_line = line

    resulting_optimization_iterations = int(last_line[0].strip())
    resulting_improvement = float(last_line[2].strip())
    resulting_c1 = float(last_line[4].strip())
    resulting_c2 = float(last_line[6].strip())

    TestCase().assertEqual(resulting_optimization_iterations, 4)
    TestCase().assertAlmostEqual(resulting_improvement, -2.37411E+01, 4)
    TestCase().assertAlmostEqual(resulting_c1, 1.07574E+01, 4)
    TestCase().assertAlmostEqual(resulting_c2, 4.27570E-01, 4)


os.chdir(original_directory)

# =======================================================================================================