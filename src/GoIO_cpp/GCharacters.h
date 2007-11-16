// GCharacters.h

#ifndef _GCHARACTERS_H_
#define _GCHARACTERS_H_

namespace GUTF8CharCode
{

const unsigned short kAmpersand = 0x26;

const unsigned short kNonBreakingSpace = 0xA0;
const unsigned short kCopyright = 0xA9;
const unsigned short kRegistered = 0xAE;
const unsigned short kDegree = 0xB0;
const unsigned short kPlusOrMinus = 0xB1;
const unsigned short kMu = 0xB5;
const unsigned short kBullet = 0xB7;
const unsigned short kOneFourth = 0xBC;
const unsigned short kOneHalf = 0xBD;
const unsigned short kThreeFourths = 0xBE;

const unsigned short kLowerCaseAlpha = 0x3B1;
const unsigned short kLowerCaseBeta = 0x3B2;
const unsigned short kLowerCaseTheta = 0x3B8;
const unsigned short kLowerCasePi = 0x3C0;

const unsigned short kUpperCaseDelta = 0x394;
const unsigned short kUpperCaseSigma = 0x3A3;
const unsigned short kUpperCaseOmega = 0x3A9;

const unsigned short kMathSum = 0x2211; // same as greek uppercase sigma?

const unsigned short kSqareRoot = 0x221A;
const unsigned short kIntegral = 0x222B;
const unsigned short kNotEqualTo = 0x2260;
const unsigned short kLessThanOrEqual = 0x2264;
const unsigned short kGreaterThanOrEqual = 0x2265;

const unsigned short kSuperScriptTwo = 0xB2;

} // end namespace GUTF8CharCode

#ifdef TARGET_OS_LINUX
// These character literals are readable on a Mac with any standard font.
const char kDeltaChar = 'Æ';
const char kCopyrightChar = '©';
const char kBulletChar = '¥';
const char kDegreeChar = '¡';
const char kMuChar = 'µ';
const char kPiChar = '¹';
const char kSumChar = '·';
const char kOmegaChar = '½';
const char kMathrootChar = 'Ã';
const char kNotEqualChar = '­';
const char kPlusOrMinusChar = '±';
const char kIntegralChar = 'º';
const char kOSNewlineChar = '\r';
const char kOSPathSeparatorChar = ':';
const char kAmpersandChar = '&';
const char kGreaterThanChar = '>';
const char kLessThanChar = '<';

const cppstring kOneQuarter = "1/4";
const cppstring kOneHalf = "1/2";
const cppstring kThreeQuarters = "3/4";
const cppstring kCapYAcute = "Y^";
const cppstring kLwYAcute = "y^";
const cppstring kCapSCaron = "S^";
const cppstring kLwSCaron = "s^";

const cppstring kSuperScriptTwo = "^2";

#define kOSNewlineString GSTD_S("\n")
#define kOSPathSeparatorString GSTD_S("/")

#endif // TARGET_OS_LINUX

#ifdef TARGET_OS_MAC
// These character literals are readable on a Mac with any standard font.
const char kDeltaChar = 'Æ';
const char kCopyrightChar = '©';
const char kBulletChar = '¥';
const char kDegreeChar = '¡';
const char kMuChar = 'µ';
const char kPiChar = '¹';
const char kSumChar = '·';
const char kOmegaChar = '½';
const char kMathrootChar = 'Ã';
const char kNotEqualChar = '­';
const char kPlusOrMinusChar = '±';
const char kIntegralChar = 'º';
const char kOSNewlineChar = '\r';
const char kOSPathSeparatorChar = ':';
const char kAmpersandChar = '&';
const char kGreaterThanChar = '>';
const char kLessThanChar = '<';

const cppstring kOneQuarter = "1/4";
const cppstring kOneHalf = "1/2";
const cppstring kThreeQuarters = "3/4";
const cppstring kCapYAcute = "Y^";
const cppstring kLwYAcute = "y^";
const cppstring kCapSCaron = "S^";
const cppstring kLwSCaron = "s^";

const cppstring kSuperScriptTwo = "^2";

#define kOSNewlineString GSTD_S("\r")
#define kOSPathSeparatorString GSTD_S(":")

#endif // TARGET_MAC_OS

#ifdef TARGET_OS_WIN
// Define windows special character equivalents here

const gchar kDeltaChar = GSTD_S('d');
const gchar kCopyrightChar = GSTD_S('©');
const gchar kBulletChar = GSTD_S('•');
const gchar kDegreeChar = GSTD_S('°');
const gchar kMuChar = GSTD_S('µ');
const gchar kPiChar = GSTD_S('x');
const gchar kSumChar = GSTD_S('x');
const gchar kOmegaChar = GSTD_S('x');
const gchar kMathrootChar = GSTD_S('x');
const gchar kNotEqualChar = GSTD_S('x');
const gchar kPlusOrMinusChar = GSTD_S('±');
const gchar kIntegralChar = GSTD_S('ƒ');
const gchar kOSNewlineChar = GSTD_S('\n'); // not CRLF, but it will work for text in dialogs and most edit-control situations
const gchar kOSPathSeparatorChar = GSTD_S('\\');

const gchar kAmpersandChar = '&';
const gchar kGreaterThanChar = GSTD_S('>');
const gchar kLessThanChar = GSTD_S('<');

const gchar kOneQuarter = GSTD_S('¼');
const gchar kOneHalf = GSTD_S('½');
const gchar kThreeQuarters = GSTD_S('¾');
const gchar kCapYAcute = GSTD_S('Ý');
const gchar kLwYAcute = GSTD_S('ý');
const gchar kCapSCaron = GSTD_S('Š');
const gchar kLwSCaron = GSTD_S('š');
const gchar kSuperScriptTwo = GSTD_S('²');

#define kOSNewlineString GSTD_S("\n")
#define kOSPathSeparatorString GSTD_S("\\")

#endif // TARGET_OS_WIN

#define kMacPathSeparatorString GSTD_S(":")
#define kWinPathSeparatorString GSTD_S("\\")

#endif // _GCHARACTERS_H_ 
