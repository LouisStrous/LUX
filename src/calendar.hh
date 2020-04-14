/* This is file calendar.hh.

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
#ifndef CALENDAR_H
#define CALENDAR_H

#include "action.hh"
// HEADERS
#include <math.h> // for floor
// END HEADERS

/** \file
  Calendrical calculations.

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
double CJD_now(void);

/** Returns the current Chronological Julian Day Number based on the
    system's time zone.

    \return the current Chronological Julian Day Number
*/
int32_t CJDN_now(void);

// GREGORIAN CALENDAR

/**
    Translates a Chronological Julian Day Number to a Gregorian
    calendar date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDNtoGregorian(int32_t CJDN, int32_t *year, int32_t *month, int32_t *day);

/** Translates a Chronological Julian Day Number to a Gregorian
    calendar date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!
    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, date).
 */
void   CJDNtoGregorianA(int32_t const *CJDN, int32_t *date);

/** Translates a Chronological Julian Day Number to Gregorian
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN the Chronological Julian Day number to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
char * CJDNtoGregorianS(int32_t CJDN);

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
void   CJDNtoGregorianSA(int32_t const *CJDN, char **date);

/** Translates a Chronological Julian Day to a Gregorian
    date.

    \param[in] CJD the Chronological Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDtoGregorian(double CJD, int32_t *year, int32_t *month, double *day);

/** Translates a Chronological Julian Day to a Gregorian calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void   CJDtoGregorianA(double const *CJD, double *date);

/** Translates a Chronological Julian Day to Gregorian calendar
    date text, consisting of the (fractional) day number, the full
    month name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJD the Chronological Julian Day to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
char * CJDtoGregorianS(double CJD);

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
void   CJDtoGregorianSA(double const *CJD, char **date);

/** Translates a Gregorian calendar date to a Chronological Julian Day Number.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
int32_t    GregoriantoCJDN(int32_t year, int32_t month, int32_t day);

/** Translates a Gregorian calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL.
 */
void   GregoriantoCJDNA(int32_t const *date, int32_t *CJDN);

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
int32_t    GregorianStoCJDN(char const *date);

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
void   GregorianStoCJDNA(char * const *date, int32_t *CJDN);

/** Translates a Gregorian calendar date to a Chronological Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day
 */
double GregoriantoCJD(int32_t year, int32_t month, double day);

/** Translates a Gregorian calendar date to a Chronological Julian Day.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void   GregoriantoCJDA(double const *date, double *CJD);

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
double GregorianStoCJD(char const *date);

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
void   GregorianStoCJDA(char * const *date, double *CJD);

// JULIAN CALENDAR

/**
    Translates a Chronological Julian Day Number to a Julian
    calendar date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDNtoJulian(int32_t CJDN, int32_t *year, int32_t *month, int32_t *day);

/** Translates a Chronological Julian Day Number to a Julian
    calendar date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!
    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, date).
 */
void   CJDNtoJulianA(int32_t const *CJDN, int32_t *date);

/** Translates a Chronological Julian Day Number to Julian
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN the Chronological Julian Day number to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
char * CJDNtoJulianS(int32_t CJDN);

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
void   CJDNtoJulianSA(int32_t const *CJDN, char **date);

/** Translates a Chronological Julian Day to a Julian
    date.

    \param[in] CJD the Chronological Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDtoJulian(double CJD, int32_t *year, int32_t *month, double *day);

/** Translates a Chronological Julian Day to a Julian calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void   CJDtoJulianA(double const *CJD, double *date);

/** Translates a Chronological Julian Day to Julian calendar
    date text, consisting of the (fractional) day number, the full
    month name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJD the Chronological Julian Day to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
char * CJDtoJulianS(double CJD);

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
void   CJDtoJulianSA(double const *CJD, char **date);

/** Translates a Julian calendar date to a Chronological Julian Day Number.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
int32_t    JuliantoCJDN(int32_t year, int32_t month, int32_t day);

/** Translates a Julian calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL.
 */
void   JuliantoCJDNA(int32_t const *date, int32_t *CJDN);

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
int32_t    JulianStoCJDN(char const *date);

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
void   JulianStoCJDNA(char * const *date, int32_t *CJDN);

/** Translates a Julian calendar date to a Chronological Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day
 */
double JuliantoCJD(int32_t year, int32_t month, double day);

