#ifndef CALENDAR_H
#define CALENDAR_H

#include "action.h"
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
    calendar.  
    
    The function names are built up as follows:
    \li \c CJDNtoCal = from CJDN to calendar date
    \li \c CJDtoCal = from CJD to calendar date
    \li \c CaltoCJDN = from calendar date to CJDN
    \li \c CaltoCJD = from calendar date to CJD
    \li \c -S- = to/from a calendar date in text format
    \li \c -A = with all arguments pointers to arrays

*/

/** Returns the current Chronological Julian Day value based on the
    system's time zone.

    \return the current Chronological Julian Day value
 */
Double CJD_now(void);

/** Returns the current Chronological Julian Day Number based on the
    system's time zone.

    \return the current Chronological Julian Day Number
*/
Int CJDN_now(void);

/* GREGORIAN CALENDAR */

/**    
    Translates a Chronological Julian Day Number to a Gregorian
    calendar date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDNtoGregorian(Int CJDN, Int *year, Int *month, Int *day);

/** Translates a Chronological Julian Day Number to a Gregorian
    calendar date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!
    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, date).
 */
void   CJDNtoGregorianA(Int const *CJDN, Int *date);

/** Translates a Chronological Julian Day Number to Gregorian
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN the Chronological Julian Day number to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
char * CJDNtoGregorianS(Int CJDN);

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
void   CJDNtoGregorianSA(Int const *CJDN, char **date);

/** Translates a Chronological Julian Day to a Gregorian
    date.

    \param[in] CJD the Chronological Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDtoGregorian(Double CJD, Int *year, Int *month, Double *day);

/** Translates a Chronological Julian Day to a Gregorian calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void   CJDtoGregorianA(Double const *CJD, Double *date);

/** Translates a Chronological Julian Day to Gregorian calendar
    date text, consisting of the (fractional) day number, the full
    month name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJD the Chronological Julian Day to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
char * CJDtoGregorianS(Double CJD);

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
void   CJDtoGregorianSA(Double const *CJD, char **date);

/** Translates a Gregorian calendar date to a Chronological Julian Day Number.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
Int    GregoriantoCJDN(Int year, Int month, Int day);

/** Translates a Gregorian calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL.
 */
void   GregoriantoCJDNA(Int const *date, Int *CJDN);

/** Translates a Gregorian calendar date in text form to a
    Chronological Julian Day Number.

    \param[in] date a pointer to the text containing the calendar
    date.  Must not be NULL!  The date text must consist of a day
    number, a full month name (in English), and a year number,
    separated by whitespace.  The month name is matched without
    regard to case distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal date was recognized in the input.
 */
Int    GregorianStoCJDN(char const *date);

/** Translates a Gregorian calendar date in text form to a
    Chronological Julian Day Number.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of a day number, a
    full month name (in English), and a year number, separated by
    whitespace.  The month name is matched without regard to case
    distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal date was recognized in the input.
 */
void   GregorianStoCJDNA(char * const *date, Int *CJDN);

/** Translates a Gregorian calendar date to a Chronological Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day
 */
Double GregoriantoCJD(Int year, Int month, Double day);

/** Translates a Gregorian calendar date to a Chronological Julian Day.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void   GregoriantoCJDA(Double const *date, Double *CJD);

/** Translates a Gregorian calendar date in text form to a
    Chronological Julian Day.

    \param[in] date a pointer to the text containing the calendar
    date.  Must not be NULL!  The date text must consist of a day
    number, a full month name (in English), and a year number,
    separated by whitespace.  The month name is matched without
    regard to case distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day, or 0 if no
    legal date was recognized in the input.
 */
Double GregorianStoCJD(char const *date);

/** Translates a Gregorian calendar date in text form to a
    Chronological Julian Day.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of a day number, a
    full month name (in English), and a year number, separated by
    whitespace.  The month name is matched without regard to case
    distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day, or 0 if no
    legal date was recognized in the input.
 */
void   GregorianStoCJDA(char * const *date, Double *CJD);

/* JULIAN CALENDAR */

/**    
    Translates a Chronological Julian Day Number to a Julian
    calendar date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDNtoJulian(Int CJDN, Int *year, Int *month, Int *day);

/** Translates a Chronological Julian Day Number to a Julian
    calendar date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!
    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, date).
 */
void   CJDNtoJulianA(Int const *CJDN, Int *date);

/** Translates a Chronological Julian Day Number to Julian
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN the Chronological Julian Day number to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
char * CJDNtoJulianS(Int CJDN);

/** Translates a Chronological Julian Day Number to Julian
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN a pointer to the Chronological Julian Day number
    to translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void   CJDNtoJulianSA(Int const *CJDN, char **date);

/** Translates a Chronological Julian Day to a Julian
    date.

    \param[in] CJD the Chronological Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDtoJulian(Double CJD, Int *year, Int *month, Double *day);

/** Translates a Chronological Julian Day to a Julian calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void   CJDtoJulianA(Double const *CJD, Double *date);

/** Translates a Chronological Julian Day to Julian calendar
    date text, consisting of the (fractional) day number, the full
    month name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJD the Chronological Julian Day to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
char * CJDtoJulianS(Double CJD);

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
void   CJDtoJulianSA(Double const *CJD, char **date);

/** Translates a Julian calendar date to a Chronological Julian Day Number.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
Int    JuliantoCJDN(Int year, Int month, Int day);

/** Translates a Julian calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL.
 */
