#define NUMBEROFPLANETTERMS (39198)
struct planetIndex { short unsigned int index; short unsigned int nTerms; };
double planetTerms[3*NUMBEROFPLANETTERMS];

void XYZfromVSOPA(double T, int object, double *pos, double tolerance, int getErrors);
struct planetIndex *planetIndicesForTolerance(double tolerance);
