module extern
   use constants
   use procedures
   use rstrat_types
   use timestepping
   implicit none
contains

   subroutine stratify(ti, lme_error, in_t, in_f, ou_f, &
                       coszen, lw_abs, s_w, rh, t_air, u_2, &
                       resgeo, d_z, t_z, &
                       m_zn, a_d, d_v, v_zt, s_tin, m_cal) bind(C, name="stratify")

      integer(C_INT), intent(in), value :: ti

      ! Used to indicate problem with layer mass / energy subroutine if lme_error != 0
      ! Then quit the timestep subroutine and move on to the next reservoir
      integer(C_INT), intent(inout) :: lme_error

      ! Flow input data
      real(r8), intent(in) :: in_t      ! inflow temp(k)
      real(r8), intent(in) :: in_f      ! Inflow(m^3/s)
      real(r8), intent(inout) :: ou_f   ! outflow(m^3/s)

      ! Environmental forcing data
      real(r8), intent(in) :: coszen, & ! cosine of solar zenith angle
                              lw_abs, & ! atmospheric longwave absorbtion (W/m^2)
                              s_w, &    ! solar radiation (W/m**2)
                              rh, &     ! Relative humidity (%)
                              t_air, &  ! air temperature
                              u_2       ! wind speed at 2m(m/s)

      ! Reservoir geometry description
      type(reservoir_geometry), intent(inout) :: resgeo

      ! Reservoir depth / area / volume relationship
      type(res_dav) :: dav

      real(r8), intent(inout) :: m_cal              ! Reservoir calculated mass (kg)

      ! Information that is passed on to the next time step, and / or saved to file
      real(r8), intent(inout) :: d_z(nlayer_max)    ! Depth at z from bottom (m)
      real(r8), intent(inout) :: t_z(nlayer_max)    ! lake layer temperature

      real(r8), intent(inout) :: m_zn(nlayer_max)   ! Reservoir ending mass at depth z (kg)

      real(r8), intent(inout) :: a_d(nlayer_max)    ! Area at depth z (km2)
      real(r8), intent(inout) :: d_v(nlayer_max)    ! Reservoir volume change at layer (m^3)
      real(r8), intent(inout) :: v_zt(nlayer_max)   ! Total reservoir volume at depth z from surface(m3)

      real(r8), intent(inout) :: s_tin              ! Initial total storage (m^3)
      call rgeom(resgeo, dav)
      call layer_thickness(resgeo)
      ! We reapeat the same input 24 times to simulate the hourly time steps
      do k = 1, 24
         call stratify_internal(ti, lme_error, in_t, in_f, ou_f, &
                             coszen, lw_abs, s_w, rh, t_air, u_2, &
                             resgeo, d_z, t_z, &
                             m_zn, a_d, d_v, v_zt, s_tin, m_cal, dav)
      end do
   end subroutine stratify

   subroutine stratify_internal(ti, lme_error, in_t, in_f, ou_f, &
                                coszen, lw_abs, s_w, rh, t_air, u_2, &
                                resgeo, d_z, t_z, &
                                m_zn, a_d, d_v, v_zt, s_tin, m_cal, dav)

      integer(C_INT), intent(in), value :: ti
      ! Used to indicate problem with layer mass / energy subroutine if lme_error != 0

      ! Then quit the timestep subroutine and move on to the next reservoir
      integer(C_INT), intent(inout) :: lme_error

      real(r8), intent(in) :: in_t                  ! inflow temp(k)
      real(r8), intent(in) :: in_f                  ! Inflow(m^3/s),
      real(r8), intent(inout) :: ou_f               ! outflow(m^3/s)
      real(r8), intent(in) :: coszen, & ! cosine of solar zenith angle
                              lw_abs, & ! atmospheric longwave absorbtion (W/m^2)
                              s_w, &    ! solar radiation (W/m**2)
                              rh, &     ! Relative humidity (%)
                              t_air, &  ! air temperature
                              u_2       ! wind speed at 2m(m/s)
      ! Reservoir geometry description
      type(reservoir_geometry), intent(inout) :: resgeo
      type(res_dav), intent(inout) :: dav

      real(r8), intent(inout) :: m_cal              ! Reservoir calculated mass (kg)

      ! Information that is passed on to the next time step, and / or saved to file
      real(r8), intent(inout) :: d_z(nlayer_max)    ! Depth at z from bottom (m)
      real(r8), intent(inout) :: t_z(nlayer_max)    ! lake layer temperature

      real(r8), intent(inout) :: m_zn(nlayer_max)   ! Reservoir ending mass at depth z (kg)

      real(r8), intent(inout) :: a_d(nlayer_max)    ! Area at depth z (km2)
      real(r8), intent(inout) :: d_v(nlayer_max)    ! Reservoir volume change at layer (m^3)
      real(r8), intent(inout) :: v_zt(nlayer_max)   ! Total reservoir volume at depth z from surface(m3)

      real(r8), intent(inout) :: s_tin              ! Initial total storage (m^3)

      ! ---- Local variables (keeps information between subtimesteps) ----
      real(r8) :: rho_z(nlayer_max)  ! Depth based water density  (kg/m3)
      real(r8) :: enr_0(nlayer_max)                 ! Initial inner energy
      real(r8) :: d_res_sub             ! Reservoir depth, taken as 0.95*(dam height) in GRanD database, (m)
      real(r8) :: t_zsub(nlayer_max)    ! sub-timestep lake layer temperature (k)
      real(r8) :: cntr(nlayer_max), &   ! --
                  cntr1(nlayer_max), &  ! Counters to average sub-timestep temperature
                  cntr2(nlayer_max)     ! --

      real(r8) :: d_zsb(nlayer_max)    ! Depth at z from surface averaged over sub-timestep(m)
      integer :: ww                    ! Subtimestep index
      integer :: k                     ! Layer index
      ! Initialize
      d_res_sub = zero
      ! Initialize arrays
      t_zsub = zero
      cntr = zero
      cntr1 = zero
      cntr2 = zero
      d_zsb = zero

      if (ti == 1) then
         call init_subtimestep(m_zn, t_air, resgeo, dav, d_z, a_d, v_zt, d_v, &
                               rho_z, t_z, s_tin)
      else
         do k = 1, nlayer_max
            rho_z(k) = den(t_z(k))
         end do
      end if

      ! Start calculation for each sub-timestep
      do ww = 1, s_dtime
         call subtimestep(ww, ti, resgeo%n_depth, coszen, lw_abs, s_w, rh, t_air, u_2, &
                          t_z, v_zt, resgeo%d_res, &
                          in_f, rho_z, resgeo%A_cf, a_d, s_tin, resgeo%V_df, &
                          resgeo%d_ht, ou_f, in_t, d_v, resgeo%V_cf, m_zn, &
                          resgeo%dd_z, enr_0, dav%d_zi, d_z, dav%a_di, &
                          dav%v_zti, resgeo%ddz_min, resgeo%ddz_max, m_cal, &
                          lme_error, resgeo%M_W, resgeo%M_L, d_zsb, cntr, t_zsub, &
                          d_res_sub, cntr1, cntr2)
         if (lme_error == 1) then
            return
         end if
      end do

      ! This used to happen in finalise_timestep before reversing the
      ! array to write out now reversal happens in writedata
      resgeo%d_res = d_res_sub/s_dtime
      t_z(:) = t_zsub(:)/cntr(:)
      d_z(:) = d_zsb(:)/cntr(:)

   end subroutine stratify_internal

end module
