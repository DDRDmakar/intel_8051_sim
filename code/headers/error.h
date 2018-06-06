
#ifndef __ERROR__
#define __ERROR__

void progstop(const char *message, const int errcode);
void progerr(const char *message);

#define MALLOC_NULL_CHECK(x) if (!x) progstop("Error - out of memory", 1);

#endif
