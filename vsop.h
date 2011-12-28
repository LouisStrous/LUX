struct planetIndex {
  short unsigned int index;
  short unsigned int nTerms;
};
struct VSOPdata {
  struct planetIndex indices[6*3*8];
  short unsigned int nTerms;
  double *terms;
};
struct VSOPdata planetIndicesForTolerance(struct VSOPdata *data, 
                                          double tolerance);
struct VSOPdata VSOP87Adata;
struct VSOPdata VSOP87Cdata;

void XYZJ2000fromVSOPA(double T, int object, double *pos, double tolerance);
void XYZdatefromVSOPC(double T, int object, double *pos, double tolerance);

