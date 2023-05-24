module constants
   use ISO_C_BINDING
   ! -------- Constants -------- !
   integer(C_INT), parameter :: r8 = C_DOUBLE
   ! integer(C_INT), parameter :: r8 = C_LONG_DOUBLE
   integer(C_INT), parameter :: nlayer_max = 30         ! Maximum number of layers
   integer(C_INT), parameter :: yr_max = 10             ! Maximum number of years simulated
   integer(C_INT), parameter :: dtime = 60              ! time step (sec)
   integer(C_INT) :: s_dtime = 3600/dtime               ! number of sub hourly time step
   integer(C_INT), parameter :: d_nn = 250              ! Number of vertical depth descretization to establish depth-area-volume relationship

   logical:: DEBUG = .false.                            ! Print debugging statements
   logical :: use_evap = .false.                        ! Turn on / off evaporation from volume / sfc temperature computations

   real(r8), parameter :: t_frz = 273.15_r8             ! freezing temperature (K)
   real(r8), parameter :: rho_w = 1.e3_r8               ! Water density (kg/m3)
   real(r8), parameter :: rho_a = 1.177_r8              ! Air density (kg/m3)
   real(r8), parameter :: c_w = 4.188e3_r8              ! Water specific heat capacity (J/kg/k)
   real(r8), parameter :: st_bl = 5.67e-8_r8            ! Stefan-Boltzmann constant ~ W/m^2/K^4
   real(r8), parameter :: F = 1.0_r8                    ! dimensionless factor for wind sheltering by riparian vegetation, Wu et al, 2012
   real(r8), parameter :: sfc_lyr_dpth = 0.60_r8        ! Surface layer depth (m)
   real(r8), parameter :: le = 2.501e6_r8               ! latent heat of vaporaization (kcal/kg)
   real(r8), parameter :: grav = 9.8062_r8
   real(r8), parameter :: zero = 0.0_r8
   real(r8), parameter :: missing_value = -9999._r8

end module

module rstrat_types
   use constants
   type, bind(C) :: reservoir_geometry

      real(r8) :: depth             ! Reservoir mean depth, taken from in GRanD database, (m)
      real(r8) :: d_ht              ! Dam height (m)
      real(r8) :: A_cf              ! Area correcting factor for error from geometry estimation
      real(r8) :: V_cf              ! Volume correcting factor for error from geometry estimation
      real(r8) :: M_L               ! Mean lake length (km)
      real(r8) :: M_W               ! Mean lake width (km)
      real(r8) :: V_err             ! Volume error (%) (+ve difference means underestimation)
      real(r8) :: Ar_err            ! Area error(&) (+ve difference means underestimation)
      real(r8) :: C_v               ! Volume coefficient (-)
      real(r8) :: C_a               ! Surface area coefficient (-)
      real(r8) :: V_df              ! Volume difference(mcm) (+ve difference means underestimation)
      real(r8) :: A_df              ! Area difference (km2) (+ve difference means underestimation)
      real(r8) :: d_res             ! Reservoir depth, taken as 0.95*(dam height) in GRanD database, (m)
      real(r8) :: dd_z(nlayer_max)  ! Layer depth(m)
      real(r8) :: ddz_min           ! Minimum layer thickness limit
      real(r8) :: ddz_max           ! Maximum layer thickness limit

      ! Number of vertical depth discretization with n_depth + 1 layer areas
      integer(C_INT) :: n_depth

      ! ------ Geometry code ------
      ! 1='Rhombus_wedge';    2='Oval_bowl';         3='Rectangular_prism'; 4='Rectangular_bowl';
      ! 5='Elliptical_bowl';  6='Rectangular_wedge'; 7='Oval_wedge';        8='Rhombus_bowl';
      ! 9='Triangular_wedge'; 10='Parabolic_bowl';   11='Parabolic_wedge';  12='Triangular_bowl'
      integer(C_INT) :: gm_j

   end type

   type :: res_dav
      real(r8), allocatable :: d_zi(:)    ! Initial depth for reservoir geometry
      real(r8), allocatable :: a_di(:)    ! Initial area for reservoir geometry
      real(r8), allocatable :: v_zti(:)   ! Initial volume for reservoir geometry
   end type

end module
module procedures
   use constants
   use rstrat_types
