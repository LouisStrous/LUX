#ifndef CALENDAR_H
#define CALENDAR_H

/* HEADERS */
#include <math.h> /* for floor */
/* END HEADERS */

/** \file calendar.h Calendrical calculations. 

    The functions that work with only integer numbers express dates in
    terms of the Chronological Julian Day Number (CJDN).  Every
    calendar day has a CJDN that is 1 greater than the CJDN of the
    preceding calendar day.  CJDN 0 corresponds to 1 January -4712 on
    the Julian calendar.

    The first year of each calendar is year 1.  The year preceding
    that year is called year 0, and the year before that is called
    year -1, and so on.
    
    The functions that work with fractional days are mostly in terms
    of the Chronological Julian Day (CJD).  A CJD begins at midnight
    local time and runs until the next midnight local time.  NOTE that
    CJDs depend on the local time zone, including daylight savings
    time adjustments.  CJD does not provide a uniform time scale, but
    jumps by 1 hour at transitions between daylight savings time and
    standard time.

    Functions are provided to translate between Chronological Julian
    Day and Julian Day (JD).  A Julian Day begins at noon UTC or TT
    and runs until the next noon UTC or TT.  JD based on TT provides a
    uniform time scale.  JD based on UTC does not provide a uniform
    time scale, but jumps by 1 second when leap seconds are inserted.

    CJD = JD + 0.5 + TZ, where TZ is a time zone adjustment; CJDN =
    floor(CJD).

    The Common calendar is equal to the Julian calendar for dates up
    to 1582-10-04, and equal to the Gregorian calendar for later
    dates.

    The implemented Islamic calendar is the most commonly used tabular
    calendar, with the epoch corresponding to Friday 16 July 622 on
    the Julian calendar.  Islamic calendar dates begin at sunset of
    the preceding Julian/Gregorian calendar date.

    Hebrew calendar dates begin at sunset of the preceding
    Julian/Gregorian calendar date.

    The Egyptian calendar is according to the era of Seleukos, with
    the epoch corresponding to 26 February -747 on the Julian
    calendar.  */

/** Translates a Chronological Julian Day Number to a Gregorian
    calendar date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void CJDNtoGregorian(int CJDN, int *year, int *month, int *day);

/** Translates a Chronological Julian Day Number to a Gregorian
    calendar date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!
    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, date).
 */
void CJDNtoGregorianA(int const *CJDN, int *date);

/** Translates a Chronological Julian Day Number to Gregorian
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN a pointer to the Chronological Julian Day number
    to translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void CJDNtoGregorianSA(int const *CJDN, char **date);

/** Translates a Chronological Julian Day to a Gregorian calendar
    date.

    \param[in] CJD the Chronological Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void CJDtoGregorian(double CJD, int *year, int *month, double *day);

/** Translates a Chronological Julian Day to a Gregorian calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void CJDtoGregorianA(double const *CJD, double *date);

/** Translates a Chronological Julian Day to Gregorian calendar
    date text, consisting of the (fractional) day number, the full
    month name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void CJDtoGregorianSA(double const *CJD, char **date);

/** Translates a Gregorian calendar date to a Chronological Julian Day
    in text format.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
int GregoriantoCJDN(int year, int month, int day);

/** Translates a Gregorian calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL.
 */
void GregoriantoCJDNA(int const *date, int *CJDN);

/** Translates a Gregorian calendar date to a Chronological Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day
 */
double GregoriantoCJD(int year, int month, double day);

