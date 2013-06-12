/*********************************************************************************

Copyright (c) 2010, Vernier Software & Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Vernier Software & Technology nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL VERNIER SOFTWARE & TECHNOLOGY BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**********************************************************************************/
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
//namespace std {
//#ifndef _CSTDLIB_
//#include <cstdlib>
//#endif
//}; // end namespace std

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