contains
   function den(t_z) result(rho)
      ! calculate density from temperature
      implicit none
      real(r8), intent(in) :: t_z! Temperature (k)
      real(r8) :: rho! ! density (kg/m3)

      rho = 1000._r8*(1.0_r8 - 1.9549e-05_r8*(abs(t_z - 277._r8))**1.68_r8) ! modified from Subin et al, 2011 with lake ice fraction = 0
   end function den

   function avg(data) result(mean)
      integer :: k
      real(r8) :: data(:)
      real(r8) :: mean, sum
      sum = zero
      mean = zero
      do k = 1, size(data)
         sum = sum + data(k)
      end do
      mean = sum/size(data)
   end function

   subroutine rgeom(resgeo)

      ! Calculate reservoir layer average area (km2)
      implicit none
      type(reservoir_geometry), intent(inout) :: resgeo
      integer(C_INT) :: n_depth_new

      !     Depth -> N Layers
      !        <3 -> 1
      !       3-5 -> 3
      !      5-10 -> 4
      !     10-50 -> 5
      !    50-100 -> 10
      !   100-150 -> 15
      !   150-200 -> 20
      !      >200 -> 25
      if (resgeo%depth < 3._r8) then
         n_depth_new = 1
      else if (resgeo%depth >= 3._r8 .and. resgeo%depth < 5._r8) then
         n_depth_new = 3
      else if (resgeo%depth >= 5._r8 .and. resgeo%depth < 10._r8) then
         n_depth_new = 4
      else if (resgeo%depth >= 10._r8 .and. resgeo%depth < 50._r8) then
         n_depth_new = 5
      else if (resgeo%depth >= 50._r8 .and. resgeo%depth < 100._r8) then
         n_depth_new = 10
      else if (resgeo%depth >= 100._r8 .and. resgeo%depth < 150._r8) then
         n_depth_new = 15
      else if (resgeo%depth >= 150._r8 .and. resgeo%depth < 200._r8) then
         n_depth_new = 20
      else
         n_depth_new = 25
      end if

      if (resgeo%n_depth .ne. n_depth_new) then
         if (DEBUG) then
            print *, "************************************************"
            print *, "  INPUT N DEPTH =/= COMPUTED N_DEPTH"
            print *, "             DEPTH= ", resgeo%depth
            print *, "        N_DEPTH IN=", resgeo%n_depth
            print *, "          COMPUTED=", n_depth_new
            print *, "              DIFF=", n_depth_new - resgeo%n_depth
            print *, "************************************************"
         end if
         resgeo%n_depth = n_depth_new
      end if

      if (resgeo%d_ht <= zero) resgeo%d_ht = resgeo%depth
      resgeo%d_res = 0.95*resgeo%d_ht

      ! Calculate layer depth
      if (resgeo%M_W <= zero) resgeo%M_W = 1.
      if (resgeo%M_L <= zero) resgeo%M_L = 1.

   end subroutine rgeom

   subroutine depth_area_vol(resgeo, dav)
      implicit none
      type(reservoir_geometry), intent(inout) :: resgeo
      type(res_dav), intent(inout) :: dav
      real(r8) :: a_dd(d_nn + 1), a_zi(d_nn + 1)
      real(r8) :: dd_in
      real(r8) :: ar_f = 1.0e6  ! Factor to convert area to m^2
      real(r8) :: pi = 2.0*asin(1.0_r8)
      real(r8) :: d_res ! Reservoir depth
      integer :: j, k

      ! resgeo member d_res (resgeo%d_res) is re-computed each subtimestep
      ! re-compute here as 0.95 * dam_height, since it throws off calculations
      ! if the depth-area-vol computation is re-done each time.
      d_res = 0.95*resgeo%d_ht

      if (allocated(dav%v_zti)) then
         deallocate (dav%v_zti)
      end if
      if (allocated(dav%a_di)) then
         deallocate (dav%a_di)
      end if
      if (allocated(dav%d_zi)) then
         deallocate (dav%d_zi)
      end if
      allocate (dav%v_zti(d_nn + 1))
      allocate (dav%a_di(d_nn + 1))
      allocate (dav%d_zi(d_nn + 1))
      dav%v_zti = zero
      dav%a_di = zero
      dav%d_zi = zero

      ! Area and volume correcting factors for relative error as compared to GRanD
      resgeo%A_cf = 1.+(resgeo%Ar_err/100.)
      resgeo%V_cf = 1.+(resgeo%V_err/100.)

      ! Uniform subsurface layer depth for initialization and limit maximum layer thickness
      dd_in = d_res/(d_nn*1.0_r8) !bottom layers evenly descritized

      ! Calculate depth area
      ! **************** Curved Lake Bottom ****************
      do j = 1, d_nn
         if (resgeo%gm_j == 1) then
            a_dd(j) = resgeo%C_a*resgeo%M_L*resgeo%M_W*(1 - ((dd_in*(j - 1))/d_res)**2.)
         else if (resgeo%gm_j == 3) then
            a_dd(j) = resgeo%C_a*resgeo%M_L*resgeo%M_W*(1 - ((dd_in*(j - 1))/d_res)**2.)* &
                      ((d_res - (dd_in*(j - 1)))/d_res)**0.5
         else if (resgeo%gm_j == 5) then
            a_dd(j) = resgeo%C_a*pi*0.25*resgeo%M_L*resgeo%M_W*(1 - ((dd_in*(j - 1))/d_res)**2.)* &
                      ((d_res - (dd_in*(j - 1)))/d_res)**0.5
         else if (resgeo%gm_j == 2) then
            a_dd(j) = resgeo%C_a*resgeo%M_L*resgeo%M_W*(1 - ((dd_in*(j - 1))/d_res)**2.)* &
                      (1 - ((dd_in*(j - 1))/d_res))
         else if (resgeo%gm_j == 4) then
            a_dd(j) = resgeo%C_a*(2./3.)*resgeo%M_L*resgeo%M_W*(1 - ((dd_in*(j - 1))/d_res)**2.)* &
                      (1 - ((dd_in*(j - 1)))/d_res)
         end if
         a_dd(d_nn + 1) = 0.1_r8  ! Bottom area given non-zero value
      end do
      ! ****************************************************

      ! Reverse indexing so that bottom is 1 and top is n_depth+1 and convert to m2
      do j = 1, d_nn + 1
         k = d_nn + 2 - j
         dav%a_di(k) = max(a_dd(j), 1.0_r8)*ar_f
      end do

      dav%a_di(1) = 0.1_r8 ! Bottom Area
      if (dav%a_di(d_nn + 1) < dav%a_di(d_nn)) dav%a_di(d_nn + 1) = dav%a_di(d_nn)
      ! Calculate layer depth from bottom,area,and volume
      dav%d_zi(1) = zero
      do j = 2, d_nn + 1
         dav%d_zi(j) = dav%d_zi(j - 1) + dd_in
      end do
      ! Calculate layer average area,and total volume from bottom
      dav%v_zti(1) = 0.1_r8
      do j = 2, d_nn + 1
         a_zi(j - 1) = max(0.5*(dav%a_di(j) + dav%a_di(j - 1)), 1.0_r8) ! Area converted to m^2
         dav%v_zti(j) = (dav%v_zti(j - 1) + resgeo%C_v*a_zi(j - 1)*dd_in)
         ! v_zti(j) = v_zti(j-1) + C_v*a_zi(j-1)*dd_in+V_df/d_nn
      end do
      a_zi(d_nn + 1) = dav%a_di(d_nn + 1)

      dav%a_di = resgeo%A_cf*dav%a_di      ! Area corrected for error
      dav%v_zti = resgeo%V_cf*dav%v_zti    ! Volume corrected for error
      resgeo%A_cf = 1._r8
      resgeo%V_cf = 1._r8

   end subroutine depth_area_vol

   subroutine layer_thickness(resgeo) bind(C, name="layer_thickness")
      ! Initialize layer thickness if not prescribed
      implicit none
      type(reservoir_geometry), intent(inout) :: resgeo
      integer j

      resgeo%dd_z(resgeo%n_depth) = sfc_lyr_dpth  ! top layer depth kept constant
      do j = resgeo%n_depth, 1, -1
         if (j == resgeo%n_depth .and. resgeo%n_depth == 1) then
            resgeo%dd_z(j) = resgeo%d_res
         else if ((resgeo%n_depth > 1 .and. j < resgeo%n_depth) .and. &
                  (resgeo%d_res - resgeo%dd_z(resgeo%n_depth)) > zero) then
            resgeo%dd_z(j) = (resgeo%d_res - resgeo%dd_z(resgeo%n_depth))/(resgeo%n_depth - 1) ! bottom layers evenly descritized
         end if
      end do

      if (resgeo%n_depth > 1) then
         if (resgeo%dd_z(resgeo%n_depth - 1) < sfc_lyr_dpth) then  ! layer thickness too small
            resgeo%n_depth = int(resgeo%d_res/sfc_lyr_dpth) + 1
            do j = 1, resgeo%n_depth
               resgeo%dd_z(j) = zero
            end do
            ! Reinitialize layer thickness
            do j = resgeo%n_depth, 1, -1
               if (j == resgeo%n_depth) then
                  resgeo%dd_z(j) = sfc_lyr_dpth   ! top layer depth = 0.6m
               else
                  resgeo%dd_z(j) = (resgeo%d_res - resgeo%dd_z(resgeo%n_depth))/(resgeo%n_depth - 1)  ! bottom layers evenly descritized
               end if
            end do
         end if
      end if
      ! Calculate maximum and minimum layer thickness
      resgeo%ddz_min = 1.5_r8
      if (resgeo%n_depth >= 15) then
         ! resgeo%ddz_min = 1.5_r8
         if (2.5*resgeo%dd_z(resgeo%n_depth - 1) > resgeo%ddz_min) then
            resgeo%ddz_max = 5.*resgeo%dd_z(resgeo%n_depth - 1)
         else
            resgeo%ddz_max = 2.*resgeo%ddz_min
         end if
      else if (resgeo%n_depth > 1 .and. resgeo%n_depth < 15) then
         ! resgeo%ddz_min = 1.5_r8
         if (2.5*resgeo%dd_z(resgeo%n_depth - 1) > resgeo%ddz_min) then
            resgeo%ddz_max = 2.5*resgeo%dd_z(resgeo%n_depth - 1)
         else
            resgeo%ddz_max = 2.*resgeo%ddz_min
         end if
      else if (resgeo%n_depth == 1) then
         ! resgeo%ddz_min = 1.5_r8
         resgeo%ddz_max = 2.*resgeo%d_res
      end if
      ! resgeo%ddz_min = max(resgeo%ddz_max*0.25, 2.0_r8)
      if (DEBUG) then
         print *, "DDZ_MAX=", resgeo%ddz_max, "DDZ_MIN=", resgeo%ddz_min
      end if
   end subroutine layer_thickness

   subroutine setup_solve(a, b, c, r, A_cf, V_cf, phi_o, sh_net, a_d, df_eff, t_z, phi_z, rho_z, d_v, dd_z)
      implicit none
      real(r8), intent(inout) :: a(:), b(:), c(:), r(:)
      real(r8), intent(in) :: A_cf, &
                              V_cf, &
                              phi_o, &
                              sh_net, &
                              a_d(:), &
                              d_v(:), &
                              df_eff(:), &
                              t_z(:), &
                              phi_z(:), &
                              rho_z(:), &
                              dd_z(:)
      integer :: n_depth
      integer :: j
      real(r8) :: m1(size(a)), m2(size(a)), m3(size(a)), Fx(size(a))
      n_depth = size(a)
      !*****************************************************************
      ! Calculate matrix elements
      a = zero
      b = zero
      c = zero
      r = zero
      do j = 1, n_depth
         if (j == 1 .and. n_depth > 1) then
            m1(j) = 2*dtime/(0.5_r8*A_cf*(a_d(j) + a_d(j + 1))*dd_z(j))
            m2(j) = m1(j)*A_cf*a_d(j + 1)*df_eff(j + 1)/(dd_z(j) + dd_z(j + 1))
            m3(j) = zero
            Fx(j) = dtime*phi_z(j)/(V_cf*d_v(j)*c_w*rho_z(j))
            a(j) = -(m2(j))
            b(j) = 1.+(m2(j) + m3(j))
            c(j) = zero
            r(j) = t_z(j) + Fx(j) !+ dt_in(j) - dt_ou(j)! bottom boundary condition
         else if (j <= n_depth - 1 .and. n_depth > 2) then
            m1(j) = 2*dtime/(0.5*A_cf*(a_d(j) + a_d(j + 1))*dd_z(j))
            m2(j) = m1(j)*A_cf*a_d(j + 1)*df_eff(j + 1)/(dd_z(j) + dd_z(j + 1))
            m3(j) = m1(j)*A_cf*a_d(j)*df_eff(j)/(dd_z(j) + dd_z(j - 1))
            Fx(j) = dtime*phi_z(j)/(V_cf*d_v(j)*c_w*rho_z(j))
            a(j) = -m2(j)
            b(j) = 1.+m2(j) + m3(j)
            c(j) = -m3(j)
            r(j) = t_z(j) + Fx(j)
         else if (j == n_depth) then!top layer
            m1(j) = 2*dtime/(0.5*A_cf*(a_d(j) + a_d(j + 1))*dd_z(j))
            m2(j) = zero
            if (n_depth > 1) then
               m3(j) = m1(j)*A_cf*a_d(j)*df_eff(j)/(dd_z(j) + dd_z(j - 1))
            else
               m3(j) = m1(j)*A_cf*a_d(j)*df_eff(j)/(dd_z(j))
            end if
            Fx(j) = dtime*((phi_o - sh_net)*A_cf*a_d(n_depth + 1) + phi_z(j)) &
                    /(V_cf*d_v(n_depth)*c_w*rho_z(j))
            a(j) = zero
            b(j) = 1.+(m2(j) + m3(j))
            c(j) = -(m3(j))
            r(j) = t_z(j) + Fx(j) ! top boundary condition
         end if
      end do
   end subroutine setup_solve

   subroutine solve(a, b, c, r, u) !MNLAKE

      implicit none
      ! a - left coefficient , b - middle coefficient, c - right coefficient
      ! r - right hand side (known), u - the answer (unknown), n - number of equations
      real(r8), intent(in) :: a(:), b(:), c(:), r(:)
      integer :: neqn
      real(r8), intent(out) :: u(:)
      real(r8) :: bp(size(u)), rp(size(u))
      real(r8) :: tt
      integer :: i
      neqn = size(a)

      !         initialize c-prime and d-prime
      bp(1) = b(1)
      rp(1) = r(1)
      do i = 2, neqn
         tt = a(i)/b(i - 1)
         bp(i) = b(i) - c(i - 1)*tt
         rp(i) = r(i) - rp(i - 1)*tt
      end do
      bp(neqn) = b(neqn)
      ! rp(n)=r(n)
      !         initialize u
      u(neqn) = rp(neqn)/bp(neqn)

      !         Back substitution
      do i = neqn - 1, 1, -1
         u(i) = (rp(i) - c(i)*u(i + 1))/bp(i)
      end do

   end subroutine solve
   subroutine surface_props(n_depth, coszen, d_res, s_w, lw_abs, t_air, rh, U_2, &
                            in_f, t_z, sh_net, phi_o, evap, eta)
      implicit none

      integer, intent(in) :: n_depth
      real(r8), intent(in) :: coszen, d_res, s_w, lw_abs, t_air, rh, U_2, in_f
      real(r8), intent(in) :: t_z(n_depth)
      real(r8), intent(inout) :: sh_net, phi_o, evap, eta
      real(r8) :: bias, &       ! bias correction for solar radiation
                  alb_s, &      ! surface albedo shortwave radiation
                  lw_abr, &     ! atmospheric longwave absorbtion for sub-timestep (W/m^2)
                  es, &         ! Saturated vapor pressure(hPa)
                  ea, &         ! Atmospheric vapor pressure(hPa)
                  lw_ems, &     ! longwave emission
                  sn_heat, &    ! sensible heat
                  lt_heat, &    ! latent heat
                  kl            ! empirical coefficient for the turbulent exchange of water vapor (mm/d/hpa)
      if (coszen > zero) then
         alb_s = 0.05/(0.15 + coszen)
      else
         alb_s = 0.06_r8
      end if

      eta = 1.1925*(d_res)**(-0.424) ! as used in Subin et al, 2011 (citing Hakanson, 1995) but modified for actual reservoir depth
      bias = 1.00_r8

      ! Calculation of Surface fluxes and heat source
      sh_net = max(bias*s_w*(1 - alb_s), zero)                    ! Net shortwave radiation (w/m^2)
      lw_abr = (1.-0.03)*lw_abs                                   ! longwave radiation (w/m^2)
      es = 4.596*exp(17.25*(t_z(n_depth) - 273.15)/t_z(n_depth))  ! in mmHg
      ea = 0.01*rh*4.596*exp(17.25*(t_air - 273.15)/t_air)        ! in mmHg
      lw_ems = 0.97*st_bl*t_z(n_depth)**4                         ! as used in henderson-sellers, 1984 (w/m^2)
      sn_heat = 1.5701*U_2*(t_z(n_depth) - t_air)                 ! sensible heat (w/m^2)

      ! Evaporation calculated as in Wu et al, 2012
      kl = 0.211 + 0.103*U_2*F
      if (use_evap) then
         evap = max(kl*133.322368*(es - ea)/100., zero)    ! in mm/d; ea and es converted from mmHg to hpa
      else
         evap = zero
      end if
      if (t_z(n_depth) > t_frz) then
         lt_heat = rho_w*evap*le/(86.4e6)               ! latent heat (w/m^2)
      else
         lt_heat = zero
      end if
      ! Avoid continuous water abstraction if there is no inflow.
      ! Assumption is precipitation accounts for evaporation
      if (abs(in_f) < 1e-20_r8) evap = zero
      ! net surface heat flux
      phi_o = sh_net + lw_abr - lt_heat - lw_ems - sn_heat
   end subroutine surface_props

   subroutine diffusion_coeff(n_depth, u_2, A_cf, V_cf, M_W, M_L, &
                              rho_z, a_d, v_zt, dv_in, dv_ou, dd_z, &
                              drhodz, df_eff, d_z)
      implicit none

      integer, intent(in) :: n_depth
      real(r8), intent(in) :: u_2, A_cf, V_cf, M_W, M_L
      real(r8), intent(in) :: rho_z(nlayer_max), a_d(nlayer_max), v_zt(nlayer_max)
      real(r8), intent(in) :: dv_in(nlayer_max), dv_ou(nlayer_max), dd_z(nlayer_max)
      real(r8), intent(inout) :: drhodz(nlayer_max)

      real(r8), intent(inout) :: df_eff(nlayer_max), d_z(nlayer_max)

      real(r8), allocatable :: Fr(:)      ! Froude number squared and inverted for diffusion coeff. calculation
      real(r8), allocatable :: dis_ad(:)  ! rate of dissipation-inflow/outflow
      real(r8), allocatable :: q_adv(:)   ! Layer flow rate (m^3/s)
      real(r8), allocatable :: l_vel(:)   !
      real(r8), allocatable :: bv_f(:)    ! Brunt-Visala frequency [s**-2]
      real(r8), allocatable :: ri(:)      ! Richardson number
      real(r8), allocatable :: k_ad(:)    ! Effective advective kinetic energy (kg.m^2/s^2)

      real(r8) :: c_d               ! Drag coefficient
      real(r8) :: cfw, cfa
      real(r8) :: tau               ! Shear stress at surface
      real(r8) :: s_vel             ! Shear velocity at surface
      real(r8) :: dis_w             ! Rate of dissipation due to wind
      real(r8) :: k_ew              ! Effective wind
      real(r8) :: k_m               ! Molecular diffusivity
      integer :: k

      allocate (Fr(n_depth))
      allocate (q_adv(n_depth))
      allocate (dis_ad(n_depth))
      allocate (l_vel(n_depth))
      allocate (bv_f(n_depth))
      allocate (ri(n_depth))
      allocate (k_ad(n_depth))

      Fr(:) = zero
      q_adv(:) = zero
      dis_ad(:) = zero
      l_vel(:) = zero
      bv_f(:) = zero
      ri(:) = zero
      k_ad(:) = zero
      ! *********************************************************************************************************************************
      !         Calculation of effective diffusion coefficient
      if (u_2 >= 15.) then
         c_d = 2.6e-3_r8
      else
         c_d = 5.e-4*sqrt(u_2)
      end if
      tau = rho_a*c_d*u_2**2.                           ! Shear stress at surface
      s_vel = sqrt(tau/rho_w)                           ! Shear velocity at surface
      k_ew = tau*s_vel*A_cf*a_d(n_depth + 1)*dtime      ! Wind driven kinetic energy at surface
      dis_w = k_ew/(rho_w*V_cf*v_zt(n_depth + 1)*dtime) ! rate of dissipation-wind
      cfw = 1.e-02_r8                                   !
      cfa = 1.e-05_r8                                   !
      k_m = 0.57/(c_w*rho_w)                            ! molecular diffusivity
      df_eff(1) = zero                                  ! bottom interface
      df_eff(n_depth + 1) = zero                        ! air interface
      q_adv(2:n_depth) = max((dv_in(2:n_depth) + dv_ou(2:n_depth)), zero)

      ! Advection driven kinetic energy
      k_ad(2:n_depth) = 0.5*rho_w*q_adv(2:n_depth)*dtime*(q_adv(2:n_depth)/(M_W*dd_z(2:n_depth)))**2.

      ! rate of dissipation-inflow/outflow
      dis_ad(2:n_depth) = k_ad(2:n_depth)/(rho_w*V_cf*v_zt(2:n_depth)*dtime)

      ! Calculate Richardson number
      drhodz(2:n_depth) = (rho_z(1:n_depth - 1) - rho_z(2:n_depth))/0.5*(dd_z(2:n_depth) + dd_z(1:n_depth - 1))
      bv_f(2:n_depth) = max((grav/rho_w)*drhodz(2:n_depth), zero)
      ri(2:n_depth) = bv_f(2:n_depth)/((s_vel/(0.4*d_z(2:n_depth)))**2.)
      if (s_vel <= zero) ri = zero

      ! Calculate Froude number
      l_vel(2:n_depth) = q_adv(2:n_depth)*M_L/(A_cf*a_d(2:n_depth)*dd_z(2:n_depth))
      Fr(2:n_depth) = (grav*dd_z(2:n_depth)*drhodz(2:n_depth)/rho_w)/l_vel(2:n_depth)**2.

      ! Calculate diffusion coefficients
      df_eff(2:n_depth) = min(max(dtime**2.*((cfw*dis_w/(1 + ri(2:n_depth))) &
                                             + (0.5*cfa*(dis_ad(2:n_depth) + dis_ad(1:n_depth - 1)) &
                                                /(1 + Fr(2:n_depth)))), k_m), 5.56e-03_r8)

      deallocate (Fr)
      deallocate (q_adv)
      deallocate (dis_ad)
      deallocate (l_vel)
      deallocate (bv_f)
      deallocate (ri)
      deallocate (k_ad)

   end subroutine diffusion_coeff

   subroutine layer_mass_energy(n_depth, V_cf, m_ev, v_evap, dm_in, dv_in, dv_ou, &
                                d_v, m_zn, dd_z, t_z, enr_0, d_zi, rho_z, &
                                d_z, a_d, a_di, v_zti, v_zt, s_t, s_tin, V_df, A_cf, &
                                sh_net, eta, ddz_min, ddz_max, phi_z, in_t, enr_1, &
                                d_res, ww, ti, num_fac, m_cal, lme_error)
      implicit none

      integer(C_INT), intent(inout) :: n_depth
      integer :: i, j, m, k, l, ii
      integer, intent(in) :: ww, ti
      real(r8), intent(in) :: V_cf, A_cf, m_ev, v_evap
      real(r8), intent(in) :: dm_in(nlayer_max), &
                              d_zi(d_nn + 1), &
                              a_di(d_nn + 1), &
                              v_zti(d_nn + 1), &
                              ddz_min, &
                              V_df, &
                              in_t, &
                              ddz_max
      integer, intent(out) :: lme_error
      real(r8), intent(inout) :: dv_ou(nlayer_max), &
                                 dv_in(nlayer_max), &
                                 d_v(nlayer_max), &
                                 m_zn(nlayer_max), &
                                 dd_z(nlayer_max), &
                                 t_z(nlayer_max), &
                                 enr_0(nlayer_max), &
                                 rho_z(nlayer_max), &
                                 d_z(nlayer_max), &
                                 a_d(nlayer_max), &
                                 v_zt(nlayer_max), &
                                 enr_1(nlayer_max), &
                                 phi_z(nlayer_max), &
                                 m_cal, &
                                 num_fac, &
                                 s_t, &
                                 s_tin, &
                                 sh_net, &
                                 d_res, &
                                 eta

      real(r8), parameter :: beta = 0.175_r8 ! shortwave absorbtion factor
      real(r8) :: tab, &
                  delta_a, &
                  delta_z, &
                  m_mod, &              ! Reservoir modeled mass (kg)
                  e_ab, &
                  dv_ouab, &
                  dv_inab, &
                  dd_zab, &
                  m_ab, &
                  d_vab, &
                  top_d, &              ! top layer depth
                  v_mix, &              ! Total volume of mixed layer(m3)
                  sh_mix, &             ! net short wave radiation in mixed layer
                  rho_r, &              ! Density of inflow water  (kg/m3)
                  enr_err1              ! Energy error (w) before stratification

      real(r8), allocatable :: m_zo(:), &    ! Reservoir beginning mass at depth z (kg)
                               fac_1(:), &   ! Factor for calculation of triadiagonal matrices elements
                               phi_x(:), &   ! radiation absorbed by mixed layer (W/m^2)
                               enr_in(:), &  ! Layer Energy from inflow
                               enr_ou(:), &  ! Layer energy from outflow
                               dm_ou(:), &   ! initial mass removed from depth z (kg)
                               dm_nt(:)      ! net mass added at depth z (kg)

      lme_error = 0
      num_fac = 1.e6_r8
      rho_r = den(in_t)
      allocate (m_zo(n_depth))

      if (ti == 1 .and. ww == 1) then
         m_zo = V_cf*d_v*rho_z
      else
         m_zo(1:n_depth) = m_zn(1:n_depth)
      end if
      allocate (dm_ou(n_depth))
      allocate (dm_nt(n_depth))
      dm_ou(1:n_depth) = dv_ou(1:n_depth)*rho_z(1:n_depth)*dtime
      dm_ou(n_depth + 1:) = zero

