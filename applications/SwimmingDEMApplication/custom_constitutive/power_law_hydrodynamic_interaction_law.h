#ifndef KRATOS_SDEM_POWER_LAW_FLUID_HYDRODYNAMIC_INTERACTION_LAW_H
#define KRATOS_SDEM_POWER_LAW_FLUID_HYDRODYNAMIC_INTERACTION_LAW_H

#include "includes/define.h"
#include "includes/serializer.h"
#include "includes/model_part.h"
#include "containers/flags.h"

#include "hydrodynamic_interaction_law.h"

namespace Kratos {

class KRATOS_API(SWIMMING_DEM_APPLICATION) PowerLawFluidHydrodynamicInteractionLaw : public HydrodynamicInteractionLaw {

public:
    typedef Node <3> NodeType;

    // Pointer types for PowerLawFluidHydrodynamicInteractionLaw
    KRATOS_CLASS_POINTER_DEFINITION(PowerLawFluidHydrodynamicInteractionLaw);

    PowerLawFluidHydrodynamicInteractionLaw(): HydrodynamicInteractionLaw(){}

    PowerLawFluidHydrodynamicInteractionLaw(const DragLaw& r_drag_law): HydrodynamicInteractionLaw(r_drag_law){}

    PowerLawFluidHydrodynamicInteractionLaw(const InviscidForceLaw& r_inviscid_force_law): HydrodynamicInteractionLaw(r_inviscid_force_law){}

    PowerLawFluidHydrodynamicInteractionLaw(const DragLaw& r_drag_law,
                                            const InviscidForceLaw& r_inviscid_force_law): HydrodynamicInteractionLaw(r_drag_law, r_inviscid_force_law){}

    PowerLawFluidHydrodynamicInteractionLaw(const PowerLawFluidHydrodynamicInteractionLaw &rPowerLawFluidHydrodynamicInteractionLaw)
        : HydrodynamicInteractionLaw(rPowerLawFluidHydrodynamicInteractionLaw){}

    void Initialize(const ProcessInfo& r_process_info) override;

    std::string GetTypeOfLaw() override;

    /// Destructor

    virtual ~PowerLawFluidHydrodynamicInteractionLaw();

    PowerLawFluidHydrodynamicInteractionLaw::Pointer Clone() const;

    double ComputeShahParticleReynoldsNumber(const double particle_radius,
                                             const double fluid_density,
                                             const double consistency_index,
                                             const double flow_behavior_index,
                                             const double modulus_of_slip_velocity);

    void ComputeDragForce(Geometry<Node<3> >& r_geometry,
                          double particle_radius,
                          double fluid_density,
                          double fluid_kinematic_viscosity,
                          array_1d<double, 3>& slip_velocity,
                          array_1d<double, 3>& drag_force,
                          const ProcessInfo& r_current_process_info) override;

private:

    friend class Serializer;

    virtual void save(Serializer& rSerializer) const override {
        KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, Flags)

    }

    virtual void load(Serializer& rSerializer) override {
        KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, Flags)
    }

}; // Class PowerLawFluidHydrodynamicInteractionLaw : public MainCL

KRATOS_DEFINE_APPLICATION_VARIABLE(SWIMMING_DEM_APPLICATION, PowerLawFluidHydrodynamicInteractionLaw::Pointer, SDEM_POWER_LAW_FLUID_HYDRODYNAMIC_INTERACTION_LAW_POINTER)


} // Namespace Kratos

#endif // KRATOS_SDEM_POWER_LAW_FLUID_HYDRODYNAMIC_INTERACTION_LAW_H
