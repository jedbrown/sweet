/*
 * SWE_Sphere_TS_ln_erk.cpp
 *
 *  Created on: 30 May 2017
 *      Author: Martin Schreiber <M.Schreiber@exeter.ac.uk>
 */

#include "SWE_Sphere_TS_ln_erk.hpp"





/*
 * Main routine for method to be used in case of finite differences
 */
void SWE_Sphere_TS_ln_erk::euler_timestep_update(
		const SphereData &i_phi,	///< prognostic variables
		const SphereData &i_vort,	///< prognostic variables
		const SphereData &i_div,	///< prognostic variables

		SphereData &o_phi_t,	///< time updates
		SphereData &o_vort_t,	///< time updates
		SphereData &o_div_t,	///< time updates

		double &o_dt,			///< time step restriction
		double i_fixed_dt,		///< if this value is not equal to 0, use this time step size instead of computing one
		double i_simulation_timestamp
)
{
	/*
	 * TIME STEP SIZE
	 */
	if (i_fixed_dt <= 0)
		FatalError("Only fixed time step size allowed");

	o_dt = i_fixed_dt;


	/*
	 * NON-LINEAR
	 *
	 * Follows Hack & Jakob formulation
	 */

	SphereDataPhysical ug(i_phi.sphereDataConfig);
	SphereDataPhysical vg(i_phi.sphereDataConfig);

	SphereDataPhysical vrtg = i_vort.getSphereDataPhysical();
	SphereDataPhysical divg = i_div.getSphereDataPhysical();
	op.robert_vortdiv_to_uv(i_vort, i_div, ug, vg);
	SphereDataPhysical phig = i_phi.getSphereDataPhysical();

	SphereDataPhysical tmpg1 = ug*(vrtg+fg);
	SphereDataPhysical tmpg2 = vg*(vrtg+fg);

	op.robert_uv_to_vortdiv(tmpg1, tmpg2, o_div_t, o_vort_t);

	o_vort_t *= -1.0;

	SphereDataPhysical tmpg = o_div_t.getSphereDataPhysical();

	tmpg1 = ug*phig;
	tmpg2 = vg*phig;

	SphereData tmpspec(i_phi.sphereDataConfig);
	op.robert_uv_to_vortdiv(tmpg1,tmpg2, tmpspec, o_phi_t);

	o_phi_t *= -1.0;

	tmpspec = (phig+0.5*(ug*ug+vg*vg));
	tmpspec.request_data_spectral();
	o_div_t += -op.laplace(tmpspec);
}



void SWE_Sphere_TS_ln_erk::run_timestep(
		SphereData &io_phi,		///< prognostic variables
		SphereData &io_vort,	///< prognostic variables
		SphereData &io_div,		///< prognostic variables

		double &o_dt,			///< time step restriction
		double i_fixed_dt,		///< if this value is not equal to 0, use this time step size instead of computing one
		double i_simulation_timestamp,
		double i_max_simulation_time
)
{

	if (i_fixed_dt <= 0)
		FatalError("Only constant time step size allowed");

	if (i_simulation_timestamp + i_fixed_dt > i_max_simulation_time)
		i_fixed_dt = i_max_simulation_time-i_simulation_timestamp;

	o_dt = i_fixed_dt;

	// standard time stepping
	timestepping_rk.run_timestep(
			this,
			&SWE_Sphere_TS_ln_erk::euler_timestep_update,	///< pointer to function to compute euler time step updates
			io_phi, io_vort, io_div,
			o_dt,
			i_fixed_dt,
			timestepping_order,
			i_simulation_timestamp,
			i_max_simulation_time
		);
}



/*
 * Setup
 */
void SWE_Sphere_TS_ln_erk::setup(
		int i_order	///< order of RK time stepping method
)
{
	timestepping_order = i_order;

	if (simVars.sim.f_sphere)
	{
		fg.physical_update_lambda_gaussian_grid(
			[&](double lon, double mu, double &o_data)
			{
				o_data = simVars.sim.f0;
			}
		);
	}
	else
	{
		fg.physical_update_lambda_gaussian_grid(
			[&](double lon, double mu, double &o_data)
			{
				o_data = mu*2.0*simVars.sim.coriolis_omega;
			}
		);
	}

}


SWE_Sphere_TS_ln_erk::SWE_Sphere_TS_ln_erk(
		SimulationVariables &i_simVars,
		SphereOperators &i_op
)	:
		simVars(i_simVars),
		op(i_op),
		fg(i_op.sphereDataConfig)
{
	setup(simVars.disc.timestepping_order);
}



SWE_Sphere_TS_ln_erk::~SWE_Sphere_TS_ln_erk()
{
}

