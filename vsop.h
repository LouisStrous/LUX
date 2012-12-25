#include "action.h"

struct planetIndex {
  int16_t index;
  int16_t nTerms;
};
struct VSOPdata {
  struct planetIndex indices[6*3*8];
  int16_t nTerms;
  Double *terms;
};
struct VSOPdata planetIndicesForTolerance(struct VSOPdata *data, 
                                          Double tolerance);
struct VSOPdata VSOP87Adata;
struct VSOPdata VSOP87Cdata;

void XYZJ2000fromVSOPA(Double T, Int object, Double *pos, Double tolerance);
void XYZdatefromVSOPC(Double T, Int object, Double *pos, Double tolerance);