999   continue
      if (n_depth > 1) then
         dm_nt(1:n_depth) = dm_in(1:n_depth) - dm_ou(1:n_depth)
         dm_nt(n_depth - 1) = dm_nt(n_depth - 1) - m_ev
      else if (n_depth == 1) then
         dm_nt(n_depth) = dm_in(n_depth) - dm_ou(n_depth) - m_ev
      end if

      ! Calculate layer mass (kg) and energy (w)
      allocate (fac_1(n_depth))
      m_zn(1:n_depth) = m_zo(1:n_depth) + dm_nt(1:n_depth)
      fac_1(1:n_depth) = V_cf*d_v(1:n_depth)*rho_z(1:n_depth)*c_w/dtime
      enr_0(1:n_depth) = t_z(1:n_depth)*fac_1(1:n_depth)/num_fac
      deallocate (fac_1)
      m_zn(n_depth + 1:) = zero
      enr_0(n_depth + 1:) = zero
      if (ti == 1 .and. ww == 1) then
         m_cal = sum(m_zo) + sum(dm_nt)
         m_mod = sum(m_zo) + sum(dm_nt)
      else
         m_cal = m_cal + sum(dm_nt)
         m_mod = sum(m_zo) + sum(dm_nt)
      end if

      d_z(1) = zero
      a_d(1) = 0.1_r8
      v_zt(1) = 0.1_r8
      do i = 1, n_depth                ! check layers for available volume to satisfy net outflow
         if (-dm_nt(i) > m_zo(i)) then ! current layer collapses, hence remaining volume taken from next upper layer

            m_zn(i) = zero
            if (i < n_depth - 1 .and. n_depth > 1) then
               m_zn(i + 1) = m_zn(i + 1) - (-dm_nt(i) - m_zo(i))
            elseif (i == n_depth - 1 .and. n_depth > 2) then
               ! sub-layer collapses, hence remaining mass taken from next lower layer
               m_zn(i - 1) = m_zn(i - 1) - (-dm_nt(i) - m_zo(i))
               m = i - 1
               do k = m, n_depth
                  v_zt(k + 1) = v_zt(k) + m_zn(k)/rho_z(k)
                  do j = 2, d_nn + 1
                     if (v_zt(k + 1) > v_zti(j - 1) .and. v_zt(k + 1) <= v_zti(j)) then
                        delta_z = (d_zi(j) - d_zi(j - 1))*(v_zt(k + 1) - v_zti(j - 1))/(v_zti(j) - v_zti(j - 1))
                        delta_a = (a_di(j) - a_di(j - 1))*(v_zt(k + 1) - v_zti(j - 1))/(v_zti(j) - v_zti(j - 1))
                        d_z(k + 1) = d_zi(j - 1) + delta_z
                        a_d(k + 1) = a_di(j - 1) + delta_a
                     else if (v_zt(k + 1) > v_zti(d_nn + 1)) then
                        delta_z = (d_zi(d_nn + 1) - d_zi(d_nn))*(v_zt(k + 1) - v_zti(d_nn)) &
                                  /(v_zti(d_nn + 1) - v_zti(d_nn))
                        delta_a = (a_di(n_depth + 1) - a_di(n_depth))*(v_zt(k + 1) - v_zti(d_nn)) &
                                  /(v_zti(d_nn + 1) - v_zti(d_nn))
                        d_z(k + 1) = d_zi(d_nn) + delta_z
                        a_d(k + 1) = a_di(d_nn) + delta_a
                     end if
                  end do
                  d_z(k) = d_z(k + 1)
                  dd_z(k) = d_z(k + 1) - d_z(k)
               end do
            elseif ((i == n_depth - 1 .or. i == n_depth) .and. n_depth <= 2) then   ! Top layer collapses, skip outflow
               dm_ou(i) = zero
               go to 999
            end if

            v_zt(i + 1) = v_zt(i) + m_zn(i)/rho_z(i)
            do j = 2, d_nn + 1
               if (v_zt(i + 1) > v_zti(j - 1) .and. v_zt(i + 1) <= v_zti(j)) then
                  delta_z = (d_zi(j) - d_zi(j - 1))*(v_zt(i + 1) - v_zti(j - 1))/(v_zti(j) - v_zti(j - 1))
                  delta_a = (a_di(j) - a_di(j - 1))*(v_zt(i + 1) - v_zti(j - 1))/(v_zti(j) - v_zti(j - 1))
                  d_z(i + 1) = d_zi(j - 1) + delta_z
                  a_d(i + 1) = a_di(j - 1) + delta_a
               else if (v_zt(i + 1) > v_zti(d_nn + 1)) then
                  delta_z = (d_zi(d_nn + 1) - d_zi(d_nn))*(v_zt(i + 1) - v_zti(d_nn))/(v_zti(d_nn + 1) - v_zti(d_nn))
                  delta_a = (a_di(n_depth + 1) - a_di(n_depth))*(v_zt(i + 1) - v_zti(d_nn))/(v_zti(d_nn + 1) - v_zti(d_nn))
                  d_z(i + 1) = d_zi(d_nn) + delta_z
                  a_d(i + 1) = a_di(d_nn) + delta_a
               end if
            end do
            d_z(i) = d_z(i + 1)
            dd_z(i) = d_z(i + 1) - d_z(i)
         else ! enough volume, layers don't collapses

            v_zt(i + 1) = v_zt(i) + m_zn(i)/rho_z(i)
            do j = 2, d_nn + 1
               if (v_zt(i + 1) > v_zti(j - 1) .and. v_zt(i + 1) <= v_zti(j)) then
                  delta_z = (d_zi(j) - d_zi(j - 1))*(v_zt(i + 1) - v_zti(j - 1))/(v_zti(j) - v_zti(j - 1))
                  delta_a = (a_di(j) - a_di(j - 1))*(v_zt(i + 1) - v_zti(j - 1))/(v_zti(j) - v_zti(j - 1))
                  d_z(i + 1) = d_zi(j - 1) + delta_z
                  a_d(i + 1) = a_di(j - 1) + delta_a
               else if (v_zt(i + 1) > v_zti(d_nn + 1)) then
                  delta_z = (d_zi(d_nn + 1) - d_zi(d_nn))*(v_zt(i + 1) - v_zti(d_nn))/(v_zti(d_nn + 1) - v_zti(d_nn))
                  delta_a = (a_di(n_depth + 1) - a_di(n_depth))*(v_zt(i + 1) - v_zti(d_nn))/(v_zti(d_nn + 1) - v_zti(d_nn))
                  d_z(i + 1) = d_zi(d_nn) + delta_z
                  a_d(i + 1) = a_di(d_nn) + delta_a
               end if
            end do
            dd_z(i) = d_z(i + 1) - d_z(i)
         end if
      end do
      deallocate (dm_ou)
      deallocate (dm_nt)

      ! Recalculate layer thickness and volume, and reservoir depth
      d_res = zero
      do i = 1, n_depth
         d_v(i) = m_zn(i)/rho_z(i)
         d_res = d_res + dd_z(i)
      end do

      ! Check if layers are too small
      do while (minval(dd_z(1:n_depth - 1)) < ddz_min)
         layerloop: do i = 1, n_depth - 1
            ! Identify lower (small and to be merged) and upper (larger) layer
            do while (dd_z(i) < ddz_min)
               if (i < n_depth - 1) then
                  m = i + 1
               else
                  m = i - 1 ! Avoid merging to top layer
                  ! m = i
               end if
               if (m == 0) then
                  if (DEBUG) then
                     print *, "RESETTING M"
                  end if
                  m = 1
               end if
               ! Merge layers
               if (d_v(i) + d_v(m) > zero) then
                  t_z(i) = (d_v(i)*t_z(i) + d_v(m)*t_z(m))/(d_v(i) + d_v(m))
               end if

               ! Adjust new layer volume, thickness, mass and inflow/outflow
               dd_z(i) = dd_z(i) + dd_z(m)
               d_v(i) = d_v(i) + d_v(m)
               m_zn(i) = m_zn(i) + m_zn(m)
               dv_ou(i) = dv_ou(i) + dv_ou(m)
               dv_in(i) = dv_in(i) + dv_in(m)
               enr_0(i) = enr_0(i) + enr_0(m)

               ! Re-number layers before collapse
               do j = i, nlayer_max - 1
                  if (j == i .and. i < (n_depth - 1)) then
                     t_z(j) = t_z(i)
                     dv_ou(j) = dv_ou(i)
                     dv_in(j) = dv_in(i)
                     dd_z(j) = dd_z(i)
                     d_v(j) = d_v(i)
                     m_zn(j) = m_zn(i)
                     enr_0(j) = enr_0(i)
                     d_z(j + 1) = d_z(i + 2)
                     v_zt(j + 1) = v_zt(i + 2)
                     a_d(j + 1) = a_d(i + 2)
                  else if (j < n_depth .and. i < (n_depth - 1)) then
                     t_z(j) = t_z(j + 1)
                     dv_ou(j) = dv_ou(j + 1)
                     dv_in(j) = dv_in(j + 1)
                     dd_z(j) = dd_z(j + 1)
                     d_v(j) = d_v(j + 1)
                     m_zn(j) = m_zn(j + 1)
                     enr_0(j) = enr_0(j + 1)
                     d_z(j + 1) = d_z(j + 2)
                     v_zt(j + 1) = v_zt(j + 2)
                     a_d(j + 1) = a_d(j + 2)
                     ! else if (j == i .and. i == (n_depth - 1)) then
                  else if (j == (n_depth - 1) .and. n_depth - 1 > 1) then
                     t_z(j - 1) = t_z(i)
                     dv_ou(j - 1) = dv_ou(i)
                     dv_in(j - 1) = dv_in(i)
                     dd_z(j - 1) = dd_z(i)
                     d_v(j - 1) = d_v(i)
                     m_zn(j - 1) = m_zn(i)
                     enr_0(j - 1) = enr_0(i)
                     d_z(j) = d_z(j + 1)
                     v_zt(j) = v_zt(j + 1)
                     a_d(j) = a_d(j + 1)
                     t_z(n_depth - 1) = t_z(n_depth)
                     dv_ou(n_depth - 1) = dv_ou(n_depth)
                     dv_in(n_depth - 1) = dv_in(n_depth)
                     dd_z(n_depth - 1) = dd_z(n_depth)
                     d_v(n_depth - 1) = d_v(n_depth)
                     m_zn(n_depth - 1) = m_zn(n_depth)
                     enr_0(n_depth - 1) = enr_0(n_depth)
                     d_z(n_depth) = d_z(n_depth + 1)
                     v_zt(n_depth) = v_zt(n_depth + 1)
                     a_d(n_depth) = a_d(n_depth + 1)
                  else if (j == (n_depth - 1) .and. n_depth < 2) then
                     t_z(n_depth - 1) = t_z(n_depth)
                     dv_ou(n_depth - 1) = dv_ou(n_depth)
                     dv_in(n_depth - 1) = dv_in(n_depth)
                     dd_z(n_depth - 1) = dd_z(n_depth)
                     d_v(n_depth - 1) = d_v(n_depth)
                     m_zn(n_depth - 1) = m_zn(n_depth)
                     enr_0(n_depth - 1) = enr_0(n_depth)
                     d_z(n_depth) = d_z(n_depth + 1)
                     v_zt(n_depth) = v_zt(n_depth + 1)
                     a_d(n_depth) = a_d(n_depth + 1)
                  else if (j >= n_depth) then
                     t_z(j) = zero
                     dv_ou(j) = zero
                     dv_in(j) = zero
                     dd_z(j) = zero
                     d_v(j) = zero
                     m_zn(j) = zero
                     enr_0(j) = zero
                     d_z(j + 1) = zero
                     v_zt(j + 1) = zero
                     a_d(j + 1) = zero
                  end if
               end do
               n_depth = n_depth - 1

               if (n_depth < 1) then
                  lme_error = 1
                  if (DEBUG) then
                     print *, 'QUIT LAYER MASS/ENERGY SUBROUTINE NDEPTH < 1', n_depth
                  end if
                  return
               end if
               ! Quit the loop over layers so only one layer is modified per outer loop
               exit layerloop
            end do
         end do layerloop
      end do

      ! Check if layers are too big
      ! Calculate layer geometric properties to be halved
      do while (maxval(dd_z(1:n_depth)) >= ddz_max)
         do i = 1, n_depth
            do while (dd_z(i) >= ddz_max)
               dd_zab = 0.5*dd_z(i)
               m_ab = 0.5*m_zn(i)
               e_ab = 0.5*enr_0(i)
               d_vab = d_v(i)
               tab = t_z(i)
               dv_ouab = dv_ou(i)
               dv_inab = dv_in(i)

               ! Re-number layers before dividing layer
               d_z(n_depth + 2) = d_z(n_depth + 1)
               v_zt(n_depth + 2) = v_zt(n_depth + 1)
               a_d(n_depth + 2) = a_d(n_depth + 1)
               m = i + 1
               do j = m, n_depth
                  k = n_depth - j + i + 1
                  t_z(k + 1) = t_z(k)
                  dv_ou(k + 1) = dv_ou(k)
                  dv_in(k + 1) = dv_in(k)
                  d_z(k + 1) = d_z(k)
                  a_d(k + 1) = a_d(k)
                  v_zt(k + 1) = v_zt(k)
                  d_v(k + 1) = d_v(k)
                  dd_z(k + 1) = dd_z(k)
                  m_zn(k + 1) = m_zn(k)
                  enr_0(k + 1) = enr_0(k)
               end do

               ! Divide layer in half and calculate corresponding properties
               dd_z(i) = dd_zab
               dd_z(i + 1) = dd_zab
               d_z(i + 1) = d_z(i) + dd_z(i)
               do j = 2, d_nn + 1
               if (d_z(i + 1) > d_zi(j - 1) .and. d_z(i + 1) <= d_zi(j)) then
                  delta_z = (d_z(i + 1) - d_zi(j - 1))/(d_zi(j) - d_zi(j - 1))
                  a_d(i + 1) = delta_z*(a_di(j) - a_di(j - 1)) + a_di(j - 1)
                  v_zt(i + 1) = delta_z*(v_zti(j) - v_zti(j - 1)) + v_zti(j - 1)
               else if (d_z(i + 1) > d_zi(d_nn + 1)) then
                  delta_z = (d_z(i + 1) - d_zi(d_nn))/(d_zi(d_nn + 1) - d_zi(d_nn))
                  a_d(i + 1) = delta_z*(a_di(d_nn + 1) - a_di(d_nn)) + a_di(d_nn)
                  v_zt(i + 1) = delta_z*(v_zti(d_nn + 1) - v_zti(d_nn)) + v_zti(d_nn)
               end if
               end do
               d_v(i + 1) = d_vab - (v_zt(i + 1) - v_zt(i))
               d_v(i) = v_zt(i + 1) - v_zt(i)
               dv_ou(i + 1) = d_v(i + 1)*dv_ouab/(d_v(i) + d_v(i + 1))
               dv_ou(i) = d_v(i)*dv_ouab/(d_v(i) + d_v(i + 1))
               t_z(i) = tab
               t_z(i + 1) = tab
               m_zn(i) = m_ab
               m_zn(i + 1) = m_ab
               enr_0(i) = e_ab
               enr_0(i + 1) = e_ab
               dv_in(i + 1) = d_v(i + 1)*dv_inab/(d_v(i) + d_v(i + 1))
               dv_in(i) = d_v(i)*dv_inab/(d_v(i) + d_v(i + 1))
               n_depth = n_depth + 1
            end do
         end do
      end do

      ! Recalculate density after layer change
      do j = 1, nlayer_max
         if (j <= n_depth) then
            rho_z(j) = den(t_z(j))
         else
            rho_z(j) = zero
         end if
      end do

      s_t = v_zt(n_depth + 1)

      if ((s_t <= 0.0050*(s_tin + V_df*1e6) .or. d_res <= 1.0_r8) .or. &
          (n_depth == 1 .and. abs(t_z(n_depth)) >= 450.0_r8) .or. &
          (sum(m_zn) <= zero) .or. &
          (n_depth >= nlayer_max)) then
         lme_error = 1
         if (DEBUG) then
            print *, '------QUIT LAYER MASS/ENERGY SUBROUTINE-----'
            print *, '                      s_t', s_t
            print *, '                    s_tin', s_tin
            print *, '                     V_df', V_df
            print *, '                    d_res', d_res
            print *, '                  n_depth', n_depth
            print *, '                  t_z_sfc', t_z(n_depth)
            print *, '                sum(m_zn)', sum(m_zn)
            print *, '               nlayer_max', nlayer_max
            print *, '0.0050*(s_tin + V_df*1e6)', 0.0050*(s_tin + V_df*1e6)
            print *, '----------------------------------------------'
         end if
         return
      end if

      ! Calculate layer internal energy (w) due to inflow/outflow
      allocate (enr_in(n_depth))
      allocate (enr_ou(n_depth))
      do j = 1, n_depth
         enr_in(j) = dv_in(j)*in_t*rho_r*c_w/num_fac         ! Energy from inflow
         enr_ou(j) = dv_ou(j)*t_z(j)*rho_z(j)*c_w/num_fac    ! Energy loss due to outflow
      end do

      enr_1(1:n_depth) = enr_0(1:n_depth) + enr_in(1:n_depth) - enr_ou(1:n_depth)

      ! Adjust layer temperature for mixing due to inflow/outflow
      t_z(1:n_depth) = enr_1(1:n_depth)*num_fac*dtime/(m_zn(1:n_depth)*c_w)
      t_z(n_depth + 1:) = zero

      ! Check energy balance (w) after advective mixing
      enr_err1 = (sum(enr_1) - (sum(enr_0) + sum(enr_in) - sum(enr_ou)))*num_fac
      deallocate (enr_in)
      deallocate (enr_ou)

      !**********************************************
      ! Calculate solar energy absorbed at each layer
      allocate (phi_x(n_depth + 1))
      phi_x(:) = zero
      phi_z(:) = zero

      if (sh_net > zero) then
         k = 0
         top_d = d_res - d_z(n_depth - k)
         do while (top_d < 0.61_r8)
            k = k + 1
            top_d = d_res - d_z(n_depth - k)
         end do
         v_mix = V_cf*(v_zt(n_depth + 1) - v_zt(n_depth - k))

         ! Solar radiation energy absorbed at the mixed zone
         sh_mix = sh_net*A_cf*(a_d(n_depth + 1) - (1.-beta)*a_d(n_depth - k))
         j = n_depth - k
         do i = j, n_depth
            phi_z(i) = sh_mix*V_cf*d_v(i)/v_mix
         end do
         phi_x(n_depth + 1) = sh_net
         phi_x(n_depth - k) = (1.-beta)*sh_net
         if (k > 0) then
            do i = 1, k
               ii = n_depth - i + 1
               phi_x(ii) = (A_cf*a_d(ii + 1)*phi_x(ii + 1) - phi_z(ii))/(A_cf*a_d(ii))
            end do
         end if

         ! Solar radiation energy absorbed at sub layers
         l = n_depth - k - 1
         do j = 1, l
            i = (n_depth - k) - j + 1
            phi_x(i - 1) = phi_x(i)*exp(-eta*(d_z(i) - d_z(i - 1)))
         end do

         ! Solar radiation energy absorbed in each layer
         j = n_depth - k - 1
         do i = 1, j
            phi_z(i) = A_cf*(a_d(i + 1)*phi_x(i + 1) - a_d(i)*phi_x(i))
         end do
      else
         do j = 1, n_depth
            phi_z(j) = zero
         end do
      end if
      deallocate (phi_x)
   end subroutine

   subroutine convective_mix(n_depth, rho_z, t_z, d_v, m_zn, enr_1, V_cf, num_fac)
      implicit none

      integer, intent(in) :: n_depth
      real(r8), intent(inout) :: rho_z(nlayer_max), &
                                 t_z(nlayer_max), &
                                 d_v(nlayer_max), &
                                 ! V_cf, &
                                 num_fac
      real(r8), intent(in) :: enr_1(nlayer_max), &
                              m_zn(nlayer_max), &
                              V_cf

      integer :: j, k, mixlow, mixtop
      real(r8) :: enr_bc, &
                  enr_ac, &
                  enr_err2c, &! Energy error (w) before stratification, after triadiagonal solution, and after convective mixing
                  enr_2(nlayer_max), &
                  enr_2c(nlayer_max), &
                  sumvol, &
                  summas, &
                  sumenr, &
                  vlmxtp, &
                  msmxtp, &
                  enmxtp, &
                  denmix, &
                  vlmxlw, &
                  msmxlw, &
                  enmxlw, &
                  tsum, &
                  tmix

      !***************************
      ! Solve convective mixing
      ! Recalculate layer density
      do j = 1, n_depth
         rho_z(j) = den(t_z(j))
      end do
      enr_bc = zero
      enr_ac = zero

      ! Check if instability exists
      k = 1