/** Translates a Julian calendar date to a Chronological Julian Day.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void   JuliantoCJDA(double const *date, double *CJD);

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
double JulianStoCJD(char const *date);

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
void   JulianStoCJDA(char * const *date, double *CJD);

// COMMON CALENDAR

/**
    Translates a Chronological Julian Day Number to a Common
    calendar date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDNtoCommon(int32_t CJDN, int32_t *year, int32_t *month, int32_t *day);

/** Translates a Chronological Julian Day Number to a Common
    calendar date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!
    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, date).
 */
void   CJDNtoCommonA(int32_t const *CJDN, int32_t *date);

/** Translates a Chronological Julian Day Number to Common
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN the Chronological Julian Day number to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
char * CJDNtoCommonS(int32_t CJDN);

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
void   CJDNtoCommonSA(int32_t const *CJDN, char **date);

/** Translates a Chronological Julian Day to a Common
    date.

    \param[in] CJD the Chronological Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDtoCommon(double CJD, int32_t *year, int32_t *month, double *day);

/** Translates a Chronological Julian Day to a Common calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void   CJDtoCommonA(double const *CJD, double *date);

/** Translates a Chronological Julian Day to Common calendar
    date text, consisting of the (fractional) day number, the full
    month name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJD the Chronological Julian Day to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
char * CJDtoCommonS(double CJD);

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
void   CJDtoCommonSA(double const *CJD, char **date);

/** Translates a Common calendar date to a Chronological Julian Day Number.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
int32_t    CommontoCJDN(int32_t year, int32_t month, int32_t day);

/** Translates a Common calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL.
 */
void   CommontoCJDNA(int32_t const *date, int32_t *CJDN);

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
int32_t    CommonStoCJDN(char const *date);

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
void   CommonStoCJDNA(char * const *date, int32_t *CJDN);

/** Translates a Common calendar date to a Chronological Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day
 */
double CommontoCJD(int32_t year, int32_t month, double day);

/** Translates a Common calendar date to a Chronological Julian Day.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void   CommontoCJDA(double const *date, double *CJD);

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
double CommonStoCJD(char const *date);

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
void   CommonStoCJDA(char * const *date, double *CJD);

// HEBREW CALENDAR

/**
    Translates a Chronological Julian Day Number to a Hebrew
    calendar date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDNtoHebrew(int32_t CJDN, int32_t *year, int32_t *month, int32_t *day);

/** Translates a Chronological Julian Day Number to a Hebrew
    calendar date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!
    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, date).
 */
void   CJDNtoHebrewA(int32_t const *CJDN, int32_t *date);

/** Translates a Chronological Julian Day Number to Hebrew
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN the Chronological Julian Day number to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
char * CJDNtoHebrewS(int32_t CJDN);

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
void   CJDNtoHebrewSA(int32_t const *CJDN, char **date);

/** Translates a Chronological Julian Day to a Hebrew
    date.

    \param[in] CJD the Chronological Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDtoHebrew(double CJD, int32_t *year, int32_t *month, double *day);

/** Translates a Chronological Julian Day to a Hebrew calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void   CJDtoHebrewA(double const *CJD, double *date);

/** Translates a Chronological Julian Day to Hebrew calendar
    date text, consisting of the (fractional) day number, the full
    month name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJD the Chronological Julian Day to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
char * CJDtoHebrewS(double CJD);

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
void   CJDtoHebrewSA(double const *CJD, char **date);

/** Translates a Hebrew calendar date to a Chronological Julian Day Number.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
int32_t    HebrewtoCJDN(int32_t year, int32_t month, int32_t day);

/** Translates a Hebrew calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL.
 */
void   HebrewtoCJDNA(int32_t const *date, int32_t *CJDN);

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
int32_t    HebrewStoCJDN(char const *date);

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
void   HebrewStoCJDNA(char * const *date, int32_t *CJDN);

/** Translates a Hebrew calendar date to a Chronological Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day
 */
double HebrewtoCJD(int32_t year, int32_t month, double day);

/** Translates a Hebrew calendar date to a Chronological Julian Day.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void   HebrewtoCJDA(double const *date, double *CJD);

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
double HebrewStoCJD(char const *date);

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
void   HebrewStoCJDA(char * const *date, double *CJD);

// ISLAMIC CALENDAR

/**
    Translates a Chronological Julian Day Number to a Islamic
    calendar date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDNtoIslamic(int32_t CJDN, int32_t *year, int32_t *month, int32_t *day);

/** Translates a Chronological Julian Day Number to a Islamic
    calendar date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!
    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, date).
 */