void   JuliantoCJDNA(Int const *date, Int *CJDN);

/** Translates a Julian calendar date in text form to a
    Chronological Julian Day Number.

    \param[in] date a pointer to the text containing the calendar
    date.  Must not be NULL!  The date text must consist of a day
    number, a full month name (in English), and a year number,
    separated by whitespace.  The month name is matched without
    regard to case distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal date was recognized in the input.
 */
Int    JulianStoCJDN(char const *date);

/** Translates a Julian calendar date in text form to a
    Chronological Julian Day Number.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of a day number, a
    full month name (in English), and a year number, separated by
    whitespace.  The month name is matched without regard to case
    distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal date was recognized in the input.
 */
void   JulianStoCJDNA(char * const *date, Int *CJDN);

/** Translates a Julian calendar date to a Chronological Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day
 */
Double JuliantoCJD(Int year, Int month, Double day);

/** Translates a Julian calendar date to a Chronological Julian Day.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void   JuliantoCJDA(Double const *date, Double *CJD);

/** Translates a Julian calendar date in text form to a
    Chronological Julian Day.

    \param[in] date a pointer to the text containing the calendar
    date.  Must not be NULL!  The date text must consist of a day
    number, a full month name (in English), and a year number,
    separated by whitespace.  The month name is matched without
    regard to case distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day, or 0 if no
    legal date was recognized in the input.
 */
Double JulianStoCJD(char const *date);

/** Translates a Julian calendar date in text form to a
    Chronological Julian Day.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of a day number, a
    full month name (in English), and a year number, separated by
    whitespace.  The month name is matched without regard to case
    distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day, or 0 if no
    legal date was recognized in the input.
 */
void   JulianStoCJDA(char * const *date, Double *CJD);

/* COMMON CALENDAR */

/**    
    Translates a Chronological Julian Day Number to a Common
    calendar date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDNtoCommon(Int CJDN, Int *year, Int *month, Int *day);

/** Translates a Chronological Julian Day Number to a Common
    calendar date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!
    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, date).
 */
void   CJDNtoCommonA(Int const *CJDN, Int *date);

/** Translates a Chronological Julian Day Number to Common
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN the Chronological Julian Day number to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
char * CJDNtoCommonS(Int CJDN);

/** Translates a Chronological Julian Day Number to Common
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN a pointer to the Chronological Julian Day number
    to translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void   CJDNtoCommonSA(Int const *CJDN, char **date);

/** Translates a Chronological Julian Day to a Common
    date.

    \param[in] CJD the Chronological Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDtoCommon(Double CJD, Int *year, Int *month, Double *day);

/** Translates a Chronological Julian Day to a Common calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void   CJDtoCommonA(Double const *CJD, Double *date);

/** Translates a Chronological Julian Day to Common calendar
    date text, consisting of the (fractional) day number, the full
    month name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJD the Chronological Julian Day to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
char * CJDtoCommonS(Double CJD);

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
void   CJDtoCommonSA(Double const *CJD, char **date);

/** Translates a Common calendar date to a Chronological Julian Day Number.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
Int    CommontoCJDN(Int year, Int month, Int day);

/** Translates a Common calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL.
 */
void   CommontoCJDNA(Int const *date, Int *CJDN);

/** Translates a Common calendar date in text form to a
    Chronological Julian Day Number.

    \param[in] date a pointer to the text containing the calendar
    date.  Must not be NULL!  The date text must consist of a day
    number, a full month name (in English), and a year number,
    separated by whitespace.  The month name is matched without
    regard to case distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal date was recognized in the input.
 */
Int    CommonStoCJDN(char const *date);

/** Translates a Common calendar date in text form to a
    Chronological Julian Day Number.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of a day number, a
    full month name (in English), and a year number, separated by
    whitespace.  The month name is matched without regard to case
    distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal date was recognized in the input.
 */
void   CommonStoCJDNA(char * const *date, Int *CJDN);

/** Translates a Common calendar date to a Chronological Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day
 */
Double CommontoCJD(Int year, Int month, Double day);

/** Translates a Common calendar date to a Chronological Julian Day.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void   CommontoCJDA(Double const *date, Double *CJD);

/** Translates a Common calendar date in text form to a
    Chronological Julian Day.

    \param[in] date a pointer to the text containing the calendar
    date.  Must not be NULL!  The date text must consist of a day
    number, a full month name (in English), and a year number,
    separated by whitespace.  The month name is matched without
    regard to case distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day, or 0 if no
    legal date was recognized in the input.
 */
Double CommonStoCJD(char const *date);

/** Translates a Common calendar date in text form to a
    Chronological Julian Day.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of a day number, a
    full month name (in English), and a year number, separated by
    whitespace.  The month name is matched without regard to case
    distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day, or 0 if no
    legal date was recognized in the input.
 */
void   CommonStoCJDA(char * const *date, Double *CJD);

/* HEBREW CALENDAR */