/** Translates a Gregorian calendar date to a Chronological Julian Day.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void GregoriantoCJDA(double const *date, double *CJD);

/** Translates a Gregorian calendar date to a Chronological Julian
    Day.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of a (possibly
    fractional) day number, a full month name (in English), and a year
    number, separated by whitespace.

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void GregorianStoCJDA(char * const *date, double *CJD);

/** Translates a Chronological Julian Day Number to a Julian calendar
    date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void CJDNtoJulian(int CJDN, int *year, int *month, int *day);

/** Translates a Chronological Julian Day Number to a Julian calendar
    date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void CJDNtoJulianA(int const *CJDN, int *date);

/** Translates a Chronological Julian Day Number to Julian calendar
    date text, consisting of the day number, the full month name (in
    English), and the year number, separated by single whitespaces.

    \param[in] CJDN a pointer to the Chronological Julian Day number
    to translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void CJDNtoJulianSA(int const *CJDN, char **date);

/** Translates a Chronological Julian Day to a Julian calendar date.

    \param[in] CJD the Chronological Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void CJDtoJulian(double CJD, int *year, int *month, double *day);

/** Translates a Chronological Julian Day to a Julian calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements.
 */
void CJDtoJulianA(double const *CJD, double *date);

/** Translates a Chronological Julian Day to Julian calendar date
    text, consisting of the day number, the full month name (in
    English), and the year number, separated by single whitespaces.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
*/
void CJDtoJulianSA(double const *CJD, char **date);

/** Translates a Julian calendar date to a Chronological Julian Day
    Number.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
int JuliantoCJDN(int year, int month, int day);

/** Translates a Julian calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date to translate.  Must
    have at least 3 elements (year, month, day).

    \param[out] date a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL!
 */
void JuliantoCJDNA(int const *date, int *CJDN);

/** Translates a Julian calendar date to a Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Julian Day
 */
double JuliantoCJD(int year, int month, double day);

/** Translates a Julian calendar date to a Chronological Julian Day.

    \param[in] date a pointer to the calendar date to translate.  Must
    have at least 3 elements.

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void JuliantoCJDA(double const *date, double *CJD);

/** Translates a Julian calendar date to a Chronological Julian
    Day.  The opposite of \ref CJDtoJulianSA.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void JulianStoCJDA(char * const *date, double *CJD);

/** Translates a Chronological Julian Day Number to a Common calendar
    date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void CJDNtoCommon(int CJDN, int *year, int *month, int *day);

/** Translates a Chronological Julian Day Number to a Common calendar
    date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void CJDNtoCommonA(int const *CJDN, int *date);

/** Translates a Chronological Julian Day Number to Common calendar
    date text, consisting of the day number, the full month name (in
    English), and the year number, separated by single whitespaces.

    \param[in] CJDN a pointer to the Chronological Julian Day number
    to translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void CJDNtoCommonSA(int const *CJDN, char **date);

/** Translates a Chronological Julian Day to a Common calendar date.

    \param[in] CJD the Chronological Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void CJDtoCommon(double CJD, int *year, int *month, double *day);

/** Translates a Chronological Julian Day to a Common calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding date.  Must
    have at least 3 elements (year, month, day).
 */
void CJDtoCommonA(double const *CJD, double *date);

/** Translates a Chronological Julian Day to Common calendar date
    text, consisting of the day number, the full month name (in
    English), and the year number, separated by single whitespaces.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
*/
void CJDtoCommonSA(double const *CJD, char **date);

/** Translates a Common calendar date to a Chronological Julian Day
    Number.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
int CommontoCJDN(int year, int month, int day);

/** Translates a Common calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL!
 */
void CommontoCJDNA(int const *date, int *CJDN);

/** Translates a Common calendar date to a Chronological Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day
 */
double CommontoCJD(int year, int month, double day);

/** Translates a Common calendar date to a Chronological Julian Day.

    \param[in] date a pointer to the calendar date to translate.  Must
    have at least 3 elements (year, month, day).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void CommontoCJDA(double const *date, double *CJD);

/** Translates Common calendar date text to a Chronological Julian
    Day.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of a (possibly
    fractional) day number, a full month name, and a year number,
    separated by whitespace.

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void CommonStoCJDA(char * const *date, double *CJDN);

/** Translates a Chronological Julian Day Number to a Hebrew calendar
    date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void CJDNtoHebrew(int CJDN, int *year, int *month, int *day);

/** Translates a Chronological Julian Day Number to a Hebrew calendar
    date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements.
 */
void CJDNtoHebrewA(int const *CJDN, int *date);