void   CJDNtoIslamicA(int32_t const *CJDN, int32_t *date);

/** Translates a Chronological Julian Day Number to Islamic
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN the Chronological Julian Day number to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
char * CJDNtoIslamicS(int32_t CJDN);

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
void   CJDNtoIslamicSA(int32_t const *CJDN, char **date);

/** Translates a Chronological Julian Day to a Islamic
    date.

    \param[in] CJD the Chronological Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDtoIslamic(double CJD, int32_t *year, int32_t *month, double *day);

/** Translates a Chronological Julian Day to a Islamic calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void   CJDtoIslamicA(double const *CJD, double *date);

/** Translates a Chronological Julian Day to Islamic calendar
    date text, consisting of the (fractional) day number, the full
    month name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJD the Chronological Julian Day to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
char * CJDtoIslamicS(double CJD);

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
void   CJDtoIslamicSA(double const *CJD, char **date);

/** Translates a Islamic calendar date to a Chronological Julian Day Number.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
int32_t    IslamictoCJDN(int32_t year, int32_t month, int32_t day);

/** Translates a Islamic calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL.
 */
void   IslamictoCJDNA(int32_t const *date, int32_t *CJDN);

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
int32_t    IslamicStoCJDN(char const *date);

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
void   IslamicStoCJDNA(char * const *date, int32_t *CJDN);

/** Translates a Islamic calendar date to a Chronological Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day
 */
double IslamictoCJD(int32_t year, int32_t month, double day);

/** Translates a Islamic calendar date to a Chronological Julian Day.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void   IslamictoCJDA(double const *date, double *CJD);

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
double IslamicStoCJD(char const *date);

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
void   IslamicStoCJDA(char * const *date, double *CJD);

// EGYPTIAN CALENDAR

/**
    Translates a Chronological Julian Day Number to a Egyptian
    calendar date.

    \param[in] CJDN the Chronological Julian Day Number to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDNtoEgyptian(int32_t CJDN, int32_t *year, int32_t *month, int32_t *day);

/** Translates a Chronological Julian Day Number to a Egyptian
    calendar date.

    \param[in] CJDN a pointer to the Chronological Julian Day Number to
    translate.  Must not be NULL!
    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, date).
 */
void   CJDNtoEgyptianA(int32_t const *CJDN, int32_t *date);

/** Translates a Chronological Julian Day Number to Egyptian
    calendar date text, consisting of the day number, the full month
    name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJDN the Chronological Julian Day number to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.

  */
char * CJDNtoEgyptianS(int32_t CJDN);

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
void   CJDNtoEgyptianSA(int32_t const *CJDN, char **date);

/** Translates a Chronological Julian Day to a Egyptian
    date.

    \param[in] CJD the Chronological Julian Day to translate
    \param[out] year pointer to the corresponding year.  Must not be NULL!
    \param[out] month pointer to the corresponding month.  Must not be NULL!
    \param[out] day pointer to the corresponding day.  Must not be NULL!
 */
void   CJDtoEgyptian(double CJD, int32_t *year, int32_t *month, double *day);

/** Translates a Chronological Julian Day to a Egyptian calendar date.

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding calendar date.
    Must have at least 3 elements (year, month, day).
 */
void   CJDtoEgyptianA(double const *CJD, double *date);

/** Translates a Chronological Julian Day to Egyptian calendar
    date text, consisting of the (fractional) day number, the full
    month name (in English), and the year number, separated by single
    whitespaces.

    \param[in] CJD the Chronological Julian Day to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
char * CJDtoEgyptianS(double CJD);

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
void   CJDtoEgyptianSA(double const *CJD, char **date);

/** Translates a Egyptian calendar date to a Chronological Julian Day Number.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day Number
 */
int32_t    EgyptiantoCJDN(int32_t year, int32_t month, int32_t day);

/** Translates a Egyptian calendar date to a Chronological Julian Day
    Number.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL.
 */
void   EgyptiantoCJDNA(int32_t const *date, int32_t *CJDN);

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
int32_t    EgyptianStoCJDN(char const *date);

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
void   EgyptianStoCJDNA(char * const *date, int32_t *CJDN);

