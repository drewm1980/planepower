#include <acado_toolkit.hpp>

USING_NAMESPACE_ACADO

int main( void )
{
    #include <model.cpp>

    SIMexport sim(1, 0.02);
    sim.setModel( _f );
    sim.set( INTEGRATOR_TYPE, INT_IRK_RIIA5 );
    sim.set( NUM_INTEGRATOR_STEPS, 5 );
    sim.set( GENERATE_MATLAB_INTERFACE, YES );
    sim.set( MEASUREMENT_GRID, EQUIDISTANT_SUBGRID );
    
    sim.exportCode("export");
    return 0;
}
