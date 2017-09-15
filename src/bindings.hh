/* This is file bindings.hh.

Copyright 2013-2014 Louis Strous

This file is part of LUX.

LUX is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your
option) any later version.

LUX is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with LUX.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef HAVE_BINDINGS_H
#define HAVE_BINDINGS_H

/** \file

    A number of binding functions that bind a particular C function
    template to LUX and allow corresponding C functions to be quickly
    mapped to corresponding LUX function/subroutines.

    Each declaration must be on a single line, so that bindings.pl can
    easily detect it.
*/

int32_t lux_d_dT3_iaiqiqrq_012_f_(int32_t narg, int32_t ps[], double (*f)(double, double, double));
int32_t lux_d_dT4_iaiqT3rq_0T3_4_f_(int32_t narg, int32_t ps[], double (*f)(double, double, double, double));
int32_t lux_d_dT4_iaiqrq_01_2_f_(int32_t narg, int32_t ps[], double (*f)(double, double, double, double));
int32_t lux_d_dT4dp33_iaiqip3p3qrq_00112_3_f_(int32_t narg, int32_t ps[], double (*f)(double, double, double, double, double [3][3]));
int32_t lux_d_dT6_iaiT4rq_0z1T4_5_f_(int32_t narg, int32_t ps[], double (*f)(double, double, double, double, double, double));
int32_t lux_d_d_iarq_0_1_f_(int32_t narg, int32_t ps[], double (*f)(double));
int32_t lux_d_dd_iDaDrDq_01_2_f_(int32_t narg, int32_t ps[], double (*f)(double, double));
int32_t lux_d_dd_iDarDq_01_2_f_(int32_t narg, int32_t ps[], double (*f)(double, double));
int32_t lux_d_dd_iaibrq_01_2_f_(int32_t narg, int32_t ps[], double (*f)(double, double));
int32_t lux_d_dd_iaiqrq_01_2_f_(int32_t narg, int32_t ps[], double (*f)(double, double));
int32_t lux_d_dd_iarq_0z_1_f_(int32_t narg, int32_t ps[], double (*f)(double, double));
int32_t lux_d_dd_iarq_0z_1_f_(int32_t narg, int32_t ps[], double (*f)(double, double));
int32_t lux_d_dp33d_i33aimmqrcq_01_2_f_(int32_t narg, int32_t ps[], double (*f)(double [3][3], double));
int32_t lux_d_dp3_i3arm3q_0_1_f_(int32_t narg, int32_t ps[], double (*f)(double [3]));
int32_t lux_d_dp3dp3_i3aiqrm3q_01_2_f_(int32_t narg, int32_t ps[], double (*f)(double [3], double [3]));
int32_t lux_d_sd_iaiarxq_000_2_f_(int32_t narg, int32_t ps[], double (*f)(double *, size_t count, size_t stride));
int32_t lux_i_dT10dpT6_iaiqT7oqT6_0T6z7z8T13_s_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, double, double, double, double, double, double, double, double, double *, double *, double *, double *, double *, double *));
int32_t lux_i_dT3dpdp_iaiqrq_0z122_f_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, double, double *, double *));
int32_t lux_i_dT5dp3_i3aiirq_120003_f_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, double, double, double, double [3]));
int32_t lux_i_dT6dp23_iaiqT5op2p3q_0T6_f_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, double, double, double, double, double [2][3]));
int32_t lux_i_dddp23T2_iaop2p3qocq_0z12_s_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, double [2][3], double [2][3]));
int32_t lux_i_dddpdp_iarq_0z11_f_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, double *, double *));
int32_t lux_i_ddidp23_iairp3p2q_0z12_f_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, int32_t, double [2][3]));
int32_t lux_i_ddipT3dp_iarp3q_0z1111_f_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, int32_t *, int32_t *, int32_t *, double *));
int32_t lux_i_dp23dpT6_i23aom2m3qomqT5_0T6_s_(int32_t narg, int32_t ps[], int32_t (*f)(double [2][3], double *, double *, double *, double *, double *, double *));
int32_t lux_i_dpiT3dp_iaiirq_00T3_f_(int32_t narg, int32_t ps[], int32_t (*f)(double *, int32_t, int32_t, int32_t, double *));
int32_t lux_i_dpiidp_iairq_00T2_f_(int32_t narg, int32_t ps[], int32_t (*f)(double *, int32_t, int32_t, double *));
int32_t lux_i_idT3dp3_i3airq_10002_f_(int32_t narg, int32_t ps[], int32_t (*f)(int32_t, double, double, double, double [3]));
int32_t lux_i_idp3dpT3_ii3arcq_0T222_f_(int32_t narg, int32_t ps[], int32_t (*f)(int32_t, double [3], double *, double *, double *));
int32_t lux_i_idpdp_iarp2q_011_f_(int32_t narg, int32_t ps[], int32_t (*f)(int32_t, double *, double *));
int32_t lux_i_sd_iaia_000_s_(int32_t narg, int32_t ps[], int32_t (*f)(double *, size_t, size_t));
int32_t lux_i_sd_iaiarq_000_f_(int32_t narg, int32_t ps[], int32_t (*f)(double *, size_t, size_t));
int32_t lux_i_sdddsd_iaiiirq_000T333_f_(int32_t narg, int32_t ps[], int32_t (*f)(double *, size_t, size_t, double, double, double *, size_t, size_t));
int32_t lux_id0ddd_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double *, double *, double *));
int32_t lux_v_dT3dp33_iaiqiqrp3p3q_0T3_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double [3][3]));
int32_t lux_v_dT3dp33_iaiqiqrp3p3q_0T3_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double [3][3]));
int32_t lux_v_dT3dp3_iaiqiqrp3q_0T3_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double [3]));
int32_t lux_v_dT4dp33_iaiqT3rp3p3q_0T4_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double [3][3]));
int32_t lux_v_dT4dp33_iaiqiqrp3p3q_0z1T3_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double [3][3]));
int32_t lux_v_dT4dpT3_iaiqoqT3_0z1z2T4_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double *, double *, double *));
int32_t lux_v_dT4dpT4_iaiqiqoqT4_0T2z3T6_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double *, double *, double *, double *));
int32_t lux_v_dT4dpdp3T5_iaiqiqoqop3p3qocqT4_0z1T8_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double *, double [3][3], double [3][3], double [3][3], double [3][3], double [3][3]));
int32_t lux_v_dT4dpdp_iaiqT3oqoq_0T5_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double *, double *));
int32_t lux_v_dT4dpdp_iaiqiqoqoq_0T2z34_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double *, double *));
int32_t lux_v_dT6dp23_iaiqT5op2p3q_0T6_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double, double, double [2][3]));
int32_t lux_v_dT6dp33_iaiqT3rp3p3q_0z11T4_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double, double, double [3][3]));
int32_t lux_v_dT6dpT6_iaiqT5oqT6_0T11_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double, double, double *, double *, double *, double *, double *, double *));
int32_t lux_v_dT8dp33_iaiqT4rp3p3q_0zzz1T5_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double, double, double, double, double [3][3]));
int32_t lux_v_dddp33T2_iaip3p3qrcq_0z12_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double [3][3], double [3][3]));
int32_t lux_v_dddp33T3_iao33op3p3qocq_0z1T3_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double [3][3], double [3][3], double [3][3]));
int32_t lux_v_dddp33_iarp3p3q_0z1_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double [3][3]));
int32_t lux_v_dddp3_iaiqrp3q_0T2_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double [3]));
int32_t lux_v_dddp3dpT3_i3aiirq_120333_f_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, double [3], double *, double *, double *));
int32_t lux_v_dddpT16_iaoqT16_0z1T16_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *));
int32_t lux_v_dddpT3_iaoqT3_0z1T3_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double *, double *, double *));
int32_t lux_v_dddpT3dp33T5_iaoqT3op3p3qocqT4_0z1T8_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double *, double *, double *, double [3][3], double [3][3], double [3][3], double [3][3], double [3][3]));
int32_t lux_v_dddpT4_iaoqT4_0z1T4_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double *, double *, double *, double *));
int32_t lux_v_dddpdp_iaoqoq_0z12_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double *, double *));
int32_t lux_v_ddp33_iao33q_01_s_(int32_t narg, int32_t ps[], void (*f)(double, double [3][3]));
int32_t lux_v_ddpdp_iaoqoq_012_s_(int32_t narg, int32_t ps[], void (*f)(double, double*, double*));
int32_t lux_v_dp23T2dp2_i23aiqrkmq_0T2_f_(int32_t narg, int32_t ps[], void (*f)(double [2][3], double [2][3], double [2]));
int32_t lux_v_dp23T3_i23aiqrq_0T2_f_(int32_t narg, int32_t ps[], void (*f)(double [2][3], double [2][3], double [2][3]));
int32_t lux_v_dp23dpT6_iD23aommqocqT5_0T6_s_(int32_t narg, int32_t ps[], void (*f)(double [2][3], double *, double *, double *, double *, double *, double *));
int32_t lux_v_dp23dpdp_i23aommqocq_0T2_s_(int32_t narg, int32_t ps[], void (*f)(double [2][3], double *, double *));
int32_t lux_v_dp33T2_i33arq_01_f_(int32_t narg, int32_t ps[], void (*f)(double [3][3], double [3][3]));
int32_t lux_v_dp33T3_i33aiqrq_0T2_f_(int32_t narg, int32_t ps[], void (*f)(double [3][3], double [3][3], double [3][3]));
int32_t lux_v_dp33_r33_0_f_(int32_t narg, int32_t ps[], void (*f)(double [3][3]));
int32_t lux_v_dp33ddp33T2_i33aimmqiqrq_0T3_f_(int32_t narg, int32_t ps[], void (*f)(double [3][3], double, double [3][3], double [3][3]));
int32_t lux_v_dp33dp23T2_i33aim3p2qrcq_0T2_f_(int32_t narg, int32_t ps[], void (*f)(double [3][3], double [2][3], double [2][3]));
int32_t lux_v_dp33dp3T2_i33aimqrcq_0T2_f_(int32_t narg, int32_t ps[], void (*f)(double [3][3], double [3], double [3]));
int32_t lux_v_dp33dp3_i33armq_01_f_(int32_t narg, int32_t ps[], void (*f)(double [3][3], double [3]));
int32_t lux_v_dp33dpdp_i33aommqocq_0T2_s_(int32_t narg, int32_t ps[], void (*f)(double [3][3], double *, double *));
int32_t lux_v_dp3T3_i3aiqrq_0T2_f_(int32_t narg, int32_t ps[], void (*f)(double [3], double [3], double [3]));
int32_t lux_v_dp3dp33_i3arp3q_01_f_(int32_t narg, int32_t ps[], void (*f)(double [3], double [3][3]));
int32_t lux_v_dp3dpT3_i3arq_0111_f_(int32_t narg, int32_t ps[], void (*f)(double [3], double *, double *, double *));
int32_t lux_v_dp3dp_o33o3_01_s_(int32_t narg, int32_t ps[], void (*f)(double [3][3], double [3]));
int32_t lux_v_dp3dpdp3_i3aomqoq_0T2_s_(int32_t narg, int32_t ps[], void (*f)(double [3], double *, double [3]));
int32_t lux_v_dp3dpdp_i3arm3p2q_011_f_(int32_t narg, int32_t ps[], void (*f)(double [3], double *, double *));
int32_t lux_v_dpT3_r3_000_f_(int32_t narg, int32_t ps[], void (*f)(double *, double *, double *));
int32_t lux_v_sddsd_iairq_012_f_(int32_t narg, int32_t ps[], void (*f)(double *, size_t, size_t, double, double *, size_t, size_t));

void register_lux_f(int32_t (*f)(int32_t, int32_t []), char const* name, int32_t min_arg,
                   int32_t max_arg, char const* spec);
void register_lux_s(int32_t (*f)(int32_t, int32_t []), char const* name, int32_t min_arg,
                   int32_t max_arg, char const* spec);
/*
#define REGISTER(func, fs, name, minarg, maxarg, fsspec)        \
void register_lux_ ## func ## _ ## fs (void) __attribute__((constructor)); \
void register_lux_ ## func ## _ ## fs (void) { \
  register_lux_ ## fs( lux_ ## func, #name, minarg, maxarg, fsspec );    \
}

#define BIND(func, type, fs, name, minarg, maxarg, fsspec)      \
int32_t lux_ ## func ## _ ## fs(int32_t narg, int32_t ps[]) { \
 int32_t result = lux_ ## type ## _ ## fs ## _(narg, ps, func); \
 if (result < 0) luxerror("Error in " #name, 0); \
 return result; \
} \
REGISTER(func ## _ ## fs, fs, name, minarg, maxarg, fsspec)
*/
#define REGISTER(func, fs, name, minarg, maxarg, fsspec)
#define BIND(func, type, fs, name, minarg, maxarg, fsspec)
#endif
