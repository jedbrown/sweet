/*
 * SWE_Sphere_TS_lg_irk_lf_n_erk.hpp
 *
 *  Created on: 30 May 2017
 *      Author: Martin Schreiber <M.Schreiber@exeter.ac.uk>
 */

#ifndef SRC_PROGRAMS_SWE_SPHERE_REXI_SWE_SPHERE_TS_LG_IRK_LF_N_ERK_HPP_
#define SRC_PROGRAMS_SWE_SPHERE_REXI_SWE_SPHERE_TS_LG_IRK_LF_N_ERK_HPP_

#include <limits>
#include <sweet/sphere/SphereData.hpp>
#include <sweet/sphere/SphereDataTimesteppingExplicitRK.hpp>
#include <sweet/sphere/SphereOperators.hpp>
#include <sweet/SimulationVariables.hpp>

#include "SWE_Sphere_TS_interface.hpp"
#include "SWE_Sphere_TS_lg_irk.hpp"
#include "SWE_Sphere_TS_lg_cn.hpp"
#include "SWE_Sphere_TS_lg_erk_lc_n_erk.hpp"



class SWE_Sphere_TS_lg_irk_lc_n_erk	: public SWE_Sphere_TS_interface
{
	SimulationVariables &simVars;
	SphereOperators &op;

	int timestepping_order;
	int timestepping_order2;

	double timestep_size;

	/*
	 * Linear time steppers
	 */
	SWE_Sphere_TS_lg_irk timestepping_lg_irk;
	SWE_Sphere_TS_lg_cn timestepping_lg_cn;

	/*
	 * Non-linear time steppers
	 */
	SWE_Sphere_TS_lg_erk_lc_n_erk timestepping_lg_erk_lc_n_erk;

	SphereDataTimesteppingExplicitRK timestepping_rk_nonlinear;


public:
	SWE_Sphere_TS_lg_irk_lc_n_erk(
			SimulationVariables &i_simVars,
			SphereOperators &i_op
		);

	void setup(
			int i_order	///< order of RK time stepping method
	);

	void run_timestep(
			SphereData &io_phi,	///< prognostic variables
			SphereData &io_vort,	///< prognostic variables
			SphereData &io_div,	///< prognostic variables

			double i_fixed_dt = 0,		///< if this value is not equal to 0, use this time step size instead of computing one
			double i_simulation_timestamp = -1
	);



	virtual ~SWE_Sphere_TS_lg_irk_lc_n_erk();
};

#endif /* SRC_PROGRAMS_SWE_PLANE_REXI_SWE_PLANE_TS_LN_ERK_HPP_ */