/**    
    Translates a Chronological Julian Day Number to a Hebrew
    calendar date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDNtoHebrew(Int CJDN, Int *year, Int *month, Int *day);

/** Translates a Chronological Julian Day Number to a Hebrew
    calendar date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!
    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, date).
 */
void   CJDNtoHebrewA(Int const *CJDN, Int *date);

/** Translates a Chronological Julian Day Number to Hebrew
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN the Chronological Julian Day number to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
char * CJDNtoHebrewS(Int CJDN);

/** Translates a Chronological Julian Day Number to Hebrew
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN a pointer to the Chronological Julian Day number
    to translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void   CJDNtoHebrewSA(Int const *CJDN, char **date);

/** Translates a Chronological Julian Day to a Hebrew
    date.

    \param[in] CJD the Chronological Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDtoHebrew(Double CJD, Int *year, Int *month, Double *day);

/** Translates a Chronological Julian Day to a Hebrew calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void   CJDtoHebrewA(Double const *CJD, Double *date);

/** Translates a Chronological Julian Day to Hebrew calendar
    date text, consisting of the (fractional) day number, the full
    month name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJD the Chronological Julian Day to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
char * CJDtoHebrewS(Double CJD);

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
void   CJDtoHebrewSA(Double const *CJD, char **date);

/** Translates a Hebrew calendar date to a Chronological Julian Day Number.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
Int    HebrewtoCJDN(Int year, Int month, Int day);

/** Translates a Hebrew calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL.
 */
void   HebrewtoCJDNA(Int const *date, Int *CJDN);

/** Translates a Hebrew calendar date in text form to a
    Chronological Julian Day Number.

    \param[in] date a pointer to the text containing the calendar
    date.  Must not be NULL!  The date text must consist of a day
    number, a full month name (in English), and a year number,
    separated by whitespace.  The month name is matched without
    regard to case distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal date was recognized in the input.
 */
Int    HebrewStoCJDN(char const *date);

/** Translates a Hebrew calendar date in text form to a
    Chronological Julian Day Number.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of a day number, a
    full month name (in English), and a year number, separated by
    whitespace.  The month name is matched without regard to case
    distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal date was recognized in the input.
 */
void   HebrewStoCJDNA(char * const *date, Int *CJDN);

/** Translates a Hebrew calendar date to a Chronological Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day
 */
Double HebrewtoCJD(Int year, Int month, Double day);

/** Translates a Hebrew calendar date to a Chronological Julian Day.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void   HebrewtoCJDA(Double const *date, Double *CJD);

/** Translates a Hebrew calendar date in text form to a
    Chronological Julian Day.

    \param[in] date a pointer to the text containing the calendar
    date.  Must not be NULL!  The date text must consist of a day
    number, a full month name (in English), and a year number,
    separated by whitespace.  The month name is matched without
    regard to case distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day, or 0 if no
    legal date was recognized in the input.
 */
Double HebrewStoCJD(char const *date);

/** Translates a Hebrew calendar date in text form to a
    Chronological Julian Day.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of a day number, a
    full month name (in English), and a year number, separated by
    whitespace.  The month name is matched without regard to case
    distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day, or 0 if no
    legal date was recognized in the input.
 */
void   HebrewStoCJDA(char * const *date, Double *CJD);

/* ISLAMIC CALENDAR */

/**    
    Translates a Chronological Julian Day Number to a Islamic
    calendar date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDNtoIslamic(Int CJDN, Int *year, Int *month, Int *day);

/** Translates a Chronological Julian Day Number to a Islamic
    calendar date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!
    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, date).
 */
void   CJDNtoIslamicA(Int const *CJDN, Int *date);

/** Translates a Chronological Julian Day Number to Islamic
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN the Chronological Julian Day number to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
char * CJDNtoIslamicS(Int CJDN);

/** Translates a Chronological Julian Day Number to Islamic
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN a pointer to the Chronological Julian Day number
    to translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void   CJDNtoIslamicSA(Int const *CJDN, char **date);

/** Translates a Chronological Julian Day to a Islamic
    date.

    \param[in] CJD the Chronological Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDtoIslamic(Double CJD, Int *year, Int *month, Double *day);

/** Translates a Chronological Julian Day to a Islamic calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void   CJDtoIslamicA(Double const *CJD, Double *date);

/** Translates a Chronological Julian Day to Islamic calendar
    date text, consisting of the (fractional) day number, the full
    month name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJD the Chronological Julian Day to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
char * CJDtoIslamicS(Double CJD);

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
void   CJDtoIslamicSA(Double const *CJD, char **date);

/** Translates a Islamic calendar date to a Chronological Julian Day Number.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
Int    IslamictoCJDN(Int year, Int month, Int day);

/** Translates a Islamic calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL.
 */
void   IslamictoCJDNA(Int const *date, Int *CJDN);

/** Translates a Islamic calendar date in text form to a
    Chronological Julian Day Number.

    \param[in] date a pointer to the text containing the calendar
    date.  Must not be NULL!  The date text must consist of a day
    number, a full month name (in English), and a year number,
    separated by whitespace.  The month name is matched without
    regard to case distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal date was recognized in the input.
 */
Int    IslamicStoCJDN(char const *date);

