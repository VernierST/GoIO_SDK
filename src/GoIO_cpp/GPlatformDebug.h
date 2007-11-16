// GPlatformDebug.h
//
// This file gets included in an #ifdef _DEBUG block in
// cross-platform files.  Thus any debug-build-specific,
// OS-specific stuff can go in blocks here to enable things
// like DEBUG_NEW.  USING_DEBUG_NEW should be defined in the
// prefix file, command-line define or project setting for
// targets which support this.

#ifdef USING_DEBUG_NEW

#ifdef TARGET_OS_MAC
#include "DebugNew.h"
#define OPUS_NEW NEW
#endif

#endif // USING_DEBUG_NEW
