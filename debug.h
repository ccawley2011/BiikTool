#ifndef DEBUG_H
#define DEBUG_H

#ifdef __GNUC__
# define PRINTF_ATTR(x,y) __attribute__((__format__(__printf__, x, y)))
#else
# define PRINTF_ATTR(x,y)
#endif

void warning(const char *message);
void warningf(const char *message, ...) PRINTF_ATTR(1,2);

#endif