/** Translates a Islamic calendar date in text form to a
    Chronological Julian Day Number.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of a day number, a
    full month name (in English), and a year number, separated by
    whitespace.  The month name is matched without regard to case
    distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal date was recognized in the input.
 */
void   IslamicStoCJDNA(char * const *date, Int *CJDN);

/** Translates a Islamic calendar date to a Chronological Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day
 */
Double IslamictoCJD(Int year, Int month, Double day);

/** Translates a Islamic calendar date to a Chronological Julian Day.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void   IslamictoCJDA(Double const *date, Double *CJD);

/** Translates a Islamic calendar date in text form to a
    Chronological Julian Day.

    \param[in] date a pointer to the text containing the calendar
    date.  Must not be NULL!  The date text must consist of a day
    number, a full month name (in English), and a year number,
    separated by whitespace.  The month name is matched without
    regard to case distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day, or 0 if no
    legal date was recognized in the input.
 */
Double IslamicStoCJD(char const *date);

/** Translates a Islamic calendar date in text form to a
    Chronological Julian Day.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of a day number, a
    full month name (in English), and a year number, separated by
    whitespace.  The month name is matched without regard to case
    distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day, or 0 if no
    legal date was recognized in the input.
 */
void   IslamicStoCJDA(char * const *date, Double *CJD);

/* EGYPTIAN CALENDAR */

/**    
    Translates a Chronological Julian Day Number to a Egyptian
    calendar date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDNtoEgyptian(Int CJDN, Int *year, Int *month, Int *day);

/** Translates a Chronological Julian Day Number to a Egyptian
    calendar date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!
    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, date).
 */
void   CJDNtoEgyptianA(Int const *CJDN, Int *date);

/** Translates a Chronological Julian Day Number to Egyptian
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN the Chronological Julian Day number to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
char * CJDNtoEgyptianS(Int CJDN);

/** Translates a Chronological Julian Day Number to Egyptian
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN a pointer to the Chronological Julian Day number
    to translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void   CJDNtoEgyptianSA(Int const *CJDN, char **date);

/** Translates a Chronological Julian Day to a Egyptian
    date.

    \param[in] CJD the Chronological Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDtoEgyptian(Double CJD, Int *year, Int *month, Double *day);

/** Translates a Chronological Julian Day to a Egyptian calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void   CJDtoEgyptianA(Double const *CJD, Double *date);

/** Translates a Chronological Julian Day to Egyptian calendar
    date text, consisting of the (fractional) day number, the full
    month name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJD the Chronological Julian Day to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
char * CJDtoEgyptianS(Double CJD);

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
void   CJDtoEgyptianSA(Double const *CJD, char **date);

/** Translates a Egyptian calendar date to a Chronological Julian Day Number.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
Int    EgyptiantoCJDN(Int year, Int month, Int day);

/** Translates a Egyptian calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL.
 */
void   EgyptiantoCJDNA(Int const *date, Int *CJDN);

/** Translates a Egyptian calendar date in text form to a
    Chronological Julian Day Number.

    \param[in] date a pointer to the text containing the calendar
    date.  Must not be NULL!  The date text must consist of a day
    number, a full month name (in English), and a year number,
    separated by whitespace.  The month name is matched without
    regard to case distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal date was recognized in the input.
 */
Int    EgyptianStoCJDN(char const *date);

/** Translates a Egyptian calendar date in text form to a
    Chronological Julian Day Number.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of a day number, a
    full month name (in English), and a year number, separated by
    whitespace.  The month name is matched without regard to case
    distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal date was recognized in the input.
 */
void   EgyptianStoCJDNA(char * const *date, Int *CJDN);

/** Translates a Egyptian calendar date to a Chronological Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day
 */
Double EgyptiantoCJD(Int year, Int month, Double day);

/** Translates a Egyptian calendar date to a Chronological Julian Day.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void   EgyptiantoCJDA(Double const *date, Double *CJD);

/** Translates a Egyptian calendar date in text form to a
    Chronological Julian Day.

    \param[in] date a pointer to the text containing the calendar
    date.  Must not be NULL!  The date text must consist of a day
    number, a full month name (in English), and a year number,
    separated by whitespace.  The month name is matched without
    regard to case distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day, or 0 if no
    legal date was recognized in the input.
 */
Double EgyptianStoCJD(char const *date);

/** Translates a Egyptian calendar date in text form to a
    Chronological Julian Day.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of a day number, a
    full month name (in English), and a year number, separated by
    whitespace.  The month name is matched without regard to case
    distinctions or non-alphanumerical characters.

    \return the corresponding Chronological Julian Day, or 0 if no
    legal date was recognized in the input.
 */
void   EgyptianStoCJDA(char * const *date, Double *CJD);

/* JULIAN DAY */

/** Translates Julian Day to Chronological Julian Day.

    \param[in] JD the Julian Day to translate

    \return the corresponding Chronological Julian Day
 */
Double JDtoCJD(Double JD);

/** Translates a Julian Day to a Chronological Julian Day for the
    current time zone.

    \param[in] JD a pointer to the Julian Day to translate.  Must not
    be NULL!

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void JDtoCJDA(Double const *JD, Double *CJD);

/** Translates Chronological Julian Day for the current time zone to
    Julian Day.
    
    \param[in] CJD the Chronological Julian Day to translate

    \return the corresponding Julian Day

 */
