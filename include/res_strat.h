#include <stdio.h>
#include <stdlib.h>
#define MAX_DEPTH 250
#define NLAYER_MAX 30

struct reservoir_geometry
{
    double depth;            // Reservoir mean depth, taken from in GRanD database, (m)
    double d_ht;             // Dam height (m)
    double A_cf;             // Area correcting factor for error from geometry estimation
    double V_cf;             // Volume correcting factor for error from geometry estimation
    double M_L;              // Mean lake length (km)
    double M_W;              // Mean lake width (km)
    double V_err;            // Volume error (.) (+ve difference means underestimation)
    double Ar_err;           // Area error(&) (+ve difference means underestimation)
    double C_v;              // Volume coefficient (-)
    double C_a;              // Surface area coefficient (-)
    double V_df;             // Volume difference(mcm) (+ve difference means underestimation)
    double A_df;             // Area difference (km2) (+ve difference means underestimation)
    double d_res;            // Reservoir depth, taken as 0.95*(dam height) in GRanD database, (m)
    double dd_z[NLAYER_MAX]; // Layer depth(m)
    double ddz_min;          // Minimum layer thickness limit
    double ddz_max;          // Maximum layer thickness limit

    int n_depth;
    int gm_j;
};

// Define subroutines from Fotran
void rgeom(struct reservoir_geometry *rgeom);
void layer_thickness(struct reservoir_geometry *rgeom);
void stratify(int ti, int *lme_error, double *in_t, double *in_f, double *ou_f,
              double *coszen, double *lw_abs, double *s_w, double *rh, double *t_air,
              double *u_2, struct reservoir_geometry *resgeom,
              double *d_z[], double *t_z[], double *m_zn[],
              double *a_d[], double *d_v[], double *v_zt[], double *s_tin, double *m_cal);