/** Translates a Chronological Julian Day Number to Hebrew calendar
    date text, consisting of the day number, the full month name (in
    English), and the year number, separated by single whitespaces.

    \param[in] CJDN a pointer to the Chronological Julian Day number
    to translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void CJDNtoHebrewSA(int const *CJDN, char **date);

/** Translates a Chronological Julian Day to a Hebrew calendar date.

    \param[in] CJD the Chronological Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void CJDtoHebrew(double CJD, int *year, int *month, double *day);

/** Translates a Chronological Julian Day to a Hebrew calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements.
 */
void CJDtoHebrewA(double const *CJD, double *date);

/** Translates a Chronological Julian Day to Hebrew calendar date
    text, consisting of the (fractional) day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void CJDtoHebrewSA(double const *CJD, char **date);

/** Translates a Hebrew calendar date to a Chronological Julian Day
    Number.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
int HebrewtoCJDN(int year, int month, int day);

/** Translates a Hebrew calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date to translate.  Must
    have at least 3 elements (year, month, day).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL.
 */
void HebrewtoCJDNA(int const *date, int *CJDN);

/** Translates a Hebrew calendar date to a Chronological Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day
 */
double HebrewtoCJD(int year, int month, double day);

/** Translates a Hebrew calendar date to a Chronological Julian Day.

    \param[in] date a pointer to the calendar date to translate.  Must
    have at least 3 elements (year, month, day).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void HebrewtoCJDA(double const *date, double *CJD);

/** Translates a Hebrew calendar date to a Chronological Julian Day.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of a (possibly
    fractional) day number, a full month name (in English), and a year
    number, separated by whitespace.

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void HebrewStoCJDA(char * const *date, double *CJD);

/** Translates a Chronological Julian Day Number to an Islamic
    calendar date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void CJDNtoIslamic(int CJDN, int *year, int *month, int *day);

/** Translates a Chronological Julian Day Number to an Islamic
    calendar date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void CJDNtoIslamicA(int const *CJDN, int *date);

/** Translates a Chronological Julian Day Number to Islamic calendar
    date text, consisting of the day number, the full month name (in
    English), and the year number, separated by single whitespaces.

    \param[in] CJDN a pointer to the Chronological Julian Day number
    to translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void CJDNtoIslamicSA(int const *CJDN, char **date);

/** Translates a Julian Day to an Islamic calendar date.

    \param[in] JD the Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void CJDtoIslamic(double CJD, int *year, int *month, double *day);

/** Translates a Chronological Julian Day to an Islamic calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void CJDtoIslamicA(double const *CJD, double *date);

/** Translates a Chronological Julian Day to Islamic calendar date
    text, consisting of the (fractional) day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void CJDtoIslamicSA(double const *CJD, char **date);

/** Translates an Islamic calendar date to a Chronological Julian Day
    Number.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
int IslamictoCJDN(int year, int month, int day);

/** Translates an Islamic calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date to translate.  Must
    have at least 3 elements (year, month, day).

    \param[out] CJDN a pointer to the corresponding Chronological Julian
    Day Number.  Must not be NULL!
 */
void IslamictoCJDNA(int const *date, int *CJDN);

/** Translates an Islamic calendar date to a Chronological Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day
 */
double IslamictoCJD(int year, int month, double day);

