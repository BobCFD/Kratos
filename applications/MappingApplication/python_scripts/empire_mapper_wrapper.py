import KratosMultiphysics as KM
from KratosMultiphysics.MappingApplication.python_mapper import PythonMapper

import os
import ctypes as ctp

def CreateMapper(model_part_origin, model_part_destination, mapper_settings):
    empire_mapper_type =  mapper_settings["mapper_type"].GetString()[7:] # returns the mapper-type without preceeding "empire_"

    if empire_mapper_type == "nearest_neighbor":
        return EmpireNearestNeighborMapper(model_part_origin, model_part_destination, mapper_settings)

    elif empire_mapper_type == "nearest_element":
        return EmpireNearestElementMapper(model_part_origin, model_part_destination, mapper_settings)

    elif empire_mapper_type == "barycentric":
        return EmpireBaryCentricMapper(model_part_origin, model_part_destination, mapper_settings)

    elif empire_mapper_type == "mortar":
        return EmpireMortarMapper(model_part_origin, model_part_destination, mapper_settings)

    else:
        raise Exception('Mapper "{}" not available in empire!'.format(empire_mapper_type))


class EmpireMapperWrapper(PythonMapper):
    """Wrapper for the mappers of Empire with the same Interface as the Mappers in Kratos
    This wrapper requires the development version of Empire which has the mapperlib exposed separately
    """

    mapper_count = 0
    mapper_lib = None

    def __init__(self, model_part_origin, model_part_destination, mapper_settings):
        super(EmpireMapperWrapper, self).__init__(model_part_origin, model_part_destination, mapper_settings)

        # TODO use gather-mp utility for mapping in MPI
        if model_part_origin.IsDistributed() or model_part_destination.IsDistributed():
            raise Exception('{} does not yet support mapping with distributed ModelParts!'.format(self._ClassName()))

        EmpireMapperWrapper.mapper_count += 1 # required for identification purposes
        self.__inverse_mapper = None

        if EmpireMapperWrapper.mapper_lib == None: # load it only once
            KM.Logger.PrintInfo("EmpireMapperWrapper", "Loading mapper lib ...")
            EmpireMapperWrapper.mapper_lib = self.__LoadMapperLib()


    def __LoadMapperLib(self):
        KM.Logger.PrintInfo("EmpireMapperWrapper", "Determining path to mapper lib")
        # first try automatic detection using the environment that is set by Empire => startEMPIRE
        if ('EMPIRE_MAPPER_LIBSO_ON_MACHINE' in os.environ):
            KM.Logger.PrintInfo("EmpireMapperWrapper", "EMPIRE_MAPPER_LIBSO_ON_MACHINE found in environment")
            mapper_lib_path = os.environ['EMPIRE_API_LIBSO_ON_MACHINE']

        else:
            KM.Logger.PrintInfo("EmpireMapperWrapper", "EMPIRE_MAPPER_LIBSO_ON_MACHINE NOT found in environment, using manually specified path to load the mapper lib")
            mapper_lib_path = self.mapper_settings["mapper_lib"].GetString()
            if mapper_lib_path == "":
                raise Exception('The automatic detection of the mapper lib failed, the path to the mapper lib has to be specified with "mapper_lib"')

        KM.Logger.PrintInfo("EmpireMapperWrapper", "Attempting to load the mapper lib")
        # TODO check if still both are needed! (does the mapperlib link to MPI?)
        try:
            try: # OpenMPI
                loaded_mapper_lib = ctp.CDLL(mapper_lib_path, ctp.RTLD_GLOBAL)
                KM.Logger.PrintInfo('EmpireMapperWrapper', 'Using standard OpenMPI')
            except: # Intel MPI or OpenMPI compiled with "–disable-dlopen" option
                loaded_mapper_lib = ctp.cdll.LoadLibrary(mapper_lib_path)
                KM.Logger.PrintInfo('EmpireMapperWrapper', 'Using Intel MPI or OpenMPI compiled with "–disable-dlopen" option')
        except OSError:
            raise Exception("Mapper lib could not be loaded!")

        KM.Logger.PrintInfo("EmpireMapperWrapper", "Successfully loaded the mapper lib")

        return loaded_mapper_lib



    def Map(self, variable_origin, variable_destination, mapper_flags=KM.Flags()):
        if not type() # check variables are matching

        var_dim = GetVariableDimension(variable_origin)

        origin_data_size = len(self.origin_model_part.Nodes)*var_dim
        destination_data_size = len(self.destination_model_part.Nodes)*var_dim

        c_origin_array = self.__KratosFieldToCArray(var_dim, self.origin_model_part, origin_variable)
        c_destination_array = (ctp.c_double * destination_data_size)(0.0)

        self.mapper_library.doConsistentMapping(
            ctp.c_char_p(self.name),
            ctp.c_int(var_dim),
            ctp.c_int(origin_data_size),
            c_origin_array,
            ctp.c_int(destination_data_size),
            c_destination_array
            )

        self.__CArrayToKratosField(
            var_dim,
            c_destination_array,
            destination_data_size,
            self.destination_model_part,
            destination_variable)

    def InverseMap(self, variable_origin, variable_destination, mapper_flags=KM.Flags()):
        # TODO check if using transpose => conservative

        if self.__inverse_mapper == None:
            self.__CreateInverseMapper()
        self.__inverse_mapper.Map(variable_destination, variable_origin, mapper_flags) # TODO check this!
        raise NotImplementedError('"InverseMap" was not implemented for "{}"'.format(self._ClassName()))

    def UpdateInterface(self):
        # this requires recreating the mapper completely!
        super(EmpireMapperWrapper, self).UpdateInterface()


    def __BuildCouplingMatrices(self):
        if EmpireMapperWrapper.mapper_lib.hasMapper(ctp.c_char_p(self.name)):
            EmpireMapperWrapper.mapper_lib.buildCouplingMatrices(ctp.c_char_p(self.name))
        else:
            pass # TODO print warning


    def __CreateInverseMapper(self):
        return self.__class__(self.model_part_destination, self.model_part_origin, self.mapper_settings) # TODO check this!


    def __del__(self):
        EmpireMapperWrapper.mapper_count -= 1

        if EmpireMapperWrapper.mapper_count == 0: # last mapper was destoyed
            if self.echo_level > 1:
                KM.Logger.PrintInfo('EmpireMapperWrapper', 'Destroying last instance, deleting all meshes & mappers')
            #  delete everything to make sure nothing is left
            EmpireMapperWrapper.mapper_lib.deleteAllMeshes()
            EmpireMapperWrapper.mapper_lib.deleteAllMappers()


    def __MakeEmpireFEMesh(self, mesh_name, model_part):

        c_mesh_name       = ctp.c_char_p(mesh_name)
        c_num_nodes       = ctp.c_int(len(model_part.Nodes))
        c_num_elems       = ctp.c_int(len(model_part.Conditions))
        c_node_ids        = (ctp.c_int * c_num_nodes.value) (0)
        c_node_coords     = (ctp.c_double * (3 * c_num_nodes.value))(0.0)
        c_num_nodes_per_elem = (ctp.c_int * c_num_elems.value) (0)

        for i_node, node in enumerate(model_part.Nodes):
            c_node_ids[i_node] = node.Id
            c_node_coords[i_node*3]   = node.X
            c_node_coords[i_node*3+1] = node.Y
            c_node_coords[i_node*3+2] = node.Z

        elem_node_ctr = 0
        for elem_ctr, elem in enumerate(model_part.Conditions):
            c_num_nodes_per_elem[elem_ctr] = len(elem.GetNodes())
            elem_node_ctr += c_num_nodes_per_elem[elem_ctr]

        elem_index = 0
        c_elems = (ctp.c_int * elem_node_ctr) (0)
        for elem_ctr, elem in enumerate(model_part.Conditions):
            for elem_node_ctr, elem_node in enumerate(elem.GetNodes()):
                c_elems[elem_index + elem_node_ctr] = elem_node.Id
            elem_index += len(elem.GetNodes())

        self.mapper_library.initFEMesh(c_mesh_name, c_num_nodes, c_num_elems, False)
        self.mapper_library.setNodesToFEMesh(c_mesh_name, c_node_ids, c_node_coords)
        self.mapper_library.setElementsToFEMesh(c_mesh_name, c_num_nodes_per_elem, c_elems)

    @classmethod
    def _GetDefaultSettings(cls):
        this_defaults = KM.Parameters("""{
            "mapper_lib"  : ""
        }""")
        this_defaults.AddMissingParameters(super(EmpireMapperWrapper, cls)._GetDefaultSettings())
        return this_defaults


