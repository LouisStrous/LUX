/* This is file constellations.h.

Copyright 2013 Louis Strous

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
typedef enum {
  And, Ant, Aps, Aqr, Aql, Ara, Ari, Aur, Boo, Cae, Cam, Cnc, CVn, CMa,
  CMi, Cap, Car, Cas, Cen, Cep, Cet, Cha, Cir, Col, Com, CrA, CrB, Crv,
  Crt, Cru, Cyg, Del, Dor, Dra, Equ, Eri, For, Gem, Gru, Her, Hor, Hya,
  Hyi, Ind, Lac, Leo, LMi, Lep, Lib, Lup, Lyn, Lyr, Men, Mic, Mon, Mus,
  Nor, Oct, Oph, Ori, Pav, Peg, Per, Phe, Pic, Psc, PsA, Pup, Pyx, Ret,
  Sge, Sgr, Sco, Scl, Sct, Ser, Sex, Tau, Tel, Tri, TrA, Tuc, UMa, UMi,
  Vel, Vir, Vol, Vul
} CONSTELLATION;

char *constellation_names[] = {
  "And", "Ant", "Aps", "Aqr", "Aql", "Ara", "Ari", "Aur", "Boo",
  "Cae", "Cam", "Cnc", "CVn", "CMa", "CMi", "Cap", "Car", "Cas",
  "Cen", "Cep", "Cet", "Cha", "Cir", "Col", "Com", "CrA", "CrB",
  "Crv", "Crt", "Cru", "Cyg", "Del", "Dor", "Dra", "Equ", "Eri",
  "For", "Gem", "Gru", "Her", "Hor", "Hya", "Hyi", "Ind", "Lac",
  "Leo", "LMi", "Lep", "Lib", "Lup", "Lyn", "Lyr", "Men", "Mic",
  "Mon", "Mus", "Nor", "Oct", "Oph", "Ori", "Pav", "Peg", "Per",
  "Phe", "Pic", "Psc", "PsA", "Pup", "Pyx", "Ret", "Sge", "Sgr",
  "Sco", "Scl", "Sct", "Ser", "Sex", "Tau", "Tel", "Tri", "TrA",
  "Tuc", "UMa", "UMi", "Vel", "Vir", "Vol", "Vul"
};

struct constellation_struct {
  double alpha1;
  double alpha2;
  double delta; 
  CONSTELLATION constellation;
};

#define S (1/(double)3600)

struct constellation_struct constellation_boundaries[357] = {
   {     0*S, 86400*S,  316800*S, UMi, },
   { 28800*S, 52200*S,  311400*S, UMi, },
   { 75600*S, 82800*S,  310200*S, UMi, },
   { 64800*S, 75600*S,  309600*S, UMi, },
   {     0*S, 28800*S,  306000*S, Cep, },
   { 33000*S, 38400*S,  295200*S, Cam, },
   {     0*S, 18000*S,  288000*S, Cep, },
   { 38400*S, 52200*S,  288000*S, Cam, },
   { 63000*S, 64800*S,  288000*S, UMi, },
   { 72600*S, 75600*S,  288000*S, Dra, },
   {     0*S, 12630*S,  277200*S, Cep, },
   { 41400*S, 48900*S,  277200*S, Cam, },
   { 59520*S, 63000*S,  270000*S, UMi, },
   { 72600*S, 74400*S,  270000*S, Cep, },
   { 28680*S, 33000*S,  264600*S, Cam, },
   { 33000*S, 40800*S,  264600*S, Dra, },
   { 46800*S, 59520*S,  252000*S, UMi, },
   { 11160*S, 12300*S,  244800*S, Cas, },
   { 73500*S, 74400*S,  241200*S, Dra, },
   { 40800*S, 43200*S,  239400*S, Dra, },
   {     0*S,  1200*S,  237600*S, Cep, },
   { 50400*S, 56400*S,  237600*S, UMi, },
   { 84900*S, 86400*S,  237600*S, Cep, },
   { 43200*S, 48600*S,  230400*S, Dra, },
   { 48600*S, 51900*S,  226800*S, Dra, },
   { 83400*S, 84900*S,  226800*S, Cep, },
   { 21960*S, 25200*S,  223200*S, Cam, },
   { 72000*S, 73500*S,  221400*S, Dra, },
   { 73932*S, 74160*S,  219300*S, Cep, },
   { 25200*S, 28680*S,  216000*S, Cam, },
   { 28680*S, 30300*S,  216000*S, UMa, },
   { 71160*S, 72000*S,  214200*S, Dra, },
   { 72000*S, 73932*S,  214200*S, Cep, },
   { 82320*S, 83400*S,  212700*S, Cep, },
   {     0*S,  8760*S,  210600*S, Cas, },
   { 69900*S, 71160*S,  208800*S, Dra, },
   {  6120*S,  6870*S,  207000*S, Cas, },
   {  8760*S, 11160*S,  205200*S, Cas, },
   { 11160*S, 11400*S,  205200*S, Cam, },
   { 80340*S, 82320*S,  202500*S, Cep, },
   { 18000*S, 21960*S,  201600*S, Cam, },
   { 50520*S, 51900*S,  199800*S, UMa, },
   { 51900*S, 69900*S,  199800*S, Dra, },
   { 11400*S, 12000*S,  198000*S, Cam, },
   { 79680*S, 80340*S,  198000*S, Cep, },
   { 74160*S, 79080*S,  197400*S, Cep, },
   {     0*S,  6120*S,  194400*S, Cas, },
   { 21960*S, 23400*S,  194400*S, Lyn, },
   { 43500*S, 48600*S,  190800*S, UMa, },
   { 54900*S, 56700*S,  190800*S, Dra, },
   { 79080*S, 79680*S,  189900*S, Cep, },
   { 12000*S, 18000*S,  189000*S, Cam, },
   { 82320*S, 84000*S,  189000*S, Cas, },
   { 56700*S, 61200*S,  185400*S, Dra, },
   {  7350*S,  9060*S,  181800*S, Per, },
   { 61200*S, 65640*S,  181800*S, Dra, },
   {     0*S,  4920*S,  180000*S, Cas, },
   {  4920*S,  6000*S,  180000*S, Per, },
   { 23400*S, 24480*S,  180000*S, Lyn, },
   { 84000*S, 86400*S,  180000*S, Cas, },
   { 48600*S, 50520*S,  174600*S, UMa, },
   {     0*S,  4020*S,  172800*S, Cas, },
   { 84900*S, 86400*S,  172800*S, Cas, },
   { 65430*S, 65640*S,  171000*S, Her, },
   { 65640*S, 68700*S,  171000*S, Dra, },
   { 68700*S, 69000*S,  171000*S, Cyg, },
   {  6000*S,  7350*S,  169200*S, Per, },
   { 30300*S, 33000*S,  169200*S, UMa, },
   {   600*S,  3120*S,  165600*S, Cas, },
   { 43200*S, 43500*S,  162000*S, UMa, },
   { 24480*S, 26520*S,  160200*S, Lyn, },
   { 78870*S, 79080*S,  158400*S, Cyg, },
   { 78750*S, 78870*S,  157500*S, Cyg, },
   { 69000*S, 69840*S,  156600*S, Cyg, },
   { 33000*S, 36600*S,  151200*S, UMa, },
   { 36600*S, 38820*S,  144000*S, UMa, },
   { 55560*S, 56700*S,  144000*S, Boo, },
   { 56700*S, 58800*S,  144000*S, Her, },
   { 33300*S, 34500*S,  143100*S, Lyn, },
   {     0*S,  9060*S,  132300*S, And, },
   {  9060*S,  9240*S,  132300*S, Per, },
   { 69690*S, 69840*S,  131400*S, Lyr, },
   { 16200*S, 16890*S,  129600*S, Per, },
   { 78240*S, 78750*S,  129600*S, Cyg, },
   { 78750*S, 79200*S,  129600*S, Lac, },
   { 23520*S, 26520*S,  127800*S, Aur, },
   { 26520*S, 27900*S,  127800*S, Lyn, },
   {     0*S,  7200*S,  126000*S, And, },
   { 79200*S, 82140*S,  126000*S, Lac, },
   { 82140*S, 82320*S,  124200*S, Lac, },
   { 82320*S, 84600*S,  124200*S, And, },
   {  9240*S,  9780*S,  122400*S, Per, },
   { 38820*S, 39600*S,  122400*S, UMa, },
   { 43200*S, 44400*S,  122400*S, CVn, },
   { 27900*S, 33300*S,  120600*S, Lyn, },
   { 33300*S, 35580*S,  120600*S, LMi, },
   {  2580*S,  5070*S,  118800*S, And, },
   { 54660*S, 55560*S,  118800*S, Boo, },
   { 84600*S, 85500*S,  115500*S, And, },
   { 44400*S, 47700*S,  115200*S, CVn, },
   { 85500*S, 86400*S,  112800*S, And, },
   { 50250*S, 50520*S,  110700*S, CVn, },
   {  8700*S,  9780*S,  110400*S, Tri, },
   {  9780*S, 16200*S,  110400*S, Per, },
   { 16200*S, 17100*S,  108000*S, Aur, },
   { 65430*S, 69690*S,  108000*S, Lyr, },
   { 39600*S, 43200*S,  104400*S, UMa, },
   { 70800*S, 75300*S,  104400*S, Cyg, },
   { 17100*S, 21180*S,  102600*S, Aur, },
   { 35580*S, 37800*S,  102600*S, LMi, },
   { 47700*S, 50250*S,  102600*S, CVn, },
   {     0*S,   240*S,  100800*S, And, },
   {  5070*S,  6000*S,  100800*S, Tri, },
   { 21180*S, 23520*S,  100800*S, Aur, },
   { 28380*S, 28800*S,  100800*S, Gem, },
   { 75300*S, 78240*S,  100800*S, Cyg, },
   { 69330*S, 70800*S,   99000*S, Cyg, },
   {  6900*S,  8700*S,   98100*S, Tri, },
   { 58200*S, 58800*S,   97200*S, CrB, },
   { 54300*S, 54660*S,   93600*S, Boo, },
   { 54660*S, 58200*S,   93600*S, CrB, },
   { 66120*S, 67920*S,   93600*S, Lyr, },
   { 38700*S, 39600*S,   91800*S, LMi, },
   { 67920*S, 69330*S,   91800*S, Lyr, },
   {  6000*S,  6900*S,   90000*S, Tri, },
   {  2580*S,  3060*S,   85500*S, Psc, },
   { 37800*S, 38700*S,   84600*S, LMi, },
   { 76500*S, 77100*S,   84600*S, Vul, },
   { 20520*S, 21180*S,   82200*S, Tau, },
   {   240*S,   510*S,   79200*S, And, },
   { 57300*S, 57720*S,   79200*S, Ser, },
   { 21180*S, 22380*S,   77400*S, Gem, },
   { 71400*S, 72900*S,   76500*S, Vul, },
   { 67920*S, 69300*S,   75900*S, Vul, },
   {   510*S,  3060*S,   75600*S, And, },
   { 72900*S, 74040*S,   73800*S, Vul, },
   { 28110*S, 28380*S,   72000*S, Gem, },
   { 74040*S, 76500*S,   70200*S, Vul, },
   { 69300*S, 71400*S,   69000*S, Vul, },
   { 11820*S, 12120*S,   68400*S, Ari, },
   { 67920*S, 68400*S,   66600*S, Sge, },
   { 20520*S, 20760*S,   64800*S, Ori, },
   { 22380*S, 22710*S,   63000*S, Gem, },
   { 68400*S, 71400*S,   58200*S, Sge, },
   { 17880*S, 19200*S,   57600*S, Tau, },
   { 57300*S, 57900*S,   57600*S, Her, },
   { 71400*S, 72900*S,   56700*S, Sge, },
   { 16620*S, 17880*S,   55800*S, Tau, },
   { 19200*S, 20160*S,   55800*S, Tau, },
   { 46200*S, 48600*S,   54000*S, Com, },
   { 62100*S, 65700*S,   51600*S, Her, },
   { 42720*S, 46200*S,   50400*S, Com, },
   { 27000*S, 28110*S,   48600*S, Gem, },
   { 60300*S, 62100*S,   46200*S, Her, },
   {     0*S,   510*S,   45000*S, Peg, },
   { 20160*S, 20760*S,   45000*S, Tau, },
   { 25200*S, 27000*S,   45000*S, Gem, },
   { 76020*S, 76800*S,   45000*S, Peg, },
   { 22710*S, 24960*S,   43200*S, Gem, },
   { 65700*S, 67920*S,   43200*S, Her, },
   { 75150*S, 75780*S,   42600*S, Del, },
   { 75780*S, 76020*S,   42600*S, Peg, },
   { 41460*S, 42720*S,   39600*S, Leo, },
   { 22470*S, 22710*S,   36000*S, Ori, },
   { 24960*S, 25200*S,   36000*S, Gem, },
   { 28110*S, 28530*S,   36000*S, Cnc, },
   { 85800*S, 86400*S,   36000*S, Peg, },
   {  6000*S, 11820*S,   35700*S, Ari, },
   { 72510*S, 73080*S,   30600*S, Del, },
   { 48600*S, 54300*S,   28800*S, Boo, },
   { 81900*S, 85800*S,   27000*S, Peg, },
   { 28530*S, 33300*S,   25200*S, Cnc, },
   { 33300*S, 38700*S,   25200*S, Leo, },
   { 65700*S, 67184*S,   22500*S, Oph, },
   { 67184*S, 67920*S,   22500*S, Aql, },
   { 75000*S, 75150*S,   21600*S, Del, },
   { 25200*S, 25260*S,   19800*S, CMi, },
   { 65700*S, 66330*S,   16200*S, Ser, },
   { 57900*S, 60300*S,   14400*S, Her, },
   { 65700*S, 66330*S,   10800*S, Oph, },
   { 77280*S, 78000*S,    9900*S, Peg, },
   {     0*S,  7200*S,    7200*S, Psc, },
   { 66900*S, 67920*S,    7200*S, Ser, },
   { 73080*S, 75000*S,    7200*S, Del, },
   { 75000*S, 76800*S,    7200*S, Equ, },
   { 76800*S, 77280*S,    7200*S, Peg, },
   { 79200*S, 81900*S,    7200*S, Peg, },
   { 78000*S, 79200*S,    6300*S, Peg, },
   { 25260*S, 25920*S,    5400*S, CMi, },
   { 12900*S, 16620*S,       0*S, Tau, },
   { 16620*S, 16800*S,       0*S, Ori, },
   { 25920*S, 29100*S,       0*S, CMi, },
   { 52800*S, 54300*S,       0*S, Vir, },
   { 64200*S, 65700*S,       0*S, Oph, },
   {  9540*S, 11820*S,   -6300*S, Cet, },
   { 11820*S, 12900*S,   -6300*S, Tau, },
   { 54300*S, 58560*S,  -11700*S, Ser, },
   { 16800*S, 18300*S,  -14400*S, Ori, },
   { 21000*S, 22470*S,  -14400*S, Ori, },
   { 64200*S, 64680*S,  -14400*S, Ser, },
   { 65700*S, 66900*S,  -14400*S, Ser, },
   { 66900*S, 67920*S,  -14400*S, Aql, },
   { 81900*S, 85800*S,  -14400*S, Psc, },
   { 38700*S, 41460*S,  -21600*S, Leo, },
   { 41460*S, 42600*S,  -21600*S, Vir, },
   {     0*S,  1200*S,  -25200*S, Psc, },
   { 85800*S, 86400*S,  -25200*S, Psc, },
   { 51300*S, 52800*S,  -28800*S, Vir, },
   { 57300*S, 58560*S,  -28800*S, Oph, },
   { 72000*S, 73920*S,  -32400*S, Aql, },
   { 76800*S, 78720*S,  -32400*S, Aqr, },
   { 61800*S, 64680*S,  -36000*S, Oph, },
   { 21000*S, 29100*S,  -39600*S, Mon, },
   { 17700*S, 18300*S,  -39600*S, Eri, },
   { 18300*S, 21000*S,  -39600*S, Ori, },
   { 29100*S, 30120*S,  -39600*S, Hya, },
   { 34500*S, 38700*S,  -39600*S, Sex, },
   { 42600*S, 46200*S,  -39600*S, Vir, },
   { 63300*S, 63600*S,  -42000*S, Oph, },
   { 67920*S, 72000*S,  -43320*S, Aql, },
   { 17400*S, 17700*S,  -52200*S, Eri, },
   { 73920*S, 76800*S,  -54000*S, Aqr, },
   { 61800*S, 65700*S,  -57600*S, Ser, },
   { 65700*S, 67920*S,  -57600*S, Sct, },
   { 30120*S, 30900*S,  -61200*S, Hya, },
   { 58560*S, 58950*S,  -65700*S, Oph, },
   { 30900*S, 32700*S,  -68400*S, Hya, },
   { 38700*S, 39000*S,  -68400*S, Crt, },
   { 58560*S, 58950*S,  -69300*S, Sco, },
   { 56400*S, 57300*S,  -72000*S, Lib, },
   { 45300*S, 46200*S,  -79200*S, Crv, },
   { 46200*S, 51300*S,  -79200*S, Vir, },
   { 32700*S, 35100*S,  -86400*S, Hya, },
   {  6000*S,  9540*S,  -87780*S, Cet, },
   {  9540*S, 13500*S,  -87780*S, Eri, },
   { 39000*S, 42600*S,  -88200*S, Crt, },
   { 42600*S, 45300*S,  -88200*S, Crv, },
   { 51300*S, 53700*S,  -88200*S, Lib, },
   { 58560*S, 60300*S,  -88500*S, Oph, },
   {     0*S,  6000*S,  -91800*S, Cet, },
   { 76800*S, 78720*S,  -91800*S, Cap, },
   { 78720*S, 85800*S,  -91800*S, Aqr, },
   { 85800*S, 86400*S,  -91800*S, Cet, },
   { 35100*S, 36900*S,  -95400*S, Hya, },
   { 16920*S, 17400*S,  -98100*S, Eri, },
   { 17400*S, 22020*S,  -98100*S, Lep, },
   { 72000*S, 76800*S, -100800*S, Cap, },
   { 36900*S, 38100*S, -105000*S, Hya, },
   { 45300*S, 53700*S, -106200*S, Hya, },
   { 53700*S, 56400*S, -106200*S, Lib, },
   { 56400*S, 57600*S, -106200*S, Sco, },
   { 16500*S, 16920*S, -108000*S, Eri, },
   { 60300*S, 63360*S, -108000*S, Oph, },
   { 63360*S, 64200*S, -108000*S, Sgr, },
   { 38100*S, 39000*S, -112200*S, Hya, },
   { 22020*S, 26520*S, -118800*S, CMa, },
   { 44100*S, 45300*S, -118800*S, Hya, },
   { 39000*S, 44100*S, -126000*S, Hya, },
   { 12600*S, 13500*S, -129600*S, For, },
   { 30120*S, 33720*S, -132300*S, Pyx, },
   { 15360*S, 16500*S, -133200*S, Eri, },
   { 64200*S, 69000*S, -133200*S, Sgr, },
   { 76800*S, 82800*S, -133200*S, PsA, },
   { 82800*S, 84000*S, -133200*S, Scl, },
   { 10800*S, 12600*S, -142500*S, For, },
   { 33720*S, 39600*S, -143100*S, Ant, },
   {     0*S,  6000*S, -144000*S, Scl, },
   {  6000*S, 10800*S, -144000*S, For, },
   { 13920*S, 15360*S, -144000*S, Eri, },
   { 84000*S, 86400*S, -144000*S, Scl, },
   { 51000*S, 53700*S, -151200*S, Cen, },
   { 56400*S, 57600*S, -151200*S, Lup, },
   { 57600*S, 59115*S, -151200*S, Sco, },
   { 17400*S, 18000*S, -154800*S, Cae, },
   { 18000*S, 23700*S, -154800*S, Col, },
   { 28800*S, 30120*S, -154800*S, Pup, },
   { 12300*S, 13920*S, -158400*S, Eri, },
   { 59115*S, 64200*S, -163800*S, Sco, },
   { 64200*S, 69000*S, -163800*S, CrA, },
   { 69000*S, 73200*S, -163800*S, Sgr, },
   { 73200*S, 76800*S, -163800*S, Mic, },
   { 10800*S, 12300*S, -165600*S, Eri, },
   { 16200*S, 17400*S, -167400*S, Cae, },
   { 55200*S, 56400*S, -172800*S, Lup, },
   {     0*S,  8400*S, -173400*S, Phe, },
   {  9600*S, 10800*S, -176400*S, Eri, },
   { 14700*S, 15360*S, -176400*S, Hor, },
   { 15360*S, 16200*S, -176400*S, Cae, },
   { 76800*S, 79200*S, -180000*S, Gru, },
   { 21600*S, 28800*S, -182700*S, Pup, },
   { 28800*S, 29400*S, -182700*S, Vel, },
   {  8700*S,  9600*S, -183600*S, Eri, },
   { 13800*S, 14700*S, -183600*S, Hor, },
   {     0*S,  6600*S, -185400*S, Phe, },
   { 21600*S, 22200*S, -189000*S, Car, },
   { 29400*S, 30420*S, -190800*S, Vel, },
   { 12600*S, 13800*S, -191400*S, Hor, },
   { 13800*S, 14400*S, -191400*S, Dor, },
   {     0*S,  5700*S, -192600*S, Phe, },
   {  7800*S,  8700*S, -194400*S, Eri, },
   { 16200*S, 18000*S, -194400*S, Pic, },
   { 54180*S, 55200*S, -194400*S, Lup, },
   { 30420*S, 31800*S, -196200*S, Vel, },
   { 22200*S, 23400*S, -198000*S, Car, },
   { 42600*S, 46200*S, -198000*S, Cen, },
   { 51000*S, 54180*S, -198000*S, Lup, },
   { 54180*S, 55200*S, -198000*S, Nor, },
   { 14400*S, 15600*S, -203400*S, Dor, },
   { 31800*S, 39600*S, -203400*S, Vel, },
   { 39600*S, 40500*S, -203400*S, Cen, },
   { 63000*S, 64800*S, -205200*S, Ara, },
   { 64800*S, 73200*S, -205200*S, Tel, },
   { 79200*S, 84000*S, -205200*S, Gru, },
   { 11520*S, 12600*S, -207000*S, Hor, },
   { 18000*S, 19800*S, -207000*S, Pic, },
   { 23400*S, 24600*S, -208800*S, Car, },
   {     0*S,  4800*S, -210600*S, Phe, },
   {  4800*S,  7800*S, -210600*S, Eri, },
   { 84000*S, 86400*S, -210600*S, Phe, },
   { 15600*S, 16500*S, -212400*S, Dor, },
   { 55200*S, 59115*S, -216000*S, Nor, },
   { 73200*S, 76800*S, -216000*S, Ind, },
   { 19800*S, 21600*S, -219600*S, Pic, },
   { 54600*S, 55200*S, -219600*S, Cir, },
   { 59115*S, 59700*S, -219600*S, Ara, },
   { 53700*S, 54600*S, -228900*S, Cir, },
   { 59700*S, 60300*S, -228900*S, Ara, },
   { 21600*S, 24600*S, -230400*S, Pic, },
   { 24600*S, 32520*S, -230400*S, Car, },
   { 40500*S, 42600*S, -230400*S, Cen, },
   { 42600*S, 46200*S, -230400*S, Cru, },
   { 46200*S, 52320*S, -230400*S, Cen, },
   { 48600*S, 49200*S, -234000*S, Cir, },
   { 60300*S, 60600*S, -234000*S, Ara, },
   {  7800*S, 11520*S, -243000*S, Hor, },
   { 11520*S, 16500*S, -243000*S, Ret, },
   { 53100*S, 53700*S, -243000*S, Cir, },
   { 60600*S, 63000*S, -243000*S, Ara, },
   { 63000*S, 64800*S, -243000*S, Pav, },
   { 79200*S, 84000*S, -243000*S, Tuc, },
   { 16500*S, 23700*S, -252000*S, Dor, },
   { 49200*S, 53100*S, -252000*S, Cir, },
   { 53100*S, 61200*S, -252000*S, TrA, },
   {     0*S,  4800*S, -270000*S, Tuc, },
   { 12600*S, 16500*S, -270000*S, Hyi, },
   { 23700*S, 32520*S, -270000*S, Vol, },
   { 32520*S, 40500*S, -270000*S, Car, },
   { 40500*S, 49200*S, -270000*S, Mus, },
   { 64800*S, 76800*S, -270000*S, Pav, },
   { 76800*S, 84000*S, -270000*S, Ind, },
   { 84000*S, 86400*S, -270000*S, Tuc, },
   {  2700*S,  4800*S, -273600*S, Tuc, },
   {     0*S, 12600*S, -297000*S, Hyi, },
   { 27600*S, 49200*S, -297000*S, Cha, },
   { 49200*S, 64800*S, -297000*S, Aps, },
   { 12600*S, 27600*S, -306000*S, Men, },
   {     0*S, 86400*S, -324000*S, Oct, },
};