/** Translates a Egyptian calendar date to a Chronological Julian Day.

    \param[in] year the year
    \param[in] month the month
    \param[in] day the day
    \return the corresponding Chronological Julian Day
 */
double EgyptiantoCJD(int32_t year, int32_t month, double day);

/** Translates a Egyptian calendar date to a Chronological Julian Day.

    \param[in] date a pointer to the calendar date.  Must have at least 3
    elements (year, month, day).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void   EgyptiantoCJDA(double const *date, double *CJD);

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
double EgyptianStoCJD(char const *date);

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
void   EgyptianStoCJDA(char * const *date, double *CJD);

// JULIAN DAY

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
void CJDNtoCJDNA(int32_t const *in, int32_t *out);

/** Copies a Chronological Julian Day Number.

    \param[in] in a pointer to the Chronological Julian Day Number to
    copy.  Must not be NULL!

    \param[out] out a pointer to the target.  Must not be NULL!

 */
void CJDtoCJDA(double const *in, double *out);

// LUNAR CALENDAR

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

// MAYAN TZOLKIN/HAAB CALENDAR

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
void CJDNtoMayan(int32_t CJDN, int32_t *trecena, int32_t *venteina,
                 int32_t *haab_day, int32_t *haab_month,
                 int32_t *year_trecena, int32_t *year_venteina);

/** Translates a Chronological Julian Day Number into a Mayan
    tzolkin/haab calendar date (from Tikal).

    \param[in] CJDN a pointer to the Chronological Julian Day Number
    to translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding Mayan date.  Must
    have at least 6 elements (trecena, venteina, haab day, haab month,
    trecena of first day of haab year, venteina of first day of haab
    year)!
 */
void CJDNtoMayanA(int32_t const *CJDN, int32_t *date);

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
char *CJDNtoMayanS(int32_t CJDN);

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
void CJDNtoMayanSA(int32_t const *CJDN, char **date);

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
void CJDtoMayan(double CJD, int32_t *trecena, int32_t *venteina,
                double *haab_day, int32_t *haab_month,
                int32_t *year_trecena, int32_t *year_venteina);

/** Translates a Chronological Julian Day into a Mayan tzolkin/haab
    calendar date (from Tikal).

    \param[in] CJD a pointer to the Chronological Julian Day Number
    to translate.  Must not be NULL!

    \param[out] date a pointer to the corresponding Mayan date.  Must
    have at least 6 elements (trecena, venteina, haab day, haab month,
    trecena of first day of haab year, venteina of first day of haab
    year)!
 */
void CJDtoMayanA(double const *CJD, double *date);

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
char *CJDtoMayanS(double CJD);

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
void CJDtoMayanSA(double const *CJD, char **date);

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
int32_t MayantoCJDN(int32_t trecena, int32_t venteina, int32_t haab_day, int32_t haab_month,
                int32_t year_trecena, int32_t year_venteina, int32_t CJDN_upper);

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
void MayantoCJDNA(int32_t const *date, int32_t *CJDN);

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
int32_t MayanStoCJDN(char const *date);

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
void MayanStoCJDNA(char * const *date, int32_t *CJDN);

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
double MayantoCJD(int32_t trecena, int32_t venteina, double haab_day, int32_t haab_month,
                  int32_t year_trecena, int32_t year_venteina, int32_t CJDN_upper);

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
void MayantoCJDA(double const *date, double *CJD);

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
double MayanStoCJD(char const *date);

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
void MayanStoCJDA(char * const *date, double *CJD);

// MAYAN LONG COUNT CALENDAR

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
void CJDNtoLongCount(int32_t CJDN, int32_t *baktun, int32_t *katun, int32_t *tun,
                     int32_t *uinal, int32_t *kin);

/** Translates a Chronological Julian Day Number to a Maya Long Count.

    \param[in] CJDN the Chronological Julian Day Number to translate

    \param[out] date pointer to the corresponding Long Count,
    which must have at least 5 elements (from most significant to
    least significant).  The first element [baktun] is not restricted.
    The last 4 elements are restricted to ranges from 0 through,
    respectively, 19 [katun], 19 [tun], 17 [uinal], and 19 [kin].
 */
void CJDNtoLongCountA(int32_t const *CJDN, int32_t *date);