Double CJDtoJD(Double CJD);

/** Translates a Chronological Julian Day to a Julian Day.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] JD a pointer to the corresponding Julian Day.
    Must not be NULL!

 */
void CJDtoJDA(Double const *CJD, Double *JD);

/** Copies a Chronological Julian Day.

    \param[in] in a pointer to the Chronological Julian Day to copy.
    Must not be NULL!

    \param[out] out a pointer to the target.  Must not be NULL!

 */
void CJDNtoCJDNA(Int const *in, Int *out);

/** Copies a Chronological Julian Day Number.

    \param[in] in a pointer to the Chronological Julian Day Number to
    copy.  Must not be NULL!

    \param[out] out a pointer to the target.  Must not be NULL!

 */
void CJDtoCJDA(Double const *in, Double *out);

/* LUNAR CALENDAR */

/** Translates a Chronological Julian Day to a (fractional) count of
    lunar months.

    \param[in] CJD the Chronological Julian Day
    \return the corresponding lunar month
 */ 
Double CJDtoLunar(Double CJD);

/** Translates a Chronological Julian Day to a (fractional) count of
    lunar months.

    \param[in] CJD a pointer to the Chronological Julian Day.  Must
    not be NULL!

    \param[out] lunar a pointer to the corresponding lunar month.
    Must not be NULL!
 */ 
void CJDtoLunarA(Double const *CJD, Double *lunar);

/** Translates a (fractional) count of lunar months to a Chronological
    Julian Day.

    \param[in] lunar the lunar month
    \return the corresponding Chronological Julian Day
 */ 
Double LunartoCJD(Double lunar);

/** Translates a (fractional) count of lunar months to a Chronological
    Julian Day.

    \param[in] lunar a pointer to the lunar month.  Must not be NULL!

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */ 
void LunartoCJDA(Double const *lunar, Double *CJD);

/* MAYAN TZOLKIN/HAAB CALENDAR */

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
void CJDNtoMayan(Int CJDN, Int *trecena, Int *venteina,
                 Int *haab_day, Int *haab_month,
                 Int *year_trecena, Int *year_venteina);

/** Translates a Chronological Julian Day Number into a Mayan
    tzolkin/haab calendar date (from Tikal).

    \param[in] CJDN a pointer to the Chronological Julian Day Number
    to translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding Mayan date.  Must
    have at least 6 elements (trecena, venteina, haab day, haab month,
    trecena of first day of haab year, venteina of first day of haab
    year)!
 */
void CJDNtoMayanA(Int const *CJDN, Int *date);

/** Translates a Chronological Julian Day Number to Mayan calendar
    date text, consisting of the trecena number, the full venteina
    name, the haab day number, the full haab month name, the trecena
    number of the first day of the haab year, and the full venteina
    name of the first day of the haab year, all separated by single
    whitespaces, and with the first-day-of-the-year part enclosed in
    parentheses.

    \param[in] CJDN the Chronological Julian Day Number to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
char *CJDNtoMayanS(Int CJDN);

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
void CJDNtoMayanSA(Int const *CJDN, char **date);

/** Translates a Chronological Julian Day into a Mayan tzolkin/haab
    calendar date (from Tikal).

    \param[in] CJD the Chronological Julian Day to translate

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
void CJDtoMayan(Double CJD, Int *trecena, Int *venteina,
                Double *haab_day, Int *haab_month,
                Int *year_trecena, Int *year_venteina);

/** Translates a Chronological Julian Day into a Mayan tzolkin/haab
    calendar date (from Tikal).

    \param[in] CJD a pointer to the Chronological Julian Day Number
    to translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding Mayan date.  Must
    have at least 6 elements (trecena, venteina, haab day, haab month,
    trecena of first day of haab year, venteina of first day of haab
    year)!
 */
void CJDtoMayanA(Double const *CJD, Double *date);

/** Translates a Chronological Julian Day to Mayan calendar date text,
    consisting of the trecena number, the full venteina name, the haab
    day number, the full haab month name, the trecena number of the
    first day of the haab year, and the full venteina name of the
    first day of the haab year, all separated by single whitespaces,
    and with the first-day-of-the-year part enclosed in parentheses.

    \param[in] CJD the Chronological Julian Day to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
char *CJDtoMayanS(Double CJD);

/** Translates a Chronological Julian Day to Mayan calendar date text,
    consisting of the trecena number, the full venteina name, the haab
    day number, the full haab month name, the trecena number of the
    first day of the haab year, and the full venteina name of the
    first day of the haab year, all separated by single whitespaces,
    and with the first-day-of-the-year part enclosed in parentheses.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
void CJDtoMayanSA(Double const *CJD, char **date);

/** Translates a Mayan calendar date to a corresponding Chronological
    Julian Day Number on or before a specified Chronological Julian
    Day Number.

    \param[in] trecena the trecena number of the Mayan calendar date.
    If it is 0, then \p trecena and \p venteina are ignored.

    \param[in] venteina the venteina number of the Mayan calendar
    date.  If it is 0, then \p trecena and \p venteina are ignored.

    \param[in] haab_day the haab day number of the Mayan calendar
    date.

    \param[in] haab_month the haab month number of the Mayan calendar
    date.  If it is 0, then \p haab_day and \p haab_month are ignored.

    \param[in] year_trecena the trecena number that identifies the
    Mayan year.  If it is 0, then \p year_trecena and \p year_venteina
    are ignored.

    \param[in] year_venteina the venteina number that identifies the
    Mayan year.  If it is 0, then \p year_trecena and \p year_venteina
    are ignored.

    \param[in] CJDN_upper the upper limit to the Chronological Julian
    Day Number that may be returned.

    \return the greatest Chronological Julian Day Number not exceeding
    \p CJDN_upper that has the specified Mayan calendar date, or 0 if
    an impossible Mayan calendar date is specified.
 */