501   continue
      do while (k < n_depth)
         ! if (k >= n_depth) go to 560
         if (rho_z(k) < rho_z(k + 1)) go to 510
         k = k + 1
         go to 501
510      continue

         ! Start mixing layers
         mixlow = k
         mixtop = mixlow
         sumvol = V_cf*d_v(mixtop)
         summas = m_zn(mixtop)
         sumenr = enr_2(mixtop)
         tsum = t_z(mixtop)*m_zn(mixtop)
520      continue
         mixtop = mixtop + 1
         vlmxtp = V_cf*d_v(mixtop)
         msmxtp = m_zn(mixtop)
         enmxtp = enr_2(mixtop)
         sumvol = sumvol + vlmxtp
         summas = summas + msmxtp
         sumenr = sumenr + enmxtp
         tsum = tsum + t_z(mixtop)*msmxtp
         tmix = tsum/summas

         ! Calculate density of mixed layer
         denmix = den(tmix)
         if (mixtop == n_depth) go to 540
         if (denmix < rho_z(mixtop + 1)) go to 520
540      continue
         if (mixlow <= 1) go to 550

         ! Check if instability exists below mixed layer and mix layers
         if (rho_z(mixlow - 1) >= denmix) go to 550
         mixlow = mixlow - 1

         ! Calculate temperature of mixed layer
         vlmxlw = V_cf*d_v(mixlow)
         msmxlw = m_zn(mixlow)
         enmxlw = enr_2(mixlow)
         sumvol = sumvol + vlmxlw
         summas = summas + msmxlw
         sumenr = sumenr + enmxlw
         tsum = tsum + t_z(mixlow)*msmxlw
         tmix = tsum/summas

         ! Calculate density of mixed layer
         denmix = den(tmix)
         go to 540

