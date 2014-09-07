/* This is file terminfo.hh.

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
enum boolean_caps {
  BOOL_CAP_bw, BOOL_CAP_am, BOOL_CAP_xsb, BOOL_CAP_xhp, BOOL_CAP_xenl,
  BOOL_CAP_eo, BOOL_CAP_gn, BOOL_CAP_hc, BOOL_CAP_km, BOOL_CAP_hs,
  BOOL_CAP_in, BOOL_CAP_da, BOOL_CAP_db, BOOL_CAP_mir, BOOL_CAP_msgr,
  BOOL_CAP_os, BOOL_CAP_eslok, BOOL_CAP_xt, BOOL_CAP_hz, BOOL_CAP_ul,
  BOOL_CAP_xon, BOOL_CAP_nxon, BOOL_CAP_mc5i, BOOL_CAP_chts,
  BOOL_CAP_nrrmc, BOOL_CAP_npc, STR_CAP_ndscr, BOOL_CAP_ccc,
  BOOL_CAP_bce, BOOL_CAP_hls, BOOL_CAP_xhpa, BOOL_CAP_crxm,
  BOOL_CAP_daisy, STR_CAP_xvpa, BOOL_CAP_sam, BOOL_CAP_cpix,
  BOOL_CAP_lpix
};

enum numerical_caps {
  NUM_CAP_cols, NUM_CAP_it, NUM_CAP_lines, NUM_CAP_lm, NUM_CAP_xmc,
  NUM_CAP_pb, NUM_CAP_vt, NUM_CAP_wsl, NUM_CAP_nlab, NUM_CAP_lh,
  NUM_CAP_lw, NUM_CAP_ma, NUM_CAP_wnum, NUM_CAP_colors, NUM_CAP_pairs,
  NUM_CAP_ncv
};

enum string_caps {
  STR_CAP_cbt, STR_CAP_bel, STR_CAP_cr, STR_CAP_csr, STR_CAP_tbc,
  STR_CAP_clear, STR_CAP_el, STR_CAP_ed, STR_CAP_hpa, STR_CAP_cmdch,
  STR_CAP_cup, STR_CAP_cud1, STR_CAP_home, STR_CAP_civis,
  STR_CAP_cub1, STR_CAP_mrcup, STR_CAP_cnorm, STR_CAP_cuf1,
  STR_CAP_ll, STR_CAP_cuu1, STR_CAP_cvvis, STR_CAP_dch1, STR_CAP_dl1,
  STR_CAP_dsl, STR_CAP_hd, STR_CAP_smacs, STR_CAP_blink, STR_CAP_bold,
  STR_CAP_smcup, STR_CAP_smdc, STR_CAP_dim, STR_CAP_smir,
  STR_CAP_invis, STR_CAP_prot, STR_CAP_rev, STR_CAP_smso,
  STR_CAP_smul, STR_CAP_ech, STR_CAP_rmacs, STR_CAP_sgr0,
  STR_CAP_rmcup, STR_CAP_rmdc, STR_CAP_rmir, STR_CAP_rmso,
  STR_CAP_rmul, STR_CAP_flash, STR_CAP_ff, STR_CAP_fsl, STR_CAP_is1,
  STR_CAP_is2, STR_CAP_is3, STR_CAP_if, STR_CAP_ich1, STR_CAP_il1,
  STR_CAP_ip, STR_CAP_kbs, STR_CAP_ktbc, STR_CAP_kclr, STR_CAP_kctab,
  STR_CAP_kdch1, STR_CAP_kdl1, STR_CAP_kcud1, STR_CAP_krmir,
  STR_CAP_kel, STR_CAP_ked, STR_CAP_kf0, STR_CAP_kf1, STR_CAP_kf10,
  STR_CAP_kf2, STR_CAP_kf3, STR_CAP_kf4, STR_CAP_kf5, STR_CAP_kf6,
  STR_CAP_kf7, STR_CAP_kf8, STR_CAP_kf9, STR_CAP_khome, STR_CAP_kich1,
  STR_CAP_kil1, STR_CAP_kcub1, STR_CAP_kll, STR_CAP_knp, STR_CAP_kpp,
  STR_CAP_kcuf1, STR_CAP_kind, STR_CAP_kri, STR_CAP_khts,
  STR_CAP_kcuu1, STR_CAP_rmkx, STR_CAP_smkx, STR_CAP_lf0, STR_CAP_lf1,
  STR_CAP_lf10, STR_CAP_lf2, STR_CAP_lf3, STR_CAP_lf4, STR_CAP_lf5,
  STR_CAP_lf6, STR_CAP_lf7, STR_CAP_lf8, STR_CAP_lf9, STR_CAP_rmm,
  STR_CAP_smm, STR_CAP_nel, STR_CAP_pad, STR_CAP_dch, STR_CAP_dl,
  STR_CAP_cud, STR_CAP_ich, STR_CAP_indn, STR_CAP_il, STR_CAP_cub,
  STR_CAP_cuf, STR_CAP_rin, STR_CAP_cuu, STR_CAP_pfkey, STR_CAP_pfloc,
  STR_CAP_pfx, STR_CAP_mc0, STR_CAP_mc4, STR_CAP_mc5, STR_CAP_rep,
  STR_CAP_rs1, STR_CAP_rs2, STR_CAP_rs3, STR_CAP_rf, STR_CAP_rc,
  STR_CAP_vpa, STR_CAP_sc, STR_CAP_ind, STR_CAP_ri, STR_CAP_sgr,
  STR_CAP_hts, STR_CAP_wind, STR_CAP_ht, STR_CAP_tsl, STR_CAP_uc,
  STR_CAP_hu, STR_CAP_iprog, STR_CAP_ka1, STR_CAP_ka3, STR_CAP_kb2,
  STR_CAP_kc1, STR_CAP_kc3, STR_CAP_mc5p, STR_CAP_rmp, STR_CAP_acsc,
  STR_CAP_pln, STR_CAP_kcbt, STR_CAP_smxon, STR_CAP_rmxon,
  STR_CAP_smam, STR_CAP_rmam, STR_CAP_xonc, STR_CAP_xoffc,
  STR_CAP_enacs, STR_CAP_smln, STR_CAP_rmln, STR_CAP_kbeg,
  STR_CAP_kcan, STR_CAP_kclo, STR_CAP_kcmd, STR_CAP_kcpy,
  STR_CAP_kcrt, STR_CAP_kend, STR_CAP_kent, STR_CAP_kext,
  STR_CAP_kfnd, STR_CAP_khlp, STR_CAP_kmrk, STR_CAP_kmsg,
  STR_CAP_kmov, STR_CAP_knxt, STR_CAP_kopn, STR_CAP_kopt,
  STR_CAP_kprv, STR_CAP_kprt, STR_CAP_krdo, STR_CAP_kref,
  STR_CAP_krfr, STR_CAP_krpl, STR_CAP_krst, STR_CAP_kres,
  STR_CAP_ksav, STR_CAP_kspd, STR_CAP_kund, STR_CAP_kBEG,
  STR_CAP_kCAN, STR_CAP_kCMD, STR_CAP_kCPY, STR_CAP_kCRT, STR_CAP_kDC,
  STR_CAP_kDL, STR_CAP_kslt, STR_CAP_kEND, STR_CAP_kEOL, STR_CAP_kEXT,
  STR_CAP_kFND, STR_CAP_kHLP, STR_CAP_kHOM, STR_CAP_kIC, STR_CAP_kLFT,
  STR_CAP_kMSG, STR_CAP_kMOV, STR_CAP_kNXT, STR_CAP_kOPT,
  STR_CAP_kPRV, STR_CAP_kPRT, STR_CAP_kRDO, STR_CAP_kRPL,
  STR_CAP_kRIT, STR_CAP_kRES, STR_CAP_kSAV, STR_CAP_kSPD,
  STR_CAP_kUND, STR_CAP_rfi, STR_CAP_kf11, STR_CAP_kf12, STR_CAP_kf13,
  STR_CAP_kf14, STR_CAP_kf15, STR_CAP_kf16, STR_CAP_kf17,
  STR_CAP_kf18, STR_CAP_kf19, STR_CAP_kf20, STR_CAP_kf21,
  STR_CAP_kf22, STR_CAP_kf23, STR_CAP_kf24, STR_CAP_kf25,
  STR_CAP_kf26, STR_CAP_kf27, STR_CAP_kf28, STR_CAP_kf29,
  STR_CAP_kf30, STR_CAP_kf31, STR_CAP_kf32, STR_CAP_kf33,
  STR_CAP_kf34, STR_CAP_kf35, STR_CAP_kf36, STR_CAP_kf37,
  STR_CAP_kf38, STR_CAP_kf39, STR_CAP_kf40, STR_CAP_kf41,
  STR_CAP_kf42, STR_CAP_kf43, STR_CAP_kf44, STR_CAP_kf45,
  STR_CAP_kf46, STR_CAP_kf47, STR_CAP_kf48, STR_CAP_kf49,
  STR_CAP_kf50, STR_CAP_kf51, STR_CAP_kf52, STR_CAP_kf53,
  STR_CAP_kf54, STR_CAP_kf55, STR_CAP_kf56, STR_CAP_kf57,
  STR_CAP_kf58, STR_CAP_kf59, STR_CAP_kf60, STR_CAP_kf61,
  STR_CAP_kf62, STR_CAP_kf63, STR_CAP_el1, STR_CAP_mgc, STR_CAP_smgl,
  STR_CAP_smgr, STR_CAP_fln, STR_CAP_sclk, STR_CAP_dclk,
  STR_CAP_rmclk, STR_CAP_cwin, STR_CAP_wingo, STR_CAP_hup,
  STR_CAP_dial, STR_CAP_qdial, STR_CAP_tone, STR_CAP_pulse,
  STR_CAP_wait, STR_CAP_unknown1, STR_CAP_unknown2, STR_CAP_u0,
  STR_CAP_u1, STR_CAP_u2, STR_CAP_u3, STR_CAP_u4, STR_CAP_u5,
  STR_CAP_u6, STR_CAP_u7, STR_CAP_u8, STR_CAP_u9, STR_CAP_op,
  STR_CAP_oc, STR_CAP_initc, STR_CAP_initp, STR_CAP_scp, STR_CAP_setf,
  STR_CAP_setb, STR_CAP_cpi, STR_CAP_lpi, STR_CAP_chr, STR_CAP_cvr,
  STR_CAP_defc, STR_CAP_swidm, STR_CAP_sdrfq, STR_CAP_sitm,
  STR_CAP_slm, STR_CAP_smicm, STR_CAP_snlq, STR_CAP_snrmq,
  STR_CAP_sshm, STR_CAP_ssubm, STR_CAP_ssupm, STR_CAP_sum,
  STR_CAP_rwidm, STR_CAP_ritm, STR_CAP_rlm, STR_CAP_rmicm,
  STR_CAP_rshm, STR_CAP_rsubm, STR_CAP_rsupm, STR_CAP_rum,
  STR_CAP_mhpa, STR_CAP_mcud1, STR_CAP_mcub1, STR_CAP_mcuf1,
  STR_CAP_mvpa, STR_CAP_mcuu1, STR_CAP_porder, STR_CAP_mcud,
  STR_CAP_mcub, STR_CAP_mcuf, STR_CAP_mcuu, STR_CAP_scs, STR_CAP_smgb,
  STR_CAP_smgbp, STR_CAP_smglp, STR_CAP_smgrp, STR_CAP_smgt,
  STR_CAP_smgtp, STR_CAP_sbim, STR_CAP_scsd, STR_CAP_rbim,
  STR_CAP_rcsd, STR_CAP_subcs, STR_CAP_supcs, STR_CAP_docr,
  STR_CAP_zerom, STR_CAP_unknown3, STR_CAP_kmous
};
