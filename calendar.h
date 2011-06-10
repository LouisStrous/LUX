#define intfloor(value) (((value) < 0)? -(int)(-value): (int)(value))
#define divfloor(numerator,denominator) ((((numerator) < 0) ^ ((denominator) < 0))? -(int)((-(numerator))/(denominator)): (int)((numerator)/(denominator)))

void JDtoCommonDate(double JD, int *year, int *month, double *day);
double CommonDateToJD(int year, int month, double day);
