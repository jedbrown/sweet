
#include "SphereDataVars.hpp"
#include "SphereDataCtx.hpp"
#include "ceval.hpp"

extern "C"
{
  void cecho_error(SphereData* sd, 
		   int step)
  {
    // not implemented
  }

  void cecho_residual(SphereData* sd, 
                      int step, 
	              int iter)
  {
    // not implemented
  }

  void cecho_output_solution(
			     SphereDataCtx *i_ctx,
			     SphereDataVars *i_Y,
			     int i_current_proc,
			     int i_current_step,
			     int i_current_iter,
			     int i_nnodes,
			     int i_niters
			     )
  {
    const SphereData& phi_Y  = i_Y->get_phi();
    const SphereData& vort_Y = i_Y->get_vort();
    const SphereData& div_Y  = i_Y->get_div();

    // get the SimulationVariables object from context
    SimulationVariables* simVars(i_ctx->get_simulation_variables());

    // write the data to file
    std::string filename = "prog_phi_current_proc_"+std::to_string(i_current_proc)
                                +"_current_step_"+std::to_string(i_current_step)
                                +"_current_iter_"+std::to_string(i_current_iter)
                                +"_nnodes_"      +std::to_string(i_nnodes)
                                +"_niters_"      +std::to_string(i_niters);
    write_file(*i_ctx, phi_Y, filename.c_str());

    filename = "prog_vort_current_proc_"+std::to_string(i_current_proc)
                    +"_current_step_"+std::to_string(i_current_step)
                    +"_current_iter_"+std::to_string(i_current_iter)
                    +"_nnodes_"      +std::to_string(i_nnodes)
                    +"_niters_"      +std::to_string(i_niters);
    write_file(*i_ctx, vort_Y, filename.c_str());

    filename = "prog_div_current_proc_"+std::to_string(i_current_proc)
                    +"_current_step_"+std::to_string(i_current_step)
                    +"_current_iter_"+std::to_string(i_current_iter)
                    +"_nnodes_"      +std::to_string(i_nnodes)
                    +"_niters_"      +std::to_string(i_niters);
    write_file(*i_ctx, div_Y, filename.c_str());

  }

}