550      continue

         ! Set new layer temperature and density
         do j = mixlow, mixtop
            rho_z(j) = denmix
            t_z(j) = tmix
         end do

         ! Calculate sum of layer energy in the mixing layer before mixing
         do j = mixlow, mixtop
            enr_ac = enr_ac + t_z(j)*m_zn(j)*c_w/(dtime*num_fac)
         end do
         k = mixtop
      end do

      ! Recalculate layer final internal energy (w) after convective mixing
      do j = 1, nlayer_max
         if (j <= n_depth) then
            enr_2c(j) = t_z(j)*m_zn(j)*c_w/(dtime*num_fac)
         else
            enr_2c(j) = zero
         end if
      end do

      ! Check energy balance (w) after convective mixing
      enr_err2c = sum(enr_2c) - sum(enr_1)
      enr_err2c = enr_err2c*num_fac!/a_d(n_depth+1)

   end subroutine convective_mix

   subroutine convective_mix_nogoto(n_depth, rho_z, t_z, d_v, m_zn, enr_1, V_cf, num_fac)
      implicit none

      integer, intent(in) :: n_depth
      real(r8), intent(inout) :: rho_z(nlayer_max), &
                                 t_z(nlayer_max), &
                                 d_v(nlayer_max), &
                                 m_zn(nlayer_max), &
                                 num_fac
      real(r8), intent(in) :: enr_1(nlayer_max), &
                              V_cf

      integer :: j, k, mixlow, mixtop, first_pass
      real(r8) :: enr_bc, &
                  enr_ac, &
                  enr_err2c, &
                  sumvol, &
                  summas, &
                  sumenr, &
                  vlmxtp, &
                  msmxtp, &
                  enmxtp, &
                  denmix, &
                  vlmxlw, &
                  msmxlw, &
                  enmxlw, &
                  tsum, &
                  tmix, &
                  enr_2(nlayer_max), &
                  enr_2c(nlayer_max)

      !***************************
      ! Solve convective mixing
      ! Recalculate layer density
      do j = 1, n_depth
         rho_z(j) = den(t_z(j))
      end do
      ! Initialise local variables
      enr_bc = zero
      enr_ac = zero
      sumvol = zero
      summas = zero
      sumenr = zero
      vlmxtp = zero
      msmxtp = zero
      enmxtp = zero
      denmix = zero
      vlmxlw = zero
      msmxlw = zero
      enmxlw = zero

      ! Check if instability exists
      k = 1
      do while (k < n_depth)
         do while (rho_z(k) > rho_z(k + 1) .and. k < n_depth)
            k = k + 1
         end do

         ! Start mixing layers
         mixlow = k
         mixtop = mixlow
         sumvol = V_cf*d_v(mixtop)
         summas = m_zn(mixtop)
         sumenr = enr_2(mixtop)
         tsum = t_z(mixtop)*m_zn(mixtop)
         first_pass = 0

         ! Initialize denmix so that it's less than rho_z(mixtop + 1)
         denmix = rho_z(mixtop + 1) - 1.0_r8

         do while ((denmix < rho_z(mixtop + 1) .and. mixtop < n_depth) .or. first_pass == 0)
            ! do while (denmix < rho_z(mixtop + 1) .and. mixtop < n_depth)
            mixtop = mixtop + 1
            vlmxtp = V_cf*d_v(mixtop)
            msmxtp = m_zn(mixtop)
            enmxtp = enr_2(mixtop)
            sumvol = sumvol + vlmxtp
            summas = summas + msmxtp
            sumenr = sumenr + enmxtp
            tsum = tsum + t_z(mixtop)*msmxtp
            tmix = tsum/summas

            ! Calculate density of mixed layer
            denmix = den(tmix)
            first_pass = first_pass + 1
         end do

         ! Check if instability exists below mixed layer...and mix layers
         if (mixlow >= 2) then
            below: do while (mixlow > 1 .and. rho_z(mixlow - 1) < denmix)
               mixlow = mixlow - 1

               ! Calculate temperature of mixed layer
               vlmxlw = V_cf*d_v(mixlow)
               msmxlw = m_zn(mixlow)
               enmxlw = enr_2(mixlow)
               sumvol = sumvol + vlmxlw
               summas = summas + msmxlw
               sumenr = sumenr + enmxlw
               tsum = tsum + t_z(mixlow)*msmxlw
               tmix = tsum/summas

               ! Calculate density of mixed layer
               denmix = den(tmix)
               if (mixlow <= 2) then
                  exit below
               end if
            end do below
         end if

         ! Set new layer temperature and density
         do j = mixlow, mixtop
            rho_z(j) = denmix
            t_z(j) = tmix
         end do

         ! Calculate sum of layer energy in the mixing layer before mixing
         do j = mixlow, mixtop
            enr_ac = enr_ac + t_z(j)*m_zn(j)*c_w/(dtime*num_fac)
         end do
         k = mixtop
      end do

      !   Recalculate layer final internal energy (w) after convective mixing
      enr_2c(:) = zero
      ! do j = 1, nlayer_max
      do j = 1, n_depth
         enr_2c(j) = t_z(j)*m_zn(j)*c_w/(dtime*num_fac)
      end do

      ! Check energy balance (w) after convective mixing
      enr_err2c = sum(enr_2c) - sum(enr_1)
      enr_err2c = enr_err2c*num_fac!/a_d(n_depth+1)

   end subroutine convective_mix_nogoto

   subroutine flow_contrib(s_t, s_tin, V_df, d_res, d_ht, n_depth, ou_f, in_f, v_evap, m_ev)
      implicit none
      real(r8), intent(in) :: s_t, s_tin, V_df, d_res, d_ht, in_f
      integer, intent(in) :: n_depth
      real(r8), intent(inout) :: ou_f, v_evap, m_ev
      ! Calculation of flow contibution due to inflow/outflow
      if ((s_t <= 0.10*(s_tin + V_df*1e6) .and. ou_f > in_f) &
          .or. ((d_res < 5. .and. n_depth <= 3) .and. ou_f > in_f)) then
         v_evap = zero
         m_ev = zero
      end if
      ! if ((s_t <= 0.10*(s_tin + V_df*1e6) .and. ou_f > in_f) .or. &
      !     ! Avoid extra inflow of water if the reservoir storage exceeded total dam storage (taken to be initial storage)
      !     (s_t >= (s_tin + V_df*1e6) .and. in_f > ou_f) .or. &
      !     ! Avoid reservoir level from exceeding dam height
      !     (d_res >= d_ht .and. in_f > ou_f) .or. &
      !     ! Avoid extraction of water from shallow reservoirs beyond 5m depth for numerical stability until
      !     ! reservoir operation is calibrated
      !     ((d_res < 5. .and. n_depth <= 3) .and. ou_f > in_f)) then
      !    ou_f = in_f
      ! end if

      if (s_t <= 0.10*(s_tin + V_df*1e6) .and. ou_f > in_f) then
         if (DEBUG) then
            ! Avoid extraction of water from reservoir beyond 20% volume of the total storage
            print *, "FC-A,", (s_t - 0.10*(s_tin + V_df*1e6))/s_t
         end if
         ou_f = in_f
      else if (s_t >= (s_tin + V_df*1e6) .and. in_f > ou_f) then
         if (DEBUG) then
            ! Avoid extra inflow of water if the reservoir storage exceeded total dam storage
            print *, "FC-B,", (s_t - (s_tin + V_df*1e6))/s_t
         end if
         ou_f = in_f
      else if (d_res >= d_ht .and. in_f > ou_f) then
         if (DEBUG) then
            ! Avoid reservoir level from exceeding dam height
            print *, "FC-C,", (d_res - d_ht)/d_res
         end if
         ou_f = in_f
      else if ((d_res < 5. .and. n_depth <= 3) .and. ou_f > in_f) then
         if (DEBUG) then
            ! Avoid extraction of water from shallow reservoirs beyond 5m depth for numerical stability until
            ! reservoir operation is calibrated
            print *, "FC-D", d_res/(n_depth + 1e-16_r8)
         end if
         ou_f = in_f
      end if

   end subroutine flow_contrib

   subroutine flowdist(n_depth, in_f, in_t, ou_f, d_v, v_zt, dv_in, dv_ou, dm_in)
