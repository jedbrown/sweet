/*
 * Burgers_Plane_TS_l_irk.cpp
 *
 *  Created on: 17 June 2017
 *      Author: Andreas Schmitt <aschmitt@fnb.tu-darmstadt.de>
 *
 */

#include "Burgers_Plane_TS_l_irk.hpp"


void Burgers_Plane_TS_l_irk::run_timestep(
		PlaneData &io_u,	///< prognostic variables
		PlaneData &io_v,	///< prognostic variables
		PlaneData &io_u_prev,	///< prognostic variables
		PlaneData &io_v_prev,	///< prognostic variables

		double i_fixed_dt,		///< if this value is not equal to 0, use this time step size instead of computing one
		double i_simulation_timestamp
)
{
	if (i_fixed_dt <= 0)
		FatalError("Burgers_Plane_TS_l_irk: Only constant time step size allowed");


	// setup dummy data
	PlaneData tmp(io_u.planeDataConfig);
#if SWEET_USE_PLANE_SPECTRAL_SPACE
	tmp.spectral_set_all(0,0);
#endif
	tmp.physical_set_all(0);


	// Setting explicit right hand side and operator of the left hand side
	PlaneData rhs_u = io_u;
	PlaneData rhs_v = io_v;

	if (simVars.disc.use_spectral_basis_diffs) //spectral
	{
		PlaneData lhs = io_u;

		if (timestepping_order == 1)
		{
			lhs = ((-i_fixed_dt)*simVars.sim.viscosity*(op.diff2_c_x + op.diff2_c_y)).spectral_addScalarAll(1.0);

			io_u = rhs_u.spectral_div_element_wise(lhs);
			io_v = rhs_v.spectral_div_element_wise(lhs);
		}
		else if (timestepping_order ==2)
		{
			rhs_u = simVars.sim.viscosity*(op.diff2_c_x(rhs_u) + op.diff2_c_y(rhs_u));
			rhs_v = simVars.sim.viscosity*(op.diff2_c_x(rhs_v) + op.diff2_c_y(rhs_v));
			lhs = ((-0.5*i_fixed_dt)*simVars.sim.viscosity*(op.diff2_c_x + op.diff2_c_y)).spectral_addScalarAll(1.0);

			PlaneData k1_u = rhs_u.spectral_div_element_wise(lhs);
			PlaneData k1_v = rhs_v.spectral_div_element_wise(lhs);

			io_u = io_u + i_fixed_dt*k1_u;
			io_v = io_v + i_fixed_dt*k1_v;
		}
		else
			FatalError("This timestepping order is not available with l_irk");

	} else { //Jacobi
		FatalError("NOT available");
	}

}



/*
 * Setup
 */
void Burgers_Plane_TS_l_irk::setup(
		int i_order	///< order of RK time stepping method
)
{
	timestepping_order = i_order;
}


Burgers_Plane_TS_l_irk::Burgers_Plane_TS_l_irk(
		SimulationVariables &i_simVars,
		PlaneOperators &i_op
)	:
		simVars(i_simVars),
		op(i_op)
{
	setup(simVars.disc.timestepping_order);
}



Burgers_Plane_TS_l_irk::~Burgers_Plane_TS_l_irk()
{
}

