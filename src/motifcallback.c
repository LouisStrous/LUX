/* This is file motifcallback.c.

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
 /*------------------------------------------------------------------------- */
void execute_string(char *text)
 /* this needs to be re-entrant, the parser call here may cause
 this routine to be called, so be careful */
{
 extern Int	nest, motif_input_flag;
 extern	Int	single_string_parse_flag, single_string_parse_nest, nest;
 extern Byte	*input_compile_string;	/* used for compiling strings */
 extern Byte line2[];
 extern struct	sdesc	cur_line;
 extern	char	*strsavsd();
 struct	sdesc	save_cur_line;
 Arg	args[10];
 Cardinal	n;
 unsigned char *new_text;
 Int	iq, mq, start_nest, nsym, prev_flag;
 char *text, *p, *s;
 Int	result_position, result_string;

 mq = strlen(text);
 prev_flag = motif_input_flag; /* save this state, allows for nesting */
 /* printf("command entered: %s\n", s); */
 /* check if something in the parser buffer and save if so */
 /* we'll put it back at the end */
 if (save_cur_line.n=cur_line.n) save_cur_line.p=(Byte *) strsavsd(&cur_line);
 cur_line.n = 0;
 input_compile_string = (Byte *) text;
 /* if we entered with input_modal_flag on, we don't call parser */
 if (input_modal_flag == 0) {
 single_string_parse_flag = 1;
 single_string_parse_nest = nest;	/* set to current nest level */
 start_nest = nest;
 /* parse and execute it */
 do {
 nsym = parser();
 /*printf("parsed symbol # = %d, nest = %d, start_nest = %d\n",
 	 nsym, nest, start_nest);
 */
 if (nsym > 0) execute( nsym);
 } while (nest > start_nest || cur_line.n > 0);
 /* clean up */
 /* 2/28/97 - put in the clear_edb, was forgotten and caused the main level
 edb's to be eventually used up */
 clear_edb();
 single_string_parse_flag = 0;
 /* in case there was a partial line at the start, we
 copy the original line back into line2 (our preprocessed buffer) */
 if (cur_line.n = save_cur_line.n)  {
 strncpy((char *) line2, (char *) save_cur_line.p, save_cur_line.n);
 cur_line.p = line2;
 free(save_cur_line.p);
 }
 }
 }
 /*------------------------------------------------------------------------- */