!*******************************************************************************************************
!         Calculation inflow/outflow contribution adopted from CE-QUAL-R1 model
!*******************************************************************************************************
      implicit none
      integer, intent(in)  :: n_depth
      real(r8), intent(in)  :: in_f, in_t, ou_f, d_v(nlayer_max), v_zt(nlayer_max)
      real(r8), dimension(nlayer_max), intent(out) :: dv_in, dv_ou, dm_in   ! layer inflow/outflow (m3/s)
      real(r8) :: rho_r, in_v, m_in    !
      real(r8), dimension(nlayer_max) :: th_en ! Layer thermal energy (j/s)
      integer :: j, jmax, jmin                                                        ! indices

      rho_r = den(in_t)
      in_v = in_f*dtime
      m_in = in_v*rho_r
      ! Initialize
      do j = 1, nlayer_max
         dv_in(j) = zero
         dv_ou(j) = zero
         dm_in(j) = zero
         th_en(j) = zero
      end do

      ! Layer inflow and energy contribution
      jmin = 1
      if (n_depth > 3) jmax = n_depth - 3
      if (n_depth <= 3) jmax = n_depth - 1

      if (n_depth == 1) then ! Single layer
         do j = 1, nlayer_max
            dv_in(j) = zero
            dm_in(j) = zero
            th_en(j) = zero
            dv_ou(j) = zero
         end do
         dv_in(n_depth) = in_f
         dm_in(n_depth) = m_in
         th_en(n_depth) = dv_in(n_depth)*rho_r*in_t*c_w
         dv_ou(n_depth) = ou_f
      else
         do j = jmin, jmax
            dv_in(j) = in_f*(d_v(j)/(v_zt(jmax + 1) - v_zt(jmin)))
            dm_in(j) = m_in*(d_v(j)/(v_zt(jmax + 1) - v_zt(jmin)))
            th_en(j) = dv_in(j)*rho_r*in_t*c_w
            dv_ou(j) = ou_f*(d_v(j)/(v_zt(jmax + 1) - v_zt(jmin)))
         end do
      end if

   end subroutine flowdist

