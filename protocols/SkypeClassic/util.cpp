#include <stdlib.h>

char * __cdecl strtok_r (
        char * string,
        const char * control,
		char **nextoken
        )
{
        unsigned char *str;
        const unsigned char *ctrl = (const unsigned char*)control;

        unsigned char map[32];
        int count;

        /* Clear control map */
        for (count = 0; count < 32; count++)
                map[count] = 0;

        /* Set bits in delimiter table */
        do {
                map[*ctrl >> 3] |= (1 << (*ctrl & 7));
        } while (*ctrl++);

        /* Initialize str. If string is NULL, set str to the saved
         * pointer (i.e., continue breaking tokens out of the string
         * from the last strtok call) */
        if (string)
                str = (unsigned char*)string;
        else
                str = (unsigned char*)(*nextoken);

        /* Find beginning of token (skip over leading delimiters). Note that
         * there is no token iff this loop sets str to point to the terminal
         * null (*str == '\0') */
        while ( (map[*str >> 3] & (1 << (*str & 7))) && *str )
                str++;

        string = (char*)str;

        /* Find the end of the token. If it is not the end of the string,
         * put a null there. */
        for ( ; *str ; str++ )
                if ( map[*str >> 3] & (1 << (*str & 7)) ) {
                        *str++ = '\0';
                        break;
                }

        /* Update nextoken (or the corresponding field in the per-thread data
         * structure */
        *nextoken = (char*)str;

        /* Determine if a token has been found. */
        if ( string == (char*)str )
                return NULL;
        else
                return string;
}