class EmpireNearestNeighborMapper(EmpireMapperWrapper):
    """Wrapper for the nearest neighbor mapper of Empire"""

    def __init__(self, model_part_origin, model_part_destination, mapper_settings):
        super(EmpireNearestNeighborMapper, self).__init__(model_part_origin, model_part_destination, mapper_settings)

        # create Mapper

        self.__BuildCouplingMatrices()


class EmpireNearestElementMapper(EmpireMapperWrapper):
    """Wrapper for the nearest element mapper of Empire"""

    def __init__(self, model_part_origin, model_part_destination, mapper_settings):
        super(EmpireNearestElementMapper, self).__init__(model_part_origin, model_part_destination, mapper_settings)

        # create Mapper

        self.__BuildCouplingMatrices()


class EmpireBarycentricMapper(EmpireMapperWrapper):
    """Wrapper for the barycentric mapper of Empire"""

    def __init__(self, model_part_origin, model_part_destination, mapper_settings):
        super(EmpireBarycentricMapper, self).__init__(model_part_origin, model_part_destination, mapper_settings)

        # create Mapper

        self.__BuildCouplingMatrices()


class EmpireMortarMapper(EmpireMapperWrapper):
    """Wrapper for the mortar mapper of Empire"""

    def __init__(self, model_part_origin, model_part_destination, mapper_settings):
        super(EmpireMortarMapper, self).__init__(model_part_origin, model_part_destination, mapper_settings)

        # create Mapper

        self.__BuildCouplingMatrices()

    @classmethod
    def _GetDefaultSettings(cls):
        this_defaults = KM.Parameters("""{
            "dual"                 : false,
            "enforce_consistency"  : false,
            "opposite_normals"     : false,
        }""")
        this_defaults.AddMissingParameters(super(EmpireMapperWrapper, cls)._GetDefaultSettings())
        return this_defaults



def KratosFieldToCArray(self, dim, model_part, variable, historical):
    size = dim * model_part.NumberOfNodes()

    CheckDimension()

    c_array = (ctp.c_double * size)(0.0)
    for i_node, node in enumerate(model_part.Nodes):
        node_value = node.GetSolutionStepValue(variable)
        if node_value.Size() != dim:
            raise RuntimeError("Wrong dimensions!")
        for i_dim in range(dim):
            c_array[i_node*dim + i_dim] = node_value[i_dim]

    return c_array

def CArrayToKratosField(self, dim, c_array, size, model_part, variable, historical, swap_sign, add_values):
    if size != dim * model_part.NumberOfNodes()
        raise RuntimeError("Wrong size!")

    for i_node, node in enumerate(model_part.Nodes):
        values = [ c_array[i_node*dim], c_array[i_node*dim+1], c_array[i_node*dim+2]]
        node.SetSolutionStepValue(variable, values)


def GetVariableDimension(variable):
    var_type = KM.KratosGlobals.GetVariableType(variable.Name)
    if var_type == "Array":
        return 3
    elif var_type in ["Double", "Component"]:
        return 1
    else:
        raise Exception('Wrong variable type: "{}". Only "Array", "Double" and "Component" are allowed'.format(var_type))
