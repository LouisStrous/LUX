/* This is file hershey.c.

Copyright 2013 Louis Strous, Richard Shine

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
#include <errno.h> /* for errno(3) ENOMEM(2) ENOENT(1) */
#include <malloc.h> /* for free(3) realloc(2) malloc(2) */
#include <stddef.h> /* for NULL(9) */
#include <stdint.h> /* for uint16_t(9) uint32_t(7) int8_t(2) */
#include <stdio.h> /* for fread(5) printf(5) fclose(2) fopen(1) FILE(1) */
#include <stdlib.h> /* for bsearch(1) */
#include <string.h> /* for strdup(1) */
#include "hershey.h"

static uint32_t hershey_numbers_count;
static uint16_t *hershey_numbers = NULL;
static uint32_t *hershey_indices = NULL;
static uint32_t hershey_data_size;
static int8_t *hershey_data = NULL;
static char *hershey_filename = NULL;

char *hershey_set_filename(char *filename)
{
  if (filename) {
    free(hershey_filename);
    hershey_filename = strdup(filename);
  }
  return filename;
}

static Int hershey_load(char *filename)
{
  FILE *fp;

  if (!filename)
    filename = "hersheyfont.bin";
  fp = fopen(filename, "r");
  if (!fp) {
    errno = ENOENT;
    goto err_1;
  }
  if (fread(&hershey_numbers_count, sizeof(uint32_t), 1, fp) != 1) {
    goto err_2;
  }
  hershey_numbers = realloc(hershey_numbers,
			    hershey_numbers_count*sizeof(uint16_t));
  hershey_indices = realloc(hershey_indices,
			    hershey_numbers_count*sizeof(uint32_t));
  if (!hershey_numbers || !hershey_indices) {
    errno = ENOMEM;
    goto err_3;
  }
  if (fread(hershey_numbers, sizeof(uint16_t), hershey_numbers_count, fp)
      != hershey_numbers_count
      || fread(hershey_indices, sizeof(uint32_t), hershey_numbers_count, fp)
      != hershey_numbers_count
      || fread(&hershey_data_size, sizeof(uint32_t), 1, fp) != 1) {
    goto err_4;
  }
  hershey_data = malloc(hershey_data_size*sizeof(int8_t));
  if (!hershey_data) {
    errno = ENOMEM;
    goto err_5;
  }
  if (fread(hershey_data, 1, hershey_data_size, fp) != hershey_data_size) {
    goto err_6;
  }
  fclose(fp);
  return 0;

 err_6:
  free(hershey_data);
  hershey_data = NULL;
  hershey_data_size = 0;
 err_5:
 err_4:
  free(hershey_numbers);
  hershey_numbers = NULL;
  hershey_numbers_count = 0;
  free(hershey_indices);
  hershey_indices = NULL;
 err_3: 
 err_2:
  fclose(fp);
 err_1:  
  return 1;
}

static Int uint16cmp(const void *arg1, const void *arg2)
{
  return *(uint16_t *) arg1 - *(uint16_t *) arg2;
}

Int hershey_max_char(void)
{
  if (!hershey_numbers && hershey_load(NULL))
    return 0;
  return hershey_numbers[hershey_numbers_count - 1];
}

Int hershey_max_handle(void)
{
  if (!hershey_numbers && hershey_load(NULL))
    return 0;
  return hershey_data_size/sizeof(int8_t);
}

/*
  Int hershey_exists(Int hershey_char)

  Purpose: to return HERSHEY_ERR if the specified character from the
  Hershey character set does not exist, or else a handle to the
  coordinates of the Hershey character.

  Parameters:
    hershey_char = the number of the desired Hershey character
      (between 1 and 3926, inclusive; not all numbers correspond to
       a character)

  Return values:
    HERSHEY_ERR if no such Hershey character is defined, or otherwise a
    handle to the coordinates otherwise.
 */
hershey_handle hershey_exists(Int hershey_char) {
  uint16_t number;
  uint16_t *found;

  if (!hershey_numbers && hershey_load(NULL))
    return HERSHEY_ERR;
  number = hershey_char;
  found = bsearch(&number, hershey_numbers, hershey_numbers_count,
		  sizeof(uint16_t), uint16cmp);
  if (found) {
    Int i;

    i = (found - hershey_numbers)/sizeof(uint16_t);
    return hershey_indices[i];
  } else
    return HERSHEY_ERR;
}

/*
  Int hershey_coords(Int *index, Int *x, Int *y)

  Purpose: to return coordinates of a Hershey character.

  Parameters:
    index = a pointer to a handle to the coordinate data, obtained
      from a call to hershey_exists().  The handle is updated by
      the call to the current function.
    x = a pointer to where the next x coordinate for the character
      will be copied to.
    y = a pointer to where the next y coordinate for the character
      will be copied to.

  Return values:
    HERSHEY_DRAW = *x and *y contain coordinates of the next point
      to draw to.
    HERSHEY_MOVE = *x and *y contain coordinates of the next point
      to move to.
    HERSHEY_END = no more coordinates are defined for the current
      character, *x and *y were not changed.
    HERSHEY_ERR = the handle *index was invalid.

 */
Int hershey_coords(hershey_handle *handle, Int *x, Int *y) {
  Int result = HERSHEY_DRAW;

  if (*handle < 0 || *handle >= hershey_data_size)
    return HERSHEY_ERR;

  if (hershey_data[*handle] == -50) { /* move */
    result = HERSHEY_MOVE;
    (*handle)++;
  }
  if (hershey_data[*handle] == 99) {
    result = HERSHEY_END;
  } else {
    *x = hershey_data[*handle];
    *y = hershey_data[*handle + 1];
    *handle += 2;
  }
  return result;
}

#include "unittest.h"
Int test_hershey(void)
{
  Int bad = 0;

  {
    char *f = hershey_set_filename("foo.bar");
    bad += assertEqualTexts("foo.bar", f);
    bad += assertEqualTexts("foo.bar", hershey_filename);
    f = hershey_set_filename(NULL);
    bad += assertNULL(f); 
    bad += assertEqualTexts("foo.bar", hershey_filename);
  }
  return bad;
}
