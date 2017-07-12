/*
 * Burgers_Plane_TS_ln_erk.cpp
 *
 *  Created on: 17 June 2017
 *      Author: Andreas Schmitt <aschmitt@fnb.tu-darmstadt.de>
 *
 */

#include "Burgers_Plane_TS_ln_erk.hpp"





/*
 * Main routine for method to be used in case of finite differences
 */
void Burgers_Plane_TS_ln_erk::euler_timestep_update(
		const PlaneData &i_tmp,	///< prognostic variables (perturbed part of height)
		const PlaneData &i_u,	///< prognostic variables
		const PlaneData &i_v,	///< prognostic variables

		PlaneData &o_tmp_t,	///< time updates
		PlaneData &o_u_t,	///< time updates
		PlaneData &o_v_t,	///< time updates

		double &o_dt,			///< time step restriction
		double i_fixed_dt,		///< if this value is not equal to 0, use this time step size instead of computing one
		double i_simulation_timestamp
)
{
	if (simVars.misc.verbosity > 2)
		std::cout << "p_run_euler_timestep_update()" << std::endl;

	//TODO: staggering vs. non staggering

	PlaneData f(i_u.planeDataConfig);
	BurgersValidationBenchmarks::set_source(i_simulation_timestamp,simVars,simVars.disc.use_staggering,f);
	o_tmp_t.physical_set_all(0);

	// u and v updates
	o_u_t = -(i_u*op.diff_c_x(i_u) + i_v*op.diff_c_y(i_u));
	o_u_t += simVars.sim.viscosity*(op.diff2_c_x(i_u)+op.diff2_c_y(i_u));
	o_u_t += f; // Delete this line if no source is used

	o_v_t = -(i_u*op.diff_c_x(i_v) + i_v*op.diff_c_y(i_v));
	o_v_t += simVars.sim.viscosity*(op.diff2_c_x(i_v)+op.diff2_c_y(i_v));

	o_tmp_t.physical_set_all(0);

	// Time step size
	if (i_fixed_dt > 0)
	{
		o_dt = i_fixed_dt;
	}
	else
	{
		std::cout << "Only fixed time step size supported" << std::endl;
		assert(false);
		exit(1);
	}
}



void Burgers_Plane_TS_ln_erk::run_timestep(
		PlaneData &io_u,	///< prognostic variables
		PlaneData &io_v,	///< prognostic variables
		PlaneData &io_u_prev,	///< prognostic variables
		PlaneData &io_v_prev,	///< prognostic variables

		double &o_dt,			///< time step restriction
		double i_fixed_dt,		///< if this value is not equal to 0, use this time step size instead of computing one
		double i_simulation_timestamp,
		double i_max_simulation_time
)
{
	if (i_fixed_dt <= 0)
		FatalError("Burgers_Plane_TS_ln_erk: Only constant time step size allowed");

	if (i_simulation_timestamp + i_fixed_dt > i_max_simulation_time)
		i_fixed_dt = i_max_simulation_time-i_simulation_timestamp;

	o_dt = i_fixed_dt;

	// setup dummy data
	PlaneData tmp(io_u.planeDataConfig);
#if SWEET_USE_PLANE_SPECTRAL_SPACE
	tmp.spectral_set_all(0,0);
#endif
	tmp.physical_set_all(0);

	if (timestepping_order != 21)
	{
		// setup dummy data
		tmp.physical_set_all(0);

		// run standard Runge Kutta
		timestepping_rk.run_timestep(
			this,
			&Burgers_Plane_TS_ln_erk::euler_timestep_update,	///< pointer to function to compute euler time step updates
			tmp, io_u, io_v,
			o_dt,
			i_fixed_dt,
			timestepping_order,
			i_simulation_timestamp,
			i_max_simulation_time
		);
	}
	// Explicit Runge-Kutta with order 1 in diffusion and order 2 in advection
	else
	{
		if (simVars.misc.verbosity > 2)
			std::cout << "run_timestep_imex()" << std::endl;

		PlaneData u=io_u;
		PlaneData v=io_v;

		// Modify timestep to final time if necessary
		double& t = o_dt;
		if (simVars.timecontrol.current_simulation_time+i_fixed_dt < simVars.timecontrol.max_simulation_time)
			t = i_fixed_dt;
		else
			t = simVars.timecontrol.max_simulation_time-simVars.timecontrol.current_simulation_time;

		// Initialize and set timestep dependent source for manufactured solution
		PlaneData f(io_u.planeDataConfig);
		PlaneData ff(io_u.planeDataConfig);

		BurgersValidationBenchmarks::set_source(simVars.timecontrol.current_simulation_time,simVars,simVars.disc.use_staggering,f);
		BurgersValidationBenchmarks::set_source(simVars.timecontrol.current_simulation_time+0.5*t,simVars,simVars.disc.use_staggering,ff);

		f.request_data_spectral();
		ff.request_data_spectral();

		if (simVars.disc.use_spectral_basis_diffs) //spectral
		{
			PlaneData u1 = u + t*simVars.sim.viscosity*(op.diff2_c_x(u)+op.diff2_c_y(u))
						   - 0.5*t*(u*op.diff_c_x(u)+v*op.diff_c_y(u)) + f*t;
			PlaneData v1 = v + t*simVars.sim.viscosity*(op.diff2_c_x(v)+op.diff2_c_y(v))
						   - 0.5*t*(u*op.diff_c_x(v)+v*op.diff_c_y(v));

			io_u = u + t*simVars.sim.viscosity*(op.diff2_c_x(u1)+op.diff2_c_y(u1))
				  - t*(u1*op.diff_c_x(u1)+v1*op.diff_c_y(u1)) +ff*t;
			io_v = v + t*simVars.sim.viscosity*(op.diff2_c_x(v1)+op.diff2_c_y(v1))
				  - t*(u1*op.diff_c_x(v1)+v1*op.diff_c_y(v1));

		} else { //Jacobi
			FatalError("NOT available");
		}
	}
}



/*
 * Setup
 */
void Burgers_Plane_TS_ln_erk::setup(
		int i_order	///< order of RK time stepping method
)
{
	timestepping_order = i_order;
}


Burgers_Plane_TS_ln_erk::Burgers_Plane_TS_ln_erk(
		SimulationVariables &i_simVars,
		PlaneOperators &i_op
)	:
		simVars(i_simVars),
		op(i_op)
{
	setup(simVars.disc.timestepping_order);
}



Burgers_Plane_TS_ln_erk::~Burgers_Plane_TS_ln_erk()
{
}
