#include "ctype.h"

// Checks whether a character is in the lowercase ASCII alphabet range.
bool islower(char chr){
    return chr >= 'a' && chr <= 'z';
}

// Converts a lowercase ASCII character to uppercase and leaves other characters unchanged.
char toupper(char chr){
    return islower(chr) ? (chr - 'a' + 'A') : chr;
}