/** Translates an Islamic calendar date to a Chronological Julian Day.

    \param[in] date a pointer to the calendar date to translate.  Must
    have at least 3 elements (year, month, day).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void IslamictoCJDA(double const *date, double *CJD);

/** Translates an Islamic calendar date to a Chronological Julian Day.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of a (possibly
    fractional) day number, a full month name (in English), and a year
    number, separated by whitespace.

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void IslamicStoCJDA(char * const *date, double *CJD);

/** Translates a Chronological Julian Day Number to an Egyptian
    calendar date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void CJDNtoEgyptian(int CJDN, int *year, int *month, int *day);

/** Translates a Chronological Julian Day Number to an Egyptian
    calendar date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void CJDNtoEgyptianA(int const *CJDN, int *date);

/** Translates a Chronological Julian Day Number to Egyptian calendar
    date text, consisting of the day number, the full month name (in
    English), and the year number, separated by single whitespaces.

    \param[in] CJDN a pointer to the Chronological Julian Day number
    to translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void CJDNtoEgyptianSA(int const *CJDN, char **date);

/** Translates a Chronological Julian Day to an Egyptian calendar
    date.

    \param[in] CJD the Chronological Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void CJDtoEgyptian(double CJD, int *year, int *month, double *day);

/** Translates a Chronological Julian Day to an Egyptian calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void CJDtoEgyptianA(double const *CJD, double *date);

/** Translates a Chronological Julian Day to Egyptian calendar date
    text, consisting of the (fractional) day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void CJDtoEgyptianSA(double const *CJD, char **date);

/** Translates an Egyptian calendar date to a Chronological Julian Day
    Number.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
int EgyptiantoCJDN(int year, int month, int day);

/** Translates an Egyptian calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date to translate.  Must
    have at least 3 elements (year, month, day).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL!
 */
void EgyptiantoCJDNA(int const *date, int *CJDN);

/** Translates an Egyptian calendar date to a Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Julian Day
 */
double EgyptiantoCJD(int year, int month, double day);

/** Translates an Egyptian calendar date to a Chronological Julian
    Day.

    \param[in] day a pointer to the calendar date to translate.  Must have
    at least 3 elements (year, month, day).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void EgyptiantoCJDA(double const *date, double *CJD);

/** Translates an Egyptian calendar date to a Chronological Julian
    Day.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of a (possibly
    fractional) day number, a full month name (in English), and a year
    number, separated by whitespace.

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void EgyptianStoCJDA(char * const *date, double *CJD);

/** Translates Julian Day to Chronological Julian Day.

    \param[in] JD the Julian Day to translate

    \return the corresponding Chronological Julian Day
 */
double JDtoCJD(double JD);

/** Translates a Julian Day to a Chronological Julian Day for the
    current time zone.

    \param[in] JD a pointer to the Julian Day to translate.  Must not
    be NULL!

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void JDtoCJDA(double const *JD, double *CJD);

/** Translates Chronological Julian Day for the current time zone to
    Julian Day.
    
    \param[in] CJD the Chronological Julian Day to translate

    \return the corresponding Julian Day

 */
double CJDtoJD(double CJD);

/** Translates a Chronological Julian Day to a Julian Day.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] JD a pointer to the corresponding Julian Day.
    Must not be NULL!

 */
void CJDtoJDA(double const *CJD, double *JD);

/** Copies a Chronological Julian Day.

    \param[in] in a pointer to the Chronological Julian Day to copy.
    Must not be NULL!

    \param[out] out a pointer to the target.  Must not be NULL!

 */
void CJDNtoCJDNA(int const *in, int *out);

/** Copies a Chronological Julian Day Number.

    \param[in] in a pointer to the Chronological Julian Day Number to
    copy.  Must not be NULL!

    \param[out] out a pointer to the target.  Must not be NULL!

 */
void CJDtoCJDA(double const *in, double *out);

/** Translates a Chronological Julian Day to a (fractional) count of
    lunar months.

    \param[in] CJD the Chronological Julian Day
    \return the corresponding lunar month
 */ 
double CJDtoLunar(double CJD);

/** Translates a Chronological Julian Day to a (fractional) count of
    lunar months.

    \param[in] CJD a pointer to the Chronological Julian Day.  Must
    not be NULL!

    \param[out] lunar a pointer to the corresponding lunar month.
    Must not be NULL!
 */ 
void CJDtoLunarA(double const *CJD, double *lunar);

/** Translates a (fractional) count of lunar months to a Chronological
    Julian Day.

    \param[in] lunar the lunar month
    \return the corresponding Chronological Julian Day
 */ 
double LunartoCJD(double lunar);

/** Translates a (fractional) count of lunar months to a Chronological
    Julian Day.

    \param[in] lunar a pointer to the lunar month.  Must not be NULL!

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */ 
void LunartoCJDA(double const *lunar, double *CJD);

