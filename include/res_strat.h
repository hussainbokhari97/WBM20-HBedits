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
    double M_L;              //  Mean lake length (km)
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

struct reservoir_geometry read_geom(float lat, float lon)
{
    struct reservoir_geometry resgeo;
    char fname[100];
    FILE *fptr;
    int gm_j;
    float depth;
    float d_ht;
    float M_L;
    float M_W;
    float V_err;
    float Ar_err;
    float C_v;
    float C_a;
    float V_df;
    float A_df;
    float forc[10 * 365 * 24][6];
    int n_depth;
    int i = 0;
    int j = 0;
    fptr = fopen("data/inputs/geometry/28.4375_-98.1875.txt", "r");
    if (NULL == fptr)
    {
        printf("File Cannot be opened\n");
    }
    else
    {
        printf("OPENED RES GEOMETRY FILE\n");
    }

    fscanf(fptr, "%d %f %f %f %f %f %f %f %f %f %f %d",
           &gm_j, &depth, &d_ht, &M_L, &M_W,
           &V_err, &Ar_err, &C_v, &C_a, &V_df,
           &A_df, &n_depth);
    fclose(fptr);

    resgeo.gm_j = gm_j;
    resgeo.depth = depth;
    resgeo.d_ht = d_ht;
    resgeo.M_L = M_L;
    resgeo.M_W = M_W;
    resgeo.V_err = V_err;
    resgeo.Ar_err = Ar_err;
    resgeo.C_v = C_v;
    resgeo.C_a = C_a;
    resgeo.V_df = V_df;
    resgeo.A_df = A_df;
    resgeo.n_depth = n_depth;

    return resgeo;
}

// Define subroutines from Fotran
void rgeom(struct reservoir_geometry *rgeom);
void layer_thickness(struct reservoir_geometry *rgeom);
void stratify(int ti, int *lme_error, double *in_t, double *in_f, double *ou_f,
              double *coszen, double *lw_abs, double *s_w, double *rh, double *t_air,
              double *u_2, struct reservoir_geometry *resgeom,
              double *d_z[], double *t_z[], double *m_zn[],
              double *a_d[], double *d_v[], double *v_zt[], double *s_tin, double *m_cal);