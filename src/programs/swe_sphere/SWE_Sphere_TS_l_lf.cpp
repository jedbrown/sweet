/*
 * SWE_Sphere_TS_l_lf.cpp
 *
 *  Created on: 30 May 2017
 *      Author: Martin Schreiber <M.Schreiber@exeter.ac.uk>
 */

#include "SWE_Sphere_TS_l_lf.hpp"





/*
 * Main routine for method to be used in case of finite differences
 */
void SWE_Sphere_TS_l_lf::euler_timestep_update(
		const SphereData &i_phi,	///< prognostic variables
		const SphereData &i_vort,	///< prognostic variables
		const SphereData &i_div,	///< prognostic variables

		SphereData &o_phi_t,	///< time updates
		SphereData &o_vort_t,	///< time updates
		SphereData &o_div_t,	///< time updates

		double i_fixed_dt,		///< if this value is not equal to 0, use this time step size instead of computing one
		double i_simulation_timestamp
)
{
	/*
	 * TIME STEP SIZE
	 */
	if (i_fixed_dt <= 0)
		FatalError("Only fixed time step size allowed");


	if (simVars.sim.f_sphere)
	{
		double gh = simVars.sim.gravitation * simVars.sim.h0;

		o_phi_t = -gh*i_div;
		o_div_t = -op.laplace(i_phi);

		o_vort_t = -simVars.sim.f0*i_div;
		o_div_t += simVars.sim.f0*i_vort;
	}
	else
	{
		double gh = simVars.sim.gravitation * simVars.sim.h0;

		SphereDataPhysical ug(i_phi.sphereDataConfig);
		SphereDataPhysical vg(i_phi.sphereDataConfig);
		op.robert_vortdiv_to_uv(i_vort, i_div, ug, vg);
		SphereDataPhysical phig = i_phi.getSphereDataPhysical();

		SphereDataPhysical tmpg1 = ug*fg;
		SphereDataPhysical tmpg2 = vg*fg;

		op.robert_uv_to_vortdiv(tmpg1, tmpg2, o_div_t, o_vort_t);

		o_vort_t *= -1.0;

		tmpg1 = ug*gh;
		tmpg2 = vg*gh;

		SphereData tmpspec(i_phi.sphereDataConfig);
		op.robert_uv_to_vortdiv(tmpg1,tmpg2, tmpspec, o_phi_t);

		o_phi_t *= -1.0;

		tmpspec = phig;
		tmpspec.request_data_spectral();
		o_div_t += -op.laplace(tmpspec);
	}
}



void SWE_Sphere_TS_l_lf::run_timestep(
		SphereData &io_phi,		///< prognostic variables
		SphereData &io_vort,	///< prognostic variables
		SphereData &io_div,		///< prognostic variables

		double i_fixed_dt,		///< if this value is not equal to 0, use this time step size instead of computing one
		double i_simulation_timestamp
)
{

	if (i_fixed_dt <= 0)
		FatalError("Only constant time step size allowed");


	// standard time stepping
	timestepping_lf.run_timestep(
			this,
			&SWE_Sphere_TS_l_lf::euler_timestep_update,	///< pointer to function to compute euler time step updates
			io_phi, io_vort, io_div,
			i_fixed_dt,
			timestepping_order,
			i_simulation_timestamp
		);
}



/*
 * Setup
 */
void SWE_Sphere_TS_l_lf::setup(
		int i_order,	///< order of RK time stepping method
		double i_robert_asselin_filter
)
{
	timestepping_order = i_order;
	robert_asselin_filter = i_robert_asselin_filter;

	timestepping_lf.setup(robert_asselin_filter);

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


SWE_Sphere_TS_l_lf::SWE_Sphere_TS_l_lf(
		SimulationVariables &i_simVars,
		SphereOperators &i_op
)	:
		simVars(i_simVars),
		op(i_op),
		timestepping_lf(i_op.sphereDataConfig),
		fg(i_op.sphereDataConfig)
{
}



SWE_Sphere_TS_l_lf::~SWE_Sphere_TS_l_lf()
{
}