/** Translates a Chronological Julian Day Number into a Mayan
    tzolkin/haab calendar date (from Tikal).

    \param[in] CJDN the Chronological Julian Day Number to translate

    \param[out] trecena a pointer to the trecena day count (between
    1 and 13, inclusive).  Must not be NULL!

    \param[out] venteina a pointer to the venteina day count (between
    1 and 20, inclusive).  Must not be NULL!

    \param[out] haab_day a pointer to the haab day of the month
    (between 0 and 19, inclusive).  Must not be NULL!

    \param[out] haab_month a pointer to the haab month number (between
    1 and 18, inclusive).  Must not be NULL!

    \param[out] year_trecena a pointer to the trecena day count of the
    first day of the haab year.  Must not be NULL!

    \param[out] year_venteina a pointer to the venteina day count of
    the first day of the haab year.  Must not be NULL!
 */
void CJDNtoMayan(int CJDN, int *trecena, int *venteina,
                 int *haab_day, int *haab_month,
                 int *year_trecena, int *year_venteina);

/** Translates a Chronological Julian Day Number into a Mayan
    tzolkin/haab calendar date (from Tikal).

    \param[in] CJDN a pointer to the Chronological Julian Day Number
    to translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding Mayan date.  Must
    have at least 6 elements (trecena, venteina, haab day, haab month,
    trecena of first day of haab year, venteina of first day of haab
    year)!
 */
void CJDNtoMayanA(int const *CJDN, int *date);

/** Translates a Chronological Julian Day Number to Mayan calendar
    date text, consisting of the trecena number, the full venteina
    name, the haab day number, the full haab month name, the trecena
    number of the first day of the haab year, and the full venteina
    name of the first day of the haab year, all separated by single
    whitespaces, and with the first-day-of-the-year part enclosed in
    parentheses.

    \param[in] CJDN a pointer to the Chronological Julian Day number
    to translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void CJDNtoMayanSA(int const *CJDN, char **date);

/** Translates a Chronological Julian Day Number to a Maya Long Count.

    \param[in] CJDN the Chronological Julian Day Number to translate

    \param[out] longcount pointer to the corresponding Long Count,
    which must have at least 6 elements (from least significant to
    most significant).  The first 5 elements are restricted to ranges
    from 0 through, respectively, 20 [kin], 18 [uinal], 20 [tun], 20
    [katun], and 13 [baktun].  The 6th element [may] is not
    restricted.
 */
void CJDNtoLongCount(int CJDN, int *longcount);

/** Translates a Chronological Julian Day Number to a Maya Long Count.

    \param[in] CJDN pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!

    \param[out] longcount pointer to the corresponding Long Count,
    which must have at least 6 elements (from least significant to
    most significant).  The first 5 elements are restricted to ranges
    from 0 through, respectively, 20 [kin], 18 [uinal], 20 [tun], 20
    [katun], and 13 [baktun].  The 6th element [may] is not
    restricted.
 */
void CJDNtoLongCountA(int const *CJDN, int *longcount);

/** Translates a Chronological Julian Day Number to Mayan Long Count
    calendar date text, consisting of the five numbers in decreasing
    order of significance, separated by periods.

    \param[in] CJDN a pointer to the Chronological Julian Day number
    to translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void CJDNtoLongCountSA(int const *CJDN, char **longcount);

/** Translates a Maya Long Count to a Chronological Julian Day Number.

    \param[in] longcount a pointer to the Maya Long Count, which must
    have at least 6 elements (from least significant to most
    significant).

    \return the corresponding Chronological Julian Day Number.
 */
int LongCounttoCJDN(int const *longcount);

/** Translates a Maya Long Count to a Chronological Julian Day Number.

    \param[in] longcount a pointer to the Maya Long Count, which must
    have at least 6 elements (from least significant to most
    significant).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL!
 */
void LongCounttoCJDNA(int const *longcount, int *CJDN);

#endif
