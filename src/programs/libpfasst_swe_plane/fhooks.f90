module hooks_module
  use pf_mod_dtype
  use encap_module
  use feval_module
  implicit none

  interface
     
     ! prototypes of the C functions

     subroutine cecho_error(sd,step) bind(c, name="cecho_error")
       use iso_c_binding
       type(c_ptr),intent(in),value :: sd
       integer,intent(in)           :: step
     end subroutine cecho_error

     subroutine cecho_residual(sd,step,iter) bind(c, name="cecho_residual")
       use iso_c_binding
       type(c_ptr),intent(in),value :: sd
       integer,intent(in)           :: step,iter
     end subroutine cecho_residual
  end interface

contains

  ! error function

  subroutine fecho_error(pf, level, state) 
    use iso_c_binding    
    type(pf_pfasst_t),  intent(inout) :: pf
    class(pf_level_t),  intent(inout) :: level
    type(pf_state_t),   intent(in)    :: state

    class(pf_encap_t),  allocatable   :: Y_reference

    if (state%t0 + state%dt > 999) then
     
       ! allocate memory for vector Y_exact
       call level%ulevel%factory%create_single(Y_reference, & 
                                               level%level, & 
                                               SDC_KIND_SOL_FEVAL, & 
                                               level%nvars, &
                                               level%shape)
       
       ! compute the reference solution (currently with explicit RK)
       call freference(level%ulevel%sweeper, &
                       state%t0+state%dt, & 
                       Y_reference)
    

       print *, 'time at which reference is computed:', &
            state%t0+state%dt
       
       ! compute the norm of the error
       call Y_reference%axpy(-1.0_pfdp, & 
                             level%qend)

       print '("error: step: ",i7.5," iter: ",i5.3," error: ",es14.7)', &
            state%step+1, state%iter, Y_reference%norm()
       
       ! release memory
       call level%ulevel%factory%destroy_single(Y_reference, &
                                                level%level, & 
                                                SDC_KIND_SOL_FEVAL, &
                                                level%nvars, &
                                                level%shape)
       
    end if
  end subroutine fecho_error

  
  ! function to output the residual 

  subroutine fecho_residual(pf, level, state)
    use iso_c_binding 
    use pf_mod_utils
    type(pf_pfasst_t), intent(inout) :: pf
    class(pf_level_t), intent(inout) :: level
    type(pf_state_t),  intent(in)    :: state

    select type(R => level%R(level%nnodes-1))
    type is (sweet_data_encap_t)
       
       print '("resid: step: ",i7.5," iter: ",i5.3," level: ",i2.2," resid: ",es14.7)', &
            state%step+1, state%iter, level%level, R%norm()

      !call cecho_residual(R%c_sweet_data_ptr, & 
      !                    state%step+1, & 
      !                    state%iter)

   class default
      stop "TYPE ERROR"
   end select

  end subroutine fecho_residual


  ! function to output the interpolation errors

  subroutine fecho_interpolation_errors(pf, level, state)
    use iso_c_binding
    use pf_mod_utils
    use pf_mod_restrict
    type(pf_pfasst_t), intent(inout) :: pf
    class(pf_level_t), intent(inout) :: level
    type(pf_state_t),  intent(in)    :: state

    class(pf_encap_t), allocatable   :: Q_coarse(:)                  !  Coarse in time and space
    class(pf_encap_t), allocatable   :: Q_fine(:)                    !  Fine in time and space
    class(pf_encap_t), allocatable   :: Q_fine_space_coarse_time(:)  !  Coarse in time but fine in space

    real(pfdp),        allocatable   :: c_times(:)
    real(pfdp),        allocatable   :: f_times(:)

    integer                          :: m
    
    if (level%level == pf%nlevels .and. pf%nlevels > 1) then
       
       ! allocate memory for the time nodes
       allocate(c_times(pf%levels(pf%nlevels-1)%nnodes))
       allocate(f_times(pf%levels(pf%nlevels)%nnodes))    
       c_times = state%t0 + state%dt * pf%levels(pf%nlevels-1)%nodes
       f_times = state%t0 + state%dt * pf%levels(pf%nlevels)%nodes
       
       ! allocate memory for the data values
       call pf%levels(pf%nlevels)%ulevel%factory%create_array(Q_fine,                       &
                                                              pf%levels(pf%nlevels)%nnodes, &
                                                              pf%levels(pf%nlevels)%level,  &
                                                              SDC_KIND_CORRECTION,          & 
                                                              pf%levels(pf%nlevels)%nvars,  &
                                                              pf%levels(pf%nlevels)%shape)
       call pf%levels(pf%nlevels-1)%ulevel%factory%create_array(Q_coarse,                       &
                                                                pf%levels(pf%nlevels-1)%nnodes, &
                                                                pf%levels(pf%nlevels-1)%level,  & 
                                                                SDC_KIND_CORRECTION,            &
                                                                pf%levels(pf%nlevels-1)%nvars,  &
                                                                pf%levels(pf%nlevels-1)%shape)
       call pf%levels(pf%nlevels)%ulevel%factory%create_array(Q_fine_space_coarse_time,       & 
                                                              pf%levels(pf%nlevels-1)%nnodes, &
                                                              pf%levels(pf%nlevels)%level,    &
                                                              SDC_KIND_CORRECTION,            & 
                                                              pf%levels(pf%nlevels)%nvars,    & 
                                                              pf%levels(pf%nlevels)%shape)
    
       ! copy the values at all SDC nodes into temporary vectors
       do m = 1, pf%levels(pf%nlevels-1)%nnodes
          call Q_coarse(m)%setval(0.0_pfdp)
       end do
       do m = 1, pf%levels(pf%nlevels)%nnodes
          call Q_fine(m)%copy(pf%levels(pf%nlevels)%Q(m))
       end do
       
       ! restrict in space and then in time
       call restrict_sdc(pf%levels(pf%nlevels),   &
                         pf%levels(pf%nlevels-1), & 
                         Q_fine,                  & 
                         Q_coarse,                &
                         .false.,                 &
                         f_times)
    
       
       do m = 1, pf%levels(pf%nlevels)%nnodes
          call Q_fine(m)%setval(0.0_pfdp)
       end do

       ! interpolate in space
       do m = 1, pf%levels(pf%nlevels-1)%nnodes
          call Q_fine_space_coarse_time(m)%setval(0.0_pfdp)
          call pf%levels(pf%nlevels)%ulevel%interpolate(pf%levels(pf%nlevels),       &
                                                        pf%levels(pf%nlevels-1),     &
                                                        Q_fine_space_coarse_time(m), & 
                                                        Q_coarse(m),                 & 
                                                        c_times(m))
       end do

       ! interpolate in time
       call pf_apply_mat(Q_fine,                     &
                         1.0_pfdp,                   &
                         pf%levels(pf%nlevels)%tmat, &
                         Q_fine_space_coarse_time,   &
                         .false.)

       ! compute the interpolation error
       do m = 1, pf%levels(pf%nlevels)%nnodes
          call Q_fine(m)%axpy(-1.0_pfdp, pf%levels(pf%nlevels)%Q(m))
          print *, 'interpolation_error: ', Q_fine(m)%norm()
       end do
         
       
       call pf%levels(pf%nlevels-1)%ulevel%factory%destroy_array(Q_coarse,  pf%levels(pf%nlevels-1)%nnodes, &
            pf%levels(pf%nlevels-1)%level, SDC_KIND_CORRECTION, pf%levels(pf%nlevels-1)%nvars, pf%levels(pf%nlevels-1)%shape)
       call pf%levels(pf%nlevels)%ulevel%factory%destroy_array(Q_fine, pf%levels(pf%nlevels-1)%nnodes, &
            pf%levels(pf%nlevels)%level, SDC_KIND_CORRECTION, pf%levels(pf%nlevels)%nvars, pf%levels(pf%nlevels)%shape)
       call pf%levels(pf%nlevels)%ulevel%factory%destroy_array(Q_fine_space_coarse_time, pf%levels(pf%nlevels-1)%nnodes, &
            pf%levels(pf%nlevels)%level, SDC_KIND_CORRECTION, pf%levels(pf%nlevels)%nvars, pf%levels(pf%nlevels)%shape)
              
    end if

  end subroutine fecho_interpolation_errors

end module hooks_module

