/*
 * SWE_Sphere_TS_lg_irk_lf_n_erk.cpp
 *
 *  Created on: 11 Nov 2017
 *      Author: Martin Schreiber <M.Schreiber@exeter.ac.uk>
 */

#include "SWE_Sphere_TS_lg_irk_lc_erk_ver01.hpp"



void SWE_Sphere_TS_lg_irk_lc_erk::run_timestep(
		SphereData &io_phi,		///< prognostic variables
		SphereData &io_vort,	///< prognostic variables
		SphereData &io_div,		///< prognostic variables

		double i_dt,		///< if this value is not equal to 0, use this time step size instead of computing one
		double i_simulation_timestamp
)
{
	if (timestepping_order == 1)
	{
		SphereData tmp_phi = io_phi;
		SphereData tmp_vort = io_vort;
		SphereData tmp_div = io_div;

		// first order IRK for linear
		timestepping_lg_irk.run_timestep(
				io_phi, io_vort, io_div,
				i_dt,
				i_simulation_timestamp
			);


		SphereData phi_dt(io_phi.sphereDataConfig);
		SphereData vort_dt(io_vort.sphereDataConfig);
		SphereData div_dt(io_div.sphereDataConfig);

		// first order explicit for non-linear
		timestepping_lg_erk_lc_erk.euler_timestep_update_coriolis_and_nonlinear(
				tmp_phi, tmp_vort, tmp_div,
				phi_dt, vort_dt, div_dt,
				i_simulation_timestamp
			);


		io_phi += i_dt*phi_dt;
		io_vort += i_dt*vort_dt;
		io_div += i_dt*div_dt;
	}
	else if (timestepping_order == 2)
	{
		if (version_id == 0)
		{
			// HALF time step for linear part
			timestepping_lg_cn.run_timestep(
					io_phi, io_vort, io_div,
					i_dt*0.5,
					i_simulation_timestamp
				);

			// FULL time step for non-linear part
			timestepping_rk_nonlinear.run_timestep(
					&timestepping_lg_erk_lc_erk,
					&SWE_Sphere_TS_lg_erk_lc_erk::euler_timestep_update_coriolis_and_nonlinear,	///< pointer to function to compute euler time step updates
					io_phi, io_vort, io_div,
					i_dt,
					timestepping_order,		/// This must be 2nd order accurate to get overall 2nd order accurate method
					i_simulation_timestamp
				);

			// HALF time step for linear part
			timestepping_lg_cn.run_timestep(
					io_phi, io_vort, io_div,
					i_dt*0.5,
					i_simulation_timestamp+i_dt*0.5	/* TODO: CHECK THIS, THIS MIGHT BE WRONG!!! */
				);
		}
		else if (version_id == 1)
		{
			// HALF time step for non-linear part
			timestepping_rk_nonlinear.run_timestep(
					&timestepping_lg_erk_lc_erk,
					&SWE_Sphere_TS_lg_erk_lc_erk::euler_timestep_update_coriolis_and_nonlinear,	///< pointer to function to compute euler time step updates
					io_phi, io_vort, io_div,
					i_dt*0.5,
					timestepping_order,		/// This must be 2nd order accurate to get overall 2nd order accurate method
					i_simulation_timestamp
				);

			// FULL time step for linear part
			timestepping_lg_cn.run_timestep(
					io_phi, io_vort, io_div,
					i_dt,
					i_simulation_timestamp
				);

			// HALF time step for non-linear part
			timestepping_rk_nonlinear.run_timestep(
					&timestepping_lg_erk_lc_erk,
					&SWE_Sphere_TS_lg_erk_lc_erk::euler_timestep_update_coriolis_and_nonlinear,	///< pointer to function to compute euler time step updates
					io_phi, io_vort, io_div,
					i_dt*0.5,
					timestepping_order,		/// This must be 2nd order accurate to get overall 2nd order accurate method
					i_simulation_timestamp
				);
		}
		else
		{
			FatalError("Invalid verison id");
		}
	}
	else
	{
		FatalError("Not yet supported!");
	}
}



/*
 * Setup
 */
void SWE_Sphere_TS_lg_irk_lc_erk::setup(
		int i_order,	///< order of RK time stepping method
		int i_version_id
)
{
	version_id = i_version_id;

	timestepping_order = i_order;
	timestep_size = simVars.timecontrol.current_timestep_size;

	if (timestepping_order == 1)
	{
		timestepping_lg_irk.setup(
				1,
				timestep_size*0.5
		);
	}
	else if (timestepping_order == 2)
	{
		if (version_id == 0)
		{
			timestepping_lg_cn.setup(
					simVars.disc.crank_nicolson_filter,
					timestep_size*0.5
			);
		}
		else if (version_id == 1)
		{
			timestepping_lg_cn.setup(
					simVars.disc.crank_nicolson_filter,
					timestep_size
			);
		}
		else
		{
			FatalError("Invalid version id");
		}
	}
	else
	{
		FatalError("Invalid timestepping order");
	}


	//
	// Only request 1st order time stepping methods for irk and erk
	// These 1st order methods will be combined to higher-order methods in this class
	//
	timestepping_lg_erk_lc_erk.setup(1);
}



SWE_Sphere_TS_lg_irk_lc_erk::SWE_Sphere_TS_lg_irk_lc_erk(
		SimulationVariables &i_simVars,
		SphereOperators &i_op
)	:
		simVars(i_simVars),
		op(i_op),
		timestepping_order(-1),
		timestepping_lg_irk(simVars, op),
		timestepping_lg_cn(simVars, op),
		timestepping_lg_erk_lc_erk(simVars, op)
{
}



SWE_Sphere_TS_lg_irk_lc_erk::~SWE_Sphere_TS_lg_irk_lc_erk()
{
}