end module

module timestepping
   use constants
   use procedures
   use rstrat_types
   implicit none
contains

   subroutine finalise_subtimestep(n_depth, &
                                   t_z_old, t_z, cntr, cntr1, cntr2, phi_z, &
                                   dd_z, d_zsb, t_zsub, d_res_sub, d_res)

      implicit none
      ! Calculate count for sub-timestep averaging
      integer, intent(inout) :: n_depth
      real(r8), intent(inout) :: t_z_old(nlayer_max), &
                                 t_z(nlayer_max), &
                                 cntr(nlayer_max), &
                                 cntr1(nlayer_max), &
                                 cntr2(nlayer_max), &
                                 phi_z(nlayer_max), &
                                 dd_z(nlayer_max), &
                                 d_zsb(nlayer_max), &
                                 t_zsub(nlayer_max), &
                                 d_res_sub

      real(r8), intent(inout) :: d_res
      real(r8) ::d_zs(nlayer_max)   ! Depth at z from surface (m)
      integer :: j
      do j = 1, nlayer_max
         if (abs(t_z_old(j)) > 1e-20_r8 .and. abs(t_z(j)) > 1e-20_r8) then
            cntr1(j) = cntr1(j) + 1.0_r8
            cntr2(j) = zero
         else
            cntr1(j) = zero
            cntr2(j) = cntr2(j) + 1.0_r8
         end if
         cntr(j) = cntr1(j) + cntr2(j)
      end do

      d_res_sub = d_res_sub + d_res
      phi_z(n_depth + 1:) = zero
      ! Calculate layer depth for profile plot(minimum at the top)
      if (n_depth >= 2) then
         d_zs(1) = d_res - 0.5*dd_z(2)
         do j = 2, nlayer_max
            if (j <= n_depth) then
               d_zs(j) = d_zs(j - 1) - 0.5*dd_z(j - 1) - 0.5*dd_z(j)
            else
               d_zs(j) = zero
            end if
         end do
         d_zs(n_depth) = zero
      else
         d_zs(1) = d_res
      end if

      ! Sum sub-timestep variables
      do j = 1, nlayer_max
         t_zsub(j) = t_zsub(j) + t_z(j)
         d_zsb(j) = d_zsb(j) + d_zs(j)
      end do

   end subroutine finalise_subtimestep

   subroutine init_subtimestep(m_zn, t_air, resgeo, dav, d_z, a_d, v_zt, d_v, &
                               rho_z, t_z, s_tin)

      implicit none
      type(reservoir_geometry), intent(inout) :: resgeo
      type(res_dav), intent(in) :: dav
      real(r8), intent(in) :: t_air
      real(r8), intent(inout) :: s_tin

      real(r8), intent(inout) :: m_zn(nlayer_max), &
                                 d_z(nlayer_max), &
                                 v_zt(nlayer_max), &
                                 d_v(nlayer_max), &
                                 rho_z(nlayer_max), &
                                 t_z(nlayer_max), &
                                 a_d(nlayer_max)
      real(r8) :: delta_z
      integer i, j
      m_zn = zero       ! Reservoir ending mass at depth z (kg)

      ! Calculate layer depth (minimum at bottom)
      d_z(1) = zero
      do j = 2, nlayer_max
         if (j <= resgeo%n_depth + 1) then
            d_z(j) = d_z(j - 1) + resgeo%dd_z(j - 1)
         else
            d_z(j) = zero
         end if
      end do

      ! Assign layer area and and volume based on depth-area-volume relationship
      a_d = zero
      v_zt = zero
      d_v = zero

      a_d(1) = 0.1_r8
      v_zt(1) = 0.1_r8
      do i = 2, resgeo%n_depth + 1
         j = 2
         do while ((d_z(i) <= dav%d_zi(i - 1) .or. d_z(i) > dav%d_zi(j)) .and. j < d_nn + 1)
            j = j + 1
         end do
         delta_z = (d_z(i) - dav%d_zi(j - 1))/(dav%d_zi(j) - dav%d_zi(j - 1))
         a_d(i) = delta_z*(dav%a_di(j) - dav%a_di(j - 1)) + dav%a_di(j - 1)
         v_zt(i) = delta_z*(dav%v_zti(j) - dav%v_zti(j - 1)) + dav%v_zti(j - 1)
      end do

      t_z(:) = t_air

      !        Calculate layer volume(m^3)
      do j = 1, resgeo%n_depth
         d_v(j) = v_zt(j + 1) - v_zt(j)
         resgeo%dd_z(j) = d_z(j + 1) - d_z(j)
         rho_z(j) = den(t_z(j))
      end do

      m_zn = resgeo%V_cf*d_v*rho_z

      s_tin = v_zt(resgeo%n_depth + 1)

   end subroutine init_subtimestep

   subroutine subtimestep(ww, ti, n_depth, &
                          coszen, lw_abs, s_w, rh, t_air, u_2, &
                          t_z, v_zt, d_res, in_f, &
                          rho_z, A_cf, a_d, s_tin, V_df, d_ht, ou_f, in_t, d_v, &
                          V_cf, m_zn, dd_z, enr_0, d_zi, d_z, a_di, v_zti, &
                          ddz_min, ddz_max, m_cal, lme_error, M_W, M_L, &
                          d_zsb, cntr, t_zsub, d_res_sub, cntr1, cntr2)

      use, intrinsic :: IEEE_ARITHMETIC, only: ieee_is_nan
      integer, intent(inout) :: n_depth
      integer, intent(inout) :: lme_error
      ! Environmental forcings: coszen, lw_abs, s_w, rh, t_air, u_2
      real(r8), intent(in) :: coszen, & ! cosine of solar zenith angle
                              lw_abs, & ! atmospheric longwave absorbtion (W/m^2)
                              s_w, &    ! solar radiation (W/m**2)
                              rh, &     ! Relative humidity (%)
                              t_air, &  ! air temperature
                              u_2, &    ! wind speed at 2m(m/s)
                              ddz_min, &
                              ddz_max, &
                              A_cf, &
                              d_zi(d_nn + 1), &
                              a_di(d_nn + 1), &
                              v_zti(d_nn + 1), &
                              V_cf

      real(r8), intent(inout) :: d_res, &
                                 s_tin, &
                                 V_df, &
                                 ou_f, &
                                 m_cal, &
                                 d_res_sub, &
                                 t_z(nlayer_max), &
                                 v_zt(nlayer_max), &
                                 d_v(nlayer_max), &
                                 m_zn(nlayer_max), &
                                 a_d(nlayer_max), &
                                 rho_z(nlayer_max), &
                                 dd_z(nlayer_max), &
                                 enr_0(nlayer_max), &
                                 d_z(nlayer_max), &
                                 d_zsb(nlayer_max), &
                                 cntr(nlayer_max), &
                                 t_zsub(nlayer_max), &
                                 cntr1(nlayer_max), &
                                 cntr2(nlayer_max)
      real(r8), intent(in) :: d_ht, &
                              M_W, &
                              M_L, &
                              in_t, &
                              in_f

      integer, intent(in) :: ww, ti
      real(r8), dimension(:), allocatable :: t_z_old, &
                                             dv_ou, &   ! volume decrease at layer due to inflow(m3)
                                             dv_in, &      ! volume increment at layer due to inflow(m^3)
                                             dm_in, &      ! mass added to depth z (kg)
                                             phi_z, &      ! radiation absorbed by layer (W/m^2)
                                             enr_1, &      ! Inner energy after advection
                                             a, &          ! "a" left  diagonal of tridiagonal matrix
                                             b, &          ! "b" diagonal column for tridiagonal matrix
                                             c, &          ! "c" right  diagonal tridiagonal matrix
                                             r!, &          ! "c" right  diagonal tridiagonal matrix

      real(r8) :: df_eff(nlayer_max), &     ! Effective diffusivity (molecular + eddy) [m2/s]
                  drhodz(nlayer_max)!, &     ! d [rhow] /dz (kg/m**4)
      real(r8) :: m_ev, &       ! initial evaporation mass (kg)
                  v_evap, &     ! Evaporated volume (m^3)
                  d_evap, &     ! Evaporated depth (m)
                  phi_o, &      ! net surface radiation (W/m^2)
                  num_fac, &    !
                  s_t, &        ! Total storage at timestep t (m^3)
                  sh_net, &     ! net short wave radiation
                  eta, &        ! light extinction coefficient
                  evap          ! evaporation rate (mm/d)

      integer :: j

      s_t = v_zt(n_depth + 1)

      ! Allocate layer temperature as old for assigning counter for averaging sub-timestep result
      allocate (t_z_old(nlayer_max))
      t_z_old(1:n_depth) = t_z(1:n_depth)

      ! Calculation of Surface properties like: albedo, light extinction coefficient, etc
      call surface_props(n_depth, coszen, d_res, s_w, lw_abs, t_air, rh, U_2, &
                         in_f, t_z, sh_net, phi_o, evap, eta)

      ! Calculation of reservoir density at depth z and incoming flow
      do j = 1, n_depth
         rho_z(j) = den(t_z(j))
      end do
      rho_z(n_depth + 1:) = zero

      ! Calculation of equivalent evaporated depth (m), and volume(m^3)
      d_evap = evap*dtime/(86.4e6)
      v_evap = max(d_evap*A_cf*a_d(n_depth + 1), zero)
      if (n_depth > 1) then
         m_ev = max(v_evap*rho_z(n_depth - 1), zero)
      else
         m_ev = zero
      end if

      ! Calculation of flow contibution due to inflow/outflow
      call flow_contrib(s_t, s_tin, V_df, d_res, d_ht, n_depth, ou_f, in_f, v_evap, m_ev)

      ! Distribute flow across layers
      allocate (dv_ou(nlayer_max))
      allocate (dv_in(nlayer_max))
      allocate (dm_in(nlayer_max))
      call flowdist(n_depth, in_f, in_t, ou_f, d_v, v_zt, dv_in, dv_ou, dm_in)

      ! Resize layer thickness and numbers based on inflow/outflow contribution
      ! Calculate initial layer and total mass (kg)
      allocate (phi_z(nlayer_max))
      allocate (enr_1(nlayer_max))
      call layer_mass_energy(n_depth, V_cf, m_ev, v_evap, dm_in, dv_in, dv_ou, &
                             d_v, m_zn, dd_z, t_z, enr_0, d_zi, rho_z, &
                             d_z, a_d, a_di, v_zti, v_zt, s_t, s_tin, V_df, A_cf, &
                             sh_net, eta, ddz_min, ddz_max, phi_z, in_t, enr_1, &
                             d_res, ww, ti, num_fac, m_cal, lme_error)
      deallocate (dm_in)
      if (lme_error == 1) then
         return
      end if

      ! Calculation of effective diffusion coefficient
      call diffusion_coeff(n_depth, u_2, A_cf, V_cf, M_W, M_L, &
                           rho_z, a_d, v_zt, dv_in, dv_ou, dd_z, &
                           drhodz, df_eff, d_z)
      deallocate (dv_ou)
      deallocate (dv_in)

      ! Setup tri-diagonal matrix arrays to pass to `solve` subroutine
      allocate (a(nlayer_max))
      allocate (b(nlayer_max))
      allocate (c(nlayer_max))
      allocate (r(nlayer_max))
      call setup_solve(a(:n_depth), b(:n_depth), c(:n_depth), r(:n_depth), &
                       A_cf, V_cf, phi_o, sh_net, &
                       a_d(:n_depth + 1), df_eff(:n_depth + 1), t_z(:n_depth), &
                       phi_z(:n_depth), rho_z(:n_depth), d_v(:n_depth), dd_z(:n_depth + 1))
      ! Solve for temperature
      call solve(a(:n_depth), b(:n_depth), c(:n_depth), r(:n_depth), t_z(:n_depth))
      deallocate (a)
      deallocate (b)
      deallocate (c)
      deallocate (r)

      ! Avoid Numerical instability for multiple reservoir runs
      do j = 1, n_depth
         if (ieee_is_nan(t_z(j))) then
            write (*, *) 'check reservoir data'
            return
         end if
      end do
      call convective_mix_nogoto(n_depth, rho_z, t_z, d_v, m_zn, enr_1, V_cf, num_fac)
      ! call convective_mix(n_depth, rho_z, t_z, d_v, m_zn, enr_1, V_cf, num_fac)

      deallocate (enr_1)

      call finalise_subtimestep(n_depth, &
                                t_z_old, t_z, cntr, cntr1, cntr2, phi_z, &
                                dd_z, d_zsb, t_zsub, d_res_sub, d_res)
      deallocate (phi_z)
      deallocate (t_z_old)

   end subroutine

end module