Int MayantoCJDN(Int trecena, Int venteina, Int haab_day, Int haab_month,
                Int year_trecena, Int year_venteina, Int CJDN_upper);

/** Translates a Mayan calendar date to a corresponding Chronological
    Julian Day Number on or before a specified Chronological Julian
    Day Number.

    \param[in] *date a pointer to the Mayan calendar date components.
    Must have at least 7 elements!  The elements of \p date are: (1)
    the trecena number of the Mayan calendar date.  If it is 0, then
    the trecena and venteina numbers are ignored. (2) the venteina
    number of the Mayan calendar date.  If it is 0, then the trecena
    and venteina numbers are ignored. (3) the haab day number of the
    Mayan calendar date. (4) the haab month number of the Mayan
    calendar date.  If it is 0, then the haab day and haab month
    numbers are ignored. (5) the trecena number that identifies the
    Mayan year.  If it is 0, then the_trecena and_venteina numbers
    that identify the year are ignored. (6) the venteina number that
    identifies the Mayan year.  If it is 0, then the trecena and
    venteina numbers that identify the year are ignored. (7) the upper
    limit to the Chronological Julian Day Number that may be returned.

    \param[out] CJDN a pointer to the greatest Chronological Julian
    Day Number not exceeding the specified upper limit that has the
    specified Mayan calendar date components, or 0 if an impossible
    Mayan calendar date is specified.
 */
void MayantoCJDNA(Int const *date, Int *CJDN);

/** Translates a Mayan tzolkin/haab calendar date in text form to a
    Chronological Julian Day Number.

    \param[in] date a pointer to the calendar date text.  Must not be
    NULL!  The date text must consist of the integer trecena number,
    the full venteina name, the integer haab day number, the full haab
    month name, the integer trecena number of the first day of the
    haab year, the full venteina name of the first day of the haab
    year, and an integer Chronological Julian Day Number that limits
    the resulting CJDN.  The returned CJDN is the greatest one that
    matches the tzolkin/haab dates and that is not greater than the
    limiting CJDN.  The venteina and haab month names are matched
    without regard to case distinctions or non-alphanumerical
    characters.  All but one of the components (tzolkin - haab -
    first-day-of-the-year tzolkin - limiting CJDN) may be omitted.
    The order of the tzolkin and haab components is free, but the
    order within the components (e.g., haab day and haab month) must
    be as stated before, and the first-day-of-the-year tzolkin always
    comes after the regular tzolkin.  If the limiting CJDN is omitted,
    then today's CJDN is used for it.

    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal date was recognized in the input.
 */
Int MayanStoCJDN(char const *date);

/** Translates a Mayan tzolkin/haab calendar date in text form to a
    Chronological Julian Day Number.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of the trecena
    number, the full venteina name, the haab day number, the full haab
    month name, the trecena number of the first day of the haab year,
    the full venteina name of the first day of the haab year, and a
    Chronological Julian Day Number that limits the resulting CJDN.
    The returned CJDN is the greatest one that matches the
    tzolkin/haab dates and that is not greater than the limiting CJDN.
    The venteina and haab month names are matched without regard to
    case distinctions or non-alphanumerical characters.  All but one
    of the components (tzolkin - haab - first-day-of-the-year tzolkin
    - limiting CJDN) may be omitted.  The order of the tzolkin and
    haab components is free, but the order within the components
    (e.g., haab day and haab month) must be as stated before, and the
    first-day-of-the-year tzolkin always comes after the regular
    tzolkin.  If the limiting CJDN is omitted, then today's CJDN is
    used for it.

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  It is set to 0 if no legal date was recognized
    in the input.
 */
void MayanStoCJDNA(char * const *date, Int *CJDN);

/** Translates a Mayan calendar date to a corresponding Chronological
    Julian Day on or before a specified Chronological Julian Day
    Number.

    \param[in] trecena the trecena number of the Mayan calendar date.
    If it is 0, then \p trecena and \p venteina are ignored.

    \param[in] venteina the venteina number of the Mayan calendar
    date.  If it is 0, then \p trecena and \p venteina are ignored.

    \param[in] haab_day the fractional haab day number of the Mayan
    calendar date.

    \param[in] haab_month the haab month number of the Mayan calendar
    date.  If it is 0, then \p haab_day and \p haab_month are ignored.

    \param[in] year_trecena the trecena number that identifies the
    Mayan year.  If it is 0, then \p year_trecena and \p year_venteina
    are ignored.

    \param[in] year_venteina the venteina number that identifies the
    Mayan year.  If it is 0, then \p year_trecena and \p year_venteina
    are ignored.

    \param[in] CJDN_upper the upper limit to the Chronological Julian
    Day Number that may be returned.

    \return the greatest Chronological Julian Day not exceeding \p
    CJDN_upper that has the specified Mayan calendar date, or 0 if an
    impossible Mayan calendar date is specified.
 */
