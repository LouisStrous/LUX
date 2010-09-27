/* editorcharclass.h */

extern char class[];

	/* character classes */
#define charClass(c)		(class[(int) c])
#define editorCharClass(c)	(charClass(c) & 0x03)
/* distinguishes between WHITESPACE_CHAR, SEPARATOR_CHAR, and ORDINARY_CHAR */
#define isFirstChar(c)		(charClass(c) & 0x04)
/* may appear at start of identifier */
#define isNextChar(c)		(charClass(c) & 0x08)
/* may appear later on in identifier */
#define isNumberChar(c)		(charClass(c) & 0x10)
/* all numbers that may appear in numbers: 0-9, A-F, O, X, W, B, L, period */
#define isTwoOperator(c)	(charClass(c) & 0x20)
/* may be first char of a two-char operator *= += -= /= ^= */
#define isFileNameChar(c)	(charClass(c) & 0x40)
/* may appear in a file name: lower case, upper case, _$/.  */
#define isNotCapital(c)		(charClass(c) != 0x40)
#define isWhiteSpace(c)		(charClass(c) == 0)
#define isSeparatorChar(c)	(charClass(c) & 0x01)
#define isOrdinaryChar(c)	(charClass(c) & 0x02)



