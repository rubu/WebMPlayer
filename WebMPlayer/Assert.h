#if defined(_WIN32)
#include <crtdbg.h>

#define Assert(x)  { _ASSERTE(x); }
#endif