/*
 * SWE_Plane_TS_l_rexi_n_etdrk.hpp
 *
 *  Created on: 29 May 2017
 *      Author: Martin Schreiber <M.Schreiber@exeter.ac.uk>
 */

#ifndef SRC_PROGRAMS_SWE_PLANE_SWE_PLANE_TS_L_REXI_N_ETDRK_HPP_
#define SRC_PROGRAMS_SWE_PLANE_SWE_PLANE_TS_L_REXI_N_ETDRK_HPP_

#include <limits>
#include <sweet/plane/PlaneData.hpp>
#include <sweet/plane/PlaneDataTimesteppingRK.hpp>
#include <sweet/SimulationVariables.hpp>
#include <sweet/plane/PlaneOperators.hpp>
#include "SWE_Plane_TS_l_rexi.hpp"

#include "../swe_plane/SWE_Plane_TS_interface.hpp"
#include "../swe_plane/SWE_Plane_TS_l_rexi.hpp"


class SWE_Plane_TS_l_rexi_n_etdrk	: public SWE_Plane_TS_interface
{
	SimulationVariables &simVars;
	PlaneOperators &op;

	SWE_Plane_TS_l_rexi ts_phi0_rexi;
	SWE_Plane_TS_l_rexi ts_phi1_rexi;
	SWE_Plane_TS_l_rexi ts_phi2_rexi;

	SWE_Plane_TS_l_rexi ts_ups0_rexi;
	SWE_Plane_TS_l_rexi ts_ups1_rexi;
	SWE_Plane_TS_l_rexi ts_ups2_rexi;
	SWE_Plane_TS_l_rexi ts_ups3_rexi;

	int timestepping_order;

private:
	void euler_timestep_update_nonlinear(
			const PlaneData &i_h,	///< prognostic variables
			const PlaneData &i_u,	///< prognostic variables
			const PlaneData &i_v,	///< prognostic variables

			PlaneData &o_h_t,	///< time updates
			PlaneData &o_u_t,	///< time updates
			PlaneData &o_v_t,	///< time updates

			double &o_dt,
			double i_dt,
			double i_max_timestamp
	);


public:
	SWE_Plane_TS_l_rexi_n_etdrk(
			SimulationVariables &i_simVars,
			PlaneOperators &i_op
		);

	void setup(
			REXI_SimulationVariables &i_rexi,
			int i_timestepping_order
	);

	void euler_timestep_update_nonlinear(
			const PlaneData &i_h,	///< prognostic variables
			const PlaneData &i_u,	///< prognostic variables
			const PlaneData &i_v,	///< prognostic variables

			PlaneData &o_h_t,	///< time updates
			PlaneData &o_u_t,	///< time updates
			PlaneData &o_v_t,	///< time updates

			double i_timestamp
	);

	void run_timestep(
			PlaneData &io_h,	///< prognostic variables
			PlaneData &io_u,	///< prognostic variables
			PlaneData &io_v,	///< prognostic variables

			double i_dt = 0,		///< if this value is not equal to 0, use this time step size instead of computing one
			double i_simulation_timestamp = -1
	);



	virtual ~SWE_Plane_TS_l_rexi_n_etdrk();
};

#endif /* SRC_PROGRAMS_SWE_PLANE_REXI_SWE_PLANE_TS_LN_ERK_HPP_ */