/** Translates a Chronological Julian Day Number to Mayan Long Count
    calendar date text, consisting of the five numbers in decreasing
    order of significance, separated by periods.

    \param[in] CJDN the Chronological Julian Day number to translate.

    \return a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
char *CJDNtoLongCountS(int32_t CJDN);

/** Translates a Chronological Julian Day Number to Mayan Long Count
    calendar date text, consisting of the five numbers in decreasing
    order of significance, separated by periods.

    \param[in] CJDN a pointer to the Chronological Julian Day number
    to translate.  Must not be NULL!

    \param[out] date a pointer to a character array containing the
    corresponding calendar date text.  The character array is
    allocated by the routine; free after use.
  */
void CJDNtoLongCountSA(int32_t const *CJDN, char **date);

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
void CJDtoLongCount(double CJD, int32_t *baktun, int32_t *katun, int32_t *tun, int32_t *uinal,
                    double *kin);

/** Translates a Chronological Julian Day to a Maya Long Count.

    \param[in] CJD the Chronological Julian Day to translate.

    \param[out] date a pointer to the corresponding Long Count
    numbers, in decreasing order of significance.  Must have at least
    5 elements!
 */
void CJDtoLongCountA(double const *CJD, double *date);

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
char *CJDtoLongCountS(double CJD);

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
void CJDtoLongCountSA(double const *CJD, char **date);

/** Translates a Maya Long Count to a Chronological Julian Day Number.

    \param[in] baktun the baktun number.

    \param[in] katun the katun number.

    \param[in] tun the tun number.

    \param[in] uinal the uinal number.

    \param[in] kin the kin number.

    \return the corresponding Chronological Julian Day Number.
 */
int32_t LongCounttoCJDN(int32_t baktun, int32_t katun, int32_t tun, int32_t uinal, int32_t kin);

/** Translates a Maya Long Count to a Chronological Julian Day Number.

    \param[in] date a pointer to the Maya Long Count, which must
    have at least 5 elements (from most significant to least
    significant).

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  Must not be NULL!
 */
void LongCounttoCJDNA(int32_t const *date, int32_t *CJDN);

/** Translates a Maya Long Count text to a Chronological Julian Day
    Number.

    \param[in] date the Long Count text, which must mention up to 5
    Long Count numbers in decreasing order of significance, separated
    by non-numerical characters.

    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal Long Count was recognized.
  */
int32_t LongCountStoCJDN(char const *date);

/** Translates a Maya Long Count text to a Chronological Julian Day
    Number.

    \param[in] date the Long Count text, which must mention up to 5
    Long Count numbers in decreasing order of significance, separated
    by non-numerical characters.

    \param[out] CJDN a pointer to the corresponding Chronological
    Julian Day Number.  If no legal Long Count was recognized, then 0
    is returned.
  */
void LongCountStoCJDNA(char * const *date, int32_t *CJDN);

/** Translates a Maya Long Count to a Chronological Julian Day.

    \param[in] baktun the baktun number.

    \param[in] katun the katun number.

    \param[in] tun the tun number.

    \param[in] uinal the uinal number.

    \param[in] kin the kin number.

    \return the corresponding Chronological Julian Day.
 */
double LongCounttoCJD(int32_t baktun, int32_t katun, int32_t tun, int32_t uinal, double kin);

/** Translates a Maya Long Count to a Chronological Julian Day.

    \param[in] date a pointer to the Maya Long Count, which must have
    5 elements (from most significant to least significant).

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!
 */
void LongCounttoCJDA(double const *date, double *CJD);

/** Translates a Maya Long Count text to a Chronological Julian Day.

    \param[in] date the Long Count text, which must mention up to 5
    Long Count numbers in decreasing order of significance, separated
    by non-numerical characters.  NOTE: only integer numbers are
    recognized.

    \return the corresponding Chronological Julian Day, or 0 if no
    legal Long Count was recognized.
  */
double LongCountStoCJD(char const *date);

/** Translates a Maya Long Count text to a Chronological Julian Day.

    \param[in] date the Long Count text, which must mention up to 5
    Long Count numbers in decreasing order of significance, separated
    by non-numerical characters.  NOTE: only integer numbers are
    recognized.

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  If no legal Long Count was recognized, then 0 is
    returned.
  */
void LongCountStoCJDA(char * const *date, double *CJD);

#endif