Double MayantoCJD(Int trecena, Int venteina, Double haab_day, Int haab_month,
                  Int year_trecena, Int year_venteina, Int CJDN_upper);

/** Translates a Mayan calendar date to a corresponding Chronological
    Julian Day on or before a specified Chronological Julian Day
    Number.

    \param[in] *date a pointer to the Mayan calendar date components.
    Must have at least 7 elements!  The elements of \p date are: (1)
    the trecena number of the Mayan calendar date.  If it is 0, then
    the trecena and venteina numbers are ignored. (2) the venteina
    number of the Mayan calendar date.  If it is 0, then the trecena
    and venteina numbers are ignored. (3) the haab day number of the
    Mayan calendar date. (4) the haab month number of the Mayan
    calendar date.  If it is 0, then the haab day and haab month
    numbers are ignored. (5) the trecena number that identifies the
    Mayan year.  If it is 0, then the_trecena and_venteina numbers
    that identify the year are ignored. (6) the venteina number that
    identifies the Mayan year.  If it is 0, then the trecena and
    venteina numbers that identify the year are ignored. (7) the upper
    limit to the Chronological Julian Day Number that may be returned.
    The fractional part of the haab day number is taken into account;
    fractional parts of the other components are ignored.

    \param[out] CJD a pointer to the greatest Chronological Julian Day
    not exceeding the specified upper limit that has the specified
    Mayan calendar date components, or 0 if an impossible Mayan
    calendar date is specified.
 */
void MayantoCJDA(Double const *date, Double *CJD);

/** Translates a Mayan tzolkin/haab calendar date in text form to a
    Chronological Julian Day.

    \param[in] date a pointer to the calendar date text.  Must not be
    NULL!  The date text must consist of the fractional trecena
    number, the full venteina name, the fractional haab day number,
    the full haab month name, the integer trecena number of the first
    day of the haab year, the full venteina name of the first day of
    the haab year, and a fractional Chronological Julian Day that
    limits the resulting CJD.  The returned CJD is the greatest one
    that matches the tzolkin/haab dates and that is not greater than
    the limiting CJD.  The venteina and haab month names are matched
    without regard to case distinctions or non-alphanumerical
    characters.  All but one of the components (tzolkin - haab -
    first-day-of-the-year tzolkin - limiting CJD) may be omitted.  The
    order of the tzolkin and haab components is free, but the order
    within the components (e.g., haab day and haab month) must be as
    stated before, and the first-day-of-the-year tzolkin always comes
    after the regular tzolkin.  If the limiting CJD is omitted, then
    today's CJD is used for it.

    \return the corresponding Chronological Julian Day, or 0 if no
    legal date was recognized in the input.
 */
Double MayanStoCJD(char const *date);

/** Translates a Mayan tzolkin/haab calendar date in text form to a
    Chronological Julian Day.

    \param[in] date a pointer to a pointer to the calendar date text.
    Must not be NULL!  The date text must consist of the trecena
    number, the full venteina name, the haab day number, the full haab
    month name, the trecena number of the first day of the haab year,
    the full venteina name of the first day of the haab year, and a
    Chronological Julian Day that limits the resulting CJD.  The
    returned CJD is the greatest one that matches the tzolkin/haab
    dates and that is not greater than the limiting CJD.  The venteina
    and haab month names are matched without regard to case
    distinctions or non-alphanumerical characters.  All but one of the
    components (tzolkin - haab - first-day-of-the-year tzolkin -
    limiting CJD) may be omitted.  The order of the tzolkin and haab
    components is free, but the order within the components (e.g.,
    haab day and haab month) must be as stated before, and the
    first-day-of-the-year tzolkin always comes after the regular
    tzolkin.  If the limiting CJD is omitted, then today's CJD is used
    for it.

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  It is set to 0 if no legal date was recognized in the
    input.
 */
void MayanStoCJDA(char * const *date, Double *CJD);

/* MAYAN LONG COUNT CALENDAR */

/** Translates a Chronological Julian Day Number to a Maya Long Count.

    \param[in] CJDN pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!

    \param[out] baktun the corresponding number of baktun.

    \param[out] katun the corresponding number of katun, in the range
    from 0 through 19.

    \param[out] tun the corresponding number of tun, in the range from
    0 through 19.

    \param[out] uinal the corresponding number of uinal, in the range
    from 0 through 17.

    \param[out] kin the corresponding number of kin, in the range from
    0 through 19.
 */
void CJDNtoLongCount(Int CJDN, Int *baktun, Int *katun, Int *tun,
                     Int *uinal, Int *kin);

/** Translates a Chronological Julian Day Number to a Maya Long Count.

    \param[in] CJDN the Chronological Julian Day Number to translate

    \param[out] date pointer to the corresponding Long Count,
    which must have at least 5 elements (from most significant to
    least significant).  The first element [baktun] is not restricted.
    The last 4 elements are restricted to ranges from 0 through,
    respectively, 19 [katun], 19 [tun], 17 [uinal], and 19 [kin].
 */
