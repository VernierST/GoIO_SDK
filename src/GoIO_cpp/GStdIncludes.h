// GStdIncludes.h

// This file includes commonly used standard C++ (and C) headers.
// The proper include guards are used based on platform.

#ifndef _GSTDINCLUDES_H_
#define _GSTDINCLUDES_H_


#ifdef TARGET_OS_LINUX

// C
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cctype>
#include <ctime>
#include <exception>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>

// C++ - standard containers & related
#include <vector>
#include <list>
#include <map>
#include <deque>
#include <algorithm>
#include <limits>
#include <locale>

#endif // TARGET_OS_LINUX



#ifdef TARGET_OS_MAC

// Macintosh includes use include-guard constants from
// Metrowerks Standard Library C++ headers.

// C
#include <cstdio>
#include <cstdlib>

#ifdef __MWERKS__
#ifndef __cmath_macos__
#include <cmath.macos.h> // MSL substitue for cmath
#endif
#else // not MSL
#include <cmath>
#endif

#include <cstring>
#include <cctype>
#include <ctime>
#include <exception>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>

// C++ - standard containers & related

#include <vector>
#include <list>
#include <map>
#include <deque>
#include <algorithm>
#include <limits>
#include <locale>

#endif // TARGET_OS_MAC


#ifdef TARGET_OS_WIN
// Windows includes use include-guard constants from Visual Studio 6
// C++ headers.

#pragma warning (disable: 4786)

// C

#include <cstdio>

// put cstdlib in namespace std (VC++ bug):
namespace std {
#ifndef _CSTDLIB_
#include <cstdlib>
#endif
}; // end namespace std

#include <cmath>
#include <cstring>
#include <cctype>
#include <ctime>
#include <exception>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <list>
#include <map>
#include <deque>
#include <algorithm>
#include <limits>

#endif // TARGET_OS_WIN


// arrgh - the compiler particulars got us, we have to introduce
// namespace std here...
using namespace std;

// STANDARD CROSS-PLATFORM CONSTANT:
// note that you will need to be in namespace std to use kPI:
#ifndef kPI
#define kPI 3.141592653589793
#endif
#ifndef kSQRT2PI
#define kSQRT2PI 2.506628746
#endif
#ifndef kE
#define kE 2.718281828
#endif


#endif // _GSTDINCLUDES_H_

