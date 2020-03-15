#include <stdarg.h>
#include <stdio.h>

void warning(const char *message) {
	fputs("WARNING: ", stderr);
	fputs(message, stderr);
	fputc('\n', stderr);
}

void warningf(const char *message, ...) {
	va_list args;

	fputs("WARNING: ", stderr);

	va_start(args, message);
	vfprintf(stderr, message, args);
	va_end(args);

	fputc('\n', stderr);
}