void CJDNtoLongCountA(Int const *CJDN, Int *date);

/** Translates a Chronological Julian Day Number to Mayan Long Count
    calendar date text, consisting of the five numbers in decreasing
    order of significance, separated by periods.

    \param[in] CJDN the Chronological Julian Day number to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
char *CJDNtoLongCountS(Int CJDN);

/** Translates a Chronological Julian Day Number to Mayan Long Count
    calendar date text, consisting of the five numbers in decreasing
    order of significance, separated by periods.

    \param[in] CJDN a pointer to the Chronological Julian Day number
    to translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
void CJDNtoLongCountSA(Int const *CJDN, char **date);

/** Translates a Chronological Julian Day to a Maya Long Count.

    \param[in] CJD pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] baktun the corresponding number of baktun.

    \param[out] katun the corresponding number of katun, in the range
    from 0 through 19.

    \param[out] tun the corresponding number of tun, in the range from
    0 through 19.

    \param[out] uinal the corresponding number of uinal, in the range
    from 0 through 17.

    \param[out] kin the corresponding number of kin, in the range from
    0 through 19.
 */
void CJDtoLongCount(Double CJD, Int *baktun, Int *katun, Int *tun, Int *uinal,
                    Double *kin);

/** Translates a Chronological Julian Day to a Maya Long Count.

    \param[in] CJD the Chronological Julian Day to translate.

    \param[out] date a pointer to the corresponding Long Count
    numbers, in decreasing order of significance.  Must have at least
    5 elements!
 */
void CJDtoLongCountA(Double const *CJD, Double *date);

/** Translates a Chronological Julian Day to Mayan Long Count calendar
    date text, consisting of the five numbers in decreasing order of
    significance, separated by periods.  The fractional part of the
    Chronological Julian Day is discarded by rounding down to the
    nearest integer.

    \param[in] CJD the Chronological Julian Day to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
char *CJDtoLongCountS(Double CJD);

/** Translates a Chronological Julian Day to Mayan Long Count calendar
    date text, consisting of the five numbers in decreasing order of
    significance, separated by periods.  The fractional part of the
    Chronological Julian Day is discarded by rounding down to the
    nearest integer.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
void CJDtoLongCountSA(Double const *CJD, char **date);

/** Translates a Maya Long Count to a Chronological Julian Day Number.

    \param[in] baktun the baktun number.

    \param[in] katun the katun number.

    \param[in] tun the tun number.

    \param[in] uinal the uinal number.

    \param[in] kin the kin number.

    \return the corresponding Chronological Julian Day Number.
 */
Int LongCounttoCJDN(Int baktun, Int katun, Int tun, Int uinal, Int kin);

/** Translates a Maya Long Count to a Chronological Julian Day Number.

    \param[in] date a pointer to the Maya Long Count, which must
    have at least 5 elements (from most significant to least
    significant).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL!
 */
void LongCounttoCJDNA(Int const *date, Int *CJDN);

/** Translates a Maya Long Count text to a Chronological Julian Day
    Number.

    \param[in] date the Long Count text, which must mention up to 5
    Long Count numbers in decreasing order of significance, separated
    by non-numerical characters.

    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal Long Count was recognized.
  */
Int LongCountStoCJDN(char const *date);

/** Translates a Maya Long Count text to a Chronological Julian Day
    Number.

    \param[in] date the Long Count text, which must mention up to 5
    Long Count numbers in decreasing order of significance, separated
    by non-numerical characters.

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  If no legal Long Count was recognized, then 0
    is returned.
  */
void LongCountStoCJDNA(char * const *date, Int *CJDN);

/** Translates a Maya Long Count to a Chronological Julian Day.

    \param[in] baktun the baktun number.

    \param[in] katun the katun number.

    \param[in] tun the tun number.

    \param[in] uinal the uinal number.

    \param[in] kin the kin number.

    \return the corresponding Chronological Julian Day.
 */
Double LongCounttoCJD(Int baktun, Int katun, Int tun, Int uinal, Double kin);

/** Translates a Maya Long Count to a Chronological Julian Day.

    \param[in] date a pointer to the Maya Long Count, which must have
    5 elements (from most significant to least significant).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void LongCounttoCJDA(Double const *date, Double *CJD);

/** Translates a Maya Long Count text to a Chronological Julian Day.

    \param[in] date the Long Count text, which must mention up to 5
    Long Count numbers in decreasing order of significance, separated
    by non-numerical characters.  NOTE: only integer numbers are
    recognized.

    \return the corresponding Chronological Julian Day, or 0 if no
    legal Long Count was recognized.
  */
Double LongCountStoCJD(char const *date);

/** Translates a Maya Long Count text to a Chronological Julian Day.

    \param[in] date the Long Count text, which must mention up to 5
    Long Count numbers in decreasing order of significance, separated
    by non-numerical characters.  NOTE: only integer numbers are
    recognized.

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  If no legal Long Count was recognized, then 0 is
    returned.
  */
void LongCountStoCJDA(char * const *date, Double *CJD);

#endif