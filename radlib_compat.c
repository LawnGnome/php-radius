
#include <stddef.h>
#include "php.h"
#include "time.h"

int inet_aton(const char *cp, struct in_addr *inp) {
  inp->s_addr = inet_addr(cp);

  if (inp->s_addr == INADDR_NONE) {
	  return 0;
  }

  return 1;
}

char *strsep(char **stringp,	const char *delim)
{
	char *s;
	const char *spanp;
	int c, sc;
	char *tok;

	if ((s = *stringp) == NULL)
		return (NULL);
	for (tok = s;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}