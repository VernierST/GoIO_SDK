// GUtils.cpp

#include "stdafx.h"
#include "GUtils.h"
#include "GTextUtils.h"
#include "GCharacters.h"
/*
#include "GColumn.h"
#include "GMessenger.h"
#include "GMessages.h"
#include "GTextUtils.h"
#include "GApplicationBrain.h"
#include "GDataWorld.h"
#include "GDrawing.h"
#include "GStdIncludes.h"
#include "GMBLBrain.h"
*/
#ifdef _DEBUG
#include "GPlatformDebug.h" // for DEBUG_NEW definition
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*
using namespace std;

// kRealMin and kRealMax are the min and max allowable numbers in Opus (kRealMin replaces kRealEpsilon)
real GUtils::kRealMin = 1.1e-16;	// a very small number (~1E-16) designed to test if 2 reals are equal AND is the smallest number allowable in Opus
real GUtils::kRealMax = 1.0e50;		// a very large number (1E26) -- the largest number allowable in Opus

real GUtils::kfMetersPerFoot = 0.3048;
*/
cppofstream *	GUtils::pLogOStream = NULL;		// Used to output text to a logfile
/*
bool GUtils::IsBetween(real range1,		// one end of the range to test
					   real value,		// value to see if it's between range1 and range2
					   real range2)		// the other end of the range
{ // RETURN true if value is between the ranges (inclusive)
	bool bBetween = false;
	if (range1 <= range2)
		bBetween = ((value >= range1) && (value <= range2));
	else
		bBetween = ((value <= range1) && (value >= range2));

	return bBetween;
}

StdIDVector GUtils::ResolveIDVector(StdIDVector *pvIDs) // vector of IDs
{ // RETURN a vector of column or FunctionModel IDs based on this vector of IDs

	StdIDVector dataIDVector;

	if (pvIDs != NULL)
	{
		// Get a pointer to our dataworld
		GDataWorld *pDataWorld = GetAppBrain()->GetDataWorld();

		// We need to resolve the pvIDs into a list of GData objects only
		// (it may contain data sets).  We also remove any duplicate GData IDs.
		StdIDVector dataSetDataVector;
		StdIDVectorIterator iter = pvIDs->begin();
		while(iter != pvIDs->end())
		{
			EStdObjectType type = GMessenger::GetDataWorldChildType(*iter);
			if (type == kStdObject_DataSet)
			{
				dataSetDataVector = pDataWorld->GetObjectsInDataSet(*iter);
				for (size_t ix=0; ix<dataSetDataVector.size(); ix++)
				{
					GStdObject *pObj = dynamic_cast<GStdObject *>(GMessenger::GetDataWorldChildPtr(dataSetDataVector[ix]));
					if ((pObj != NULL) && 
						((pObj->GetType() == kStdObject_DataColumn) ||
						 (pObj->GetType() == kStdObject_FunctionModel)))
						// add if we don't find this ID already...
						if (std::find(dataIDVector.begin(), dataIDVector.end(), *iter) == dataIDVector.end())
							dataIDVector.push_back(dataSetDataVector[ix]);
				}
			}
			else
			if ((type == kStdObject_DataColumn) ||
				(type == kStdObject_FunctionModel))
			{
				// add if we don't find this ID already...
				if (std::find(dataIDVector.begin(), dataIDVector.end(), *iter) == dataIDVector.end())
					dataIDVector.push_back(*iter);
			}
			*iter++;
		}
	}
		
	return dataIDVector;
}

StdIDVector GUtils::ConvertIDListToVector(const StdIDList * pIDList) // pointer to list of StdIDs
{
	// RETURN a vector of IDs based on this list of IDs
	StdIDVector IDVector;
	if (!pIDList || pIDList->size() <= 0)
		return IDVector;
		
	for (StdIDList::const_iterator iTheID = pIDList->begin(); iTheID != pIDList->end(); iTheID++)
		IDVector.push_back(*iTheID);
	
	return IDVector;
}

StdIDList GUtils::ConvertIDVectorToList(const StdIDVector * pIDVector) // pointer to vector of StdIDs
{
	// RETURN a list of IDs based on this vector of IDs
	
	StdIDList IDList;
	if (!pIDVector || pIDVector->size() <= 0)
		return IDList;
		
	for (size_t ix = 0; ix < pIDVector->size(); ix++)
		IDList.push_back(pIDVector->at(ix));
	
	return IDList;
}


// Temp hack
#ifdef TARGET_OS_MAC
#include <DateTimeUtils.h>

char *_strtime(char *pBuffer);
char *_strtime(char *pBuffer)
{
	unsigned long nSeconds;
	GetDateTime(&nSeconds);

	Str255 sDate;
	DateString(nSeconds, shortDate, sDate, NULL);
	
	Str255 sTime;
	TimeString(nSeconds, false, sTime, NULL);
	
	if ((sTime[0] + sDate[0] > 0) && (sTime[0] + sDate[0] < 126))
	{
		memcpy(pBuffer, &sTime[1], sTime[0]);
		pBuffer[sTime[0]] = ' ';
		memcpy(pBuffer + sTime[0] + 1, &sDate[1], sDate[0]);
		pBuffer[sTime[0] + sDate[0] + 1] = 0;
	}

	return pBuffer;
}

#endif

cppstring GUtils::GetCurrentStdDateTimeText(void)
{ // Return the current date and time in a string
	gchar text[128];

	cppstring sDate;
#ifdef _UNICODE
#ifndef TARGET_OS_MAC
	TCHAR timeStr[256];
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, NULL, NULL, timeStr, 256);
	_wstrtime(text);
	sDate = sDate + GSTD_S(" ") + text;
#else
	_wstrtime(text);
	sDate = text;
	sDate.erase(sDate.length()-1, 1); // bad last character
#endif
#else
        
#ifndef TARGET_OS_MAC
	TCHAR timeStr[256];
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, NULL, NULL, timeStr, 256);
	sDate = timeStr;
	_strtime(text);
	sDate = sDate + GSTD_S(" ") + text;
#else
	_strtime(text);
	sDate = text;
#endif

#endif
	return sDate;
}

cppstring GUtils::GetCurrentStdTimeText(void)
{ // Return the current time in a string
	gchar text[128];

	cppstring sTime;
#ifdef _UNICODE
	_wstrtime(text);
#else
	_strtime(text);
#endif

	sTime = text;
	return sTime;
}

cppstring GUtils::ConvertIDVectorToString(const StdIDVector &idVect)
{ // RETURN a string of the size and IDs in the vector
	cppsstream ss;
	ss << idVect.size() << GSTD_S(" ");
	for (size_t ix = 0; ix < idVect.size(); ix++)
		ss << idVect[ix] << GSTD_S(" ");

	return ss.str();
}


StdIDVector GUtils::ConvertStringToIDVector(const cppstring &sVect)
{ // RETURN a vector of the ids in the passed-in string
	StdIDVector idVect;
	
	if (!sVect.empty())
	{
		cppsstream ss;
		ss << sVect;
		int nSize;
		ss >> nSize;
		for (int i=0; i<nSize; i++)
		{
			StdID id;
			ss >> id;
			idVect.push_back(id);
		}
	}
	return idVect;
}

cppstring GUtils::ConvertIDListToString(const StdIDList &idList)
{ // RETURN a string of the size and IDs in the list
	StdIDVector idVect = GUtils::ConvertIDListToVector(&idList);
	return GUtils::ConvertIDVectorToString(idVect);
}

StdIDList GUtils::ConvertStringToIDList(const cppstring &sList)
{ // RETURN a vector of the ids in the passed-in string

	StdIDVector idVect = GUtils::ConvertStringToIDVector(sList);
	return GUtils::ConvertIDVectorToList(&idVect);
}

cppstring GUtils::ConvertIDPairsToString(const SStdIDPairVector &vIDPairs)
{
	cppsstream ss;
	ss << vIDPairs.size() << " ";
	for (size_t ix = 0; ix < vIDPairs.size(); ix++)
		ss << vIDPairs[ix].nBaseID << " " << vIDPairs[ix].nTraceID << " ";

	return ss.str();
}

SStdIDPairVector GUtils::ConvertStringToIDPairs(const cppstring &sVect)
{ // RETURN a vector of the id pairs in the passed-in string
	SStdIDPairVector vIDPairs;
	
	if (!sVect.empty())
	{
		cppsstream ss;
		ss << sVect;
		int nSize;
		ss >> nSize;
		for (int i=0; i<nSize; i++)
		{
			SStdIDPair idPair;
			ss >> idPair.nBaseID;
			ss >> idPair.nTraceID;
			vIDPairs.push_back(idPair);
		}
	}
	return vIDPairs;
}

StdIDVector GUtils::GetUniqueIDs(const StdIDVector &vIDs) // input vector with (possible) duplicate IDs
{ // Return a vector of just the unique IDs from vIDs i.e Remove duplicates

	StdIDVector goodIDs;
	for (size_t kx=0; kx < vIDs.size(); kx++)
		if (std::find(goodIDs.begin(), goodIDs.end(), vIDs[kx]) == goodIDs.end()) 
			goodIDs.push_back(vIDs[kx]);

	return goodIDs;
}

StringVector GUtils::GetUniqueStrings(const StringVector &vs) // input vector with (possible) duplicate Strings
{ // Return a vector of just the unique strings i.e Remove duplicates
	StringVector vsGood;
	for (size_t ix = 0; ix < vs.size(); ix++)
	{
		bool bFound = false;
		for (size_t  jx = 0; jx < vsGood.size(); jx++)
			if (vs[ix] == vsGood[jx])
			{
				bFound = true;
				break;
			}
		if (!bFound)
			vsGood.push_back(vs[ix]);
	}

	return vsGood;
}

StringVector GUtils::GetNewStrings(const StringVector &oldVec, const StringVector &newVec)
{ // Return a vector of strings that are in newVec, but not in oldVec.
	StringVector vsDelta;
	for (size_t ix = 0; ix < newVec.size(); ix++)
	{
		bool bFound = false;
		for (size_t  jx = 0; jx < oldVec.size(); jx++)
			if (newVec[ix] == oldVec[jx])
			{
				bFound = true;
				break;
			}
		if (!bFound)
			vsDelta.push_back(newVec[ix]);
	}

	return vsDelta;
}

StringVector GUtils::GetOldStrings(const StringVector &oldVec, const StringVector &newVec)
{ // Return a vector of strings that are in oldVec, but not in newVec.
	return GetNewStrings(newVec, oldVec);
}

StdIDList GUtils::GetPrunedIDList(StdIDList theList)
{ // Remove any dead and duplicate IDs from this list

	// Remove dead IDs
	StdIDListIterator iter = theList.begin();
	while (iter != theList.end())
	{
		GStdObject *pObj = GMessenger::GetStdObjectPtr(*iter);
		if (pObj == NULL)
			iter = theList.erase(iter);	// item is dead, remove from list
		else
			iter++;	
	}

	// Remove duplicates
	theList.sort();
	theList.unique();

	return theList;
}

StdIDVector GUtils::GetPrunedIDVector(StdIDVector *pvIDs)
{ // Remove any dead and duplicate IDs from this vector
	StdIDList theList = GUtils::GetPrunedIDList(ConvertIDVectorToList(pvIDs));
	return GUtils::ConvertIDListToVector(&theList);
}

// Curve fitting helper functions
// The following list contains all of the resources used for the curve fitting functions
// LinearFit:: " An Introduction to Error Analysis", pages 156-157 - correlation is from page 180 of same book
long GUtils::CalculateLinearFit(SRealPointVector const &vPoints,	// vector of points to fit line to
							    real *pfSlope,				// [out] slope of linear fit
							    real *pfIntercept,			// [out] intercept of linear fit line
							    real *pfCorrelation,			// [out] correlation of line to data; if NULL don't fill in
							    real *pfStdDevSlope,			// [out] standard deviation of slope (uncertainity)
							    real *pfStdDevIntercept)		// [out] standard deviation of intercept (uncertainity)	
{ // RETURN kResponse_OK if fit went well, ID of error string otherwise
	// zero out values in case calc fails
	int i = 0;
	*pfSlope = 0.0;
	*pfIntercept = 0.0;
	if (pfCorrelation != NULL)
		*pfCorrelation = 0.0;
	int nNumPoints = vPoints.size();

	// REVIST error condition
	if (nNumPoints < 2)
		return GSTD_INDEX(IDSX_NOT_ENOUGH_POINTS);
	else
	if (nNumPoints == 2)
	{
		// check for vertical line (error)
		if (vPoints[0].x == vPoints[1].x)
			return GSTD_INDEX(IDSX_VERTICAL_LINE);

		*pfSlope = (vPoints[1].y - vPoints[0].y) / (vPoints[1].x - vPoints[0].x);
		*pfIntercept = - *pfSlope * vPoints[0].x + vPoints[0].y;
		if (pfCorrelation != NULL)
			*pfCorrelation = 1.0;
		if (pfStdDevSlope != NULL)
			*pfStdDevSlope = 0.0;
		if (pfStdDevIntercept != NULL)
			*pfStdDevIntercept = 0.0;
	}
	else
	{
		real sumX = 0;  // Sum of 1..N of x values
		real sumY = 0;  // Sum of 1..N of y values
		real sumXSqr = 0; // Sum of 1..N of x*x
		real sumXY = 0;   // Sum of 1..N of x*y
		real cur_x = 0;
		real cur_y = 0;
		real meanx = 0;   // average of x values
		real meany = 0;   // average of y values
		// first I calculate the necessary  sums:  sum(x) sum (x*x) sum (y) sum (y*y) and sum (x*y)
		// from these I then calculate the specific values for intercept, slope etc.
		// Use the following equations (from book mentioned above)
		// delta = nNumPoints* ( sumXSqr) - (sumX)*sumX)
		// intercept =  (sumXSqr*sumY - sumX*sumXY)/delta
		// slope = (nNumPoints*sumXY - sumX*sumY)/delta
		// Correlation  coefficeint is
		//
		// SUM( ( xi -meanx)(yi-meany))/ sqrt( SUM((xi -meanx)*(xi-meanx))* SUM((yi-meany)*(yi-meany)) )

		for (i=0; i<nNumPoints; i++)
		{
			cur_x = vPoints[i].x;
			cur_y = vPoints[i].y;
			sumX += cur_x;
			sumY += cur_y;
			sumXSqr += cur_x * cur_x;
			sumXY += cur_x * cur_y;
		}

		meanx = sumX/nNumPoints;
		meany = sumY/nNumPoints;

		double delta = nNumPoints * sumXSqr - sumX*sumX;

		// handle strange case if delta is zero...
		// REVISIT error handling
		if (delta == 0.0)
			return GSTD_INDEX(IDSX_VERTICAL_LINE);

		*pfSlope = (nNumPoints*sumXY - sumX*sumY)/delta;
		*pfIntercept = (sumXSqr*sumY - sumX*sumXY)/delta;
		
		if ((pfCorrelation != NULL) || (pfStdDevSlope != NULL) || (pfStdDevIntercept != NULL))
		{
			double sumXYmean = 0;  // Sum (xi- meanx)*(yi -y)
			double sumXmeanXSqr = 0; // Sum (xi -meanx)*(xi-meanx);
			double sumYmeanYSqr = 0; // Sum (yi-meany)*(yi-meany);
			
			for (i=0; i<nNumPoints; i ++)
			{
				sumXYmean += (vPoints[i].x - meanx)*(vPoints[i].y-meany);
				sumXmeanXSqr += (vPoints[i].x - meanx)*(vPoints[i].x - meanx);
				sumYmeanYSqr += (vPoints[i].y - meany)*(vPoints[i].y - meany);
			}

			if (pfCorrelation != NULL)
			{ // compute correlation
				real denominator = sumXmeanXSqr * sumYmeanYSqr;
				if (denominator == 0.0)
					*pfCorrelation = 0.0;	// double check for 0 denominator
				else
					*pfCorrelation = sumXYmean/ sqrt(denominator);
			}

			if ((pfStdDevSlope != NULL) || (pfStdDevIntercept != NULL))
			{
				real fSumXSqr = 0;
				for (i=0; i<nNumPoints; i++)
					if (GUtils::OSIsValidNumber(vPoints[i].x))
						fSumXSqr += vPoints[i].x * vPoints[i].x;
				real fSumX = 0;
				for (i=0; i<nNumPoints; i++)
					if (GUtils::OSIsValidNumber(vPoints[i].x))
						fSumX += vPoints[i].x;
				real fSumY = 0;
				for (i=0; i<nNumPoints; i++)
					if (GUtils::OSIsValidNumber(vPoints[i].y))
						fSumY += vPoints[i].y;
				real fSumXY = 0;
				for (i=0; i<nNumPoints; i++)
					if (GUtils::OSIsValidNumber(vPoints[i].x) && GUtils::OSIsValidNumber(vPoints[i].y))
						fSumXY += vPoints[i].x * vPoints[i].y;
				real fDelta = nNumPoints * fSumXSqr - (fSumX * fSumX);
				if (fDelta == 0.0)
					*pfStdDevSlope = *pfStdDevIntercept = 0.0;	// error case just set to 0 and don;t do anything else
				else
				{ // fDelta is OK, keep going
					real fB = (fSumXSqr * fSumY - fSumX * fSumXY) / fDelta;
					real fM = (nNumPoints * fSumXY - fSumX * fSumY) / fDelta;
					real fSigmaYSqr = 0;
					for (i=0; i<nNumPoints; i++)
						if (GUtils::OSIsValidNumber(vPoints[i].x) && GUtils::OSIsValidNumber(vPoints[i].y))
						{
							real fTemp = (vPoints[i].y - fB -fM * vPoints[i].x);
							fSigmaYSqr += fTemp * fTemp;
						}
					fSigmaYSqr = fSigmaYSqr / (nNumPoints - 2);

					if (pfStdDevSlope != NULL)
						*pfStdDevSlope = sqrt(nNumPoints * fSigmaYSqr / fDelta);

					if (pfStdDevIntercept != NULL)
						*pfStdDevIntercept = sqrt(fSigmaYSqr * fSumXSqr / fDelta);
				}
			}
		}
	}

	return kResponse_OK;
}

long GUtils::CalculateLinearFit(realvector const & vX,	// x Values
							    realvector const & vY,	// y Values
							    real *pfSlope,				// [out] slope of linear fit
							    real *pfIntercept,			// [out] intercept of linear fit line
							    real *pfCorrelation,			// [out] correlation of line to data; if NULL don't fill in
							    real *pfStdDevSlope,			// [out] standard deviation of slope (uncertainity)
							    real *pfStdDevIntercept)		// [out] standard deviation of intercept (uncertainity)	
{ // RETURN kResponse_OK if fit went well, ID of error string otherwise
	// REVISIT optimize (by copying code not creating new point array)
	size_t nMin = min(vX.size(), vY.size());
	SRealPointVector vPoints(nMin);
	for (size_t i=0; i<nMin; i++)
	{
		vPoints[i].x = vX[i];
		vPoints[i].y = vY[i];
	}
	return GUtils::CalculateLinearFit(vPoints, pfSlope, pfIntercept, pfCorrelation, pfStdDevSlope, pfStdDevIntercept);  
}

bool GUtils::IsMonotonic(SRealPointVector const &vPoints)
{ // RETURN true if x component of points is monotonic (not both increasing and decreasing)
	bool bMonotonic = true;
	if (vPoints.size() > 1)
	{
		bool bNotDecreasing = (vPoints[1].x >= vPoints[0].x);
		bool bNotIncreasing = (vPoints[1].x <= vPoints[0].x);
		for (size_t ix = 2; ix < vPoints.size(); ix++)
		{
			if ((vPoints[ix].x > vPoints[ix-1].x) &&
				bNotIncreasing)
			{
				bMonotonic = false;
				break;
			}
			if ((vPoints[ix].x < vPoints[ix-1].x) &&
				bNotDecreasing)
			{
				bMonotonic = false;
				break;
			}
		}
	}

	return bMonotonic;
}

bool GUtils::IsMonotonic(realvector const &vReal)
{ // RETURN true if points are monotonic (i.e. same or increasing or same or decreasing)
	bool bMonotonic = true;
	if (vReal.size() > 1)
	{
		bool bNotDecreasing = (vReal[1] >= vReal[0]);
		bool bNotIncreasing = (vReal[1] <= vReal[0]);
		for (size_t ix = 2; ix < vReal.size(); ix++)
		{
			if ((vReal[ix] > vReal[ix-1]) &&
				bNotIncreasing)
			{
				bMonotonic = false;
				break;
			}
			if ((vReal[ix] < vReal[ix-1]) &&
				bNotDecreasing)
			{
				bMonotonic = false;
				break;
			}
		}
	}

	return bMonotonic;
}

//#include <DriverServices.h>


unsigned long GUtils::TraceClock(void)
{
	cppsstream ss;
	unsigned long nClock = (unsigned long)clock();
	ss << " clock: " << nClock << kOSNewlineChar;
	GUtils::Trace(ss.str());
	return nClock;
}

short GUtils::MakeDataChecksum(unsigned char * pBuffer,		// pointer to nSize unsigned bytes
							   unsigned short nSize)		// number of bytes to sum
{ // RETURN the 2 byte checksum of nSize 1 byte (unsigned) values
	unsigned short nSum = 0;
	
	for (unsigned short i = 0; i < nSize; i++)
		nSum += pBuffer[i];

	return nSum;
}
*/
void GUtils::Trace(void * pointer, gchar * psFile, int nLine)
{
	cppsstream ss;
	ss << pointer;
	Trace(ss.str(),psFile,nLine);
	GSTD_LOG(ss.str());
}

void GUtils::Trace(cppstring msg,
				   gchar * psFile,
				   int nLine)
{
	int nLen = 0;
	if (psFile != NULL)
	{
#ifdef _UNICODE
		nLen = wcslen(psFile);
#else
		nLen = strlen(psFile);
#endif
	}
	if ((nLen > 0) && 
		(nLine != -1)	)
	{ // Only print File and Line if we have something..
		cppsstream ss;
		ss << psFile << GSTD_S("(") << nLine << GSTD_S(") : ") << msg << kOSNewlineString;
		OSTrace(ss.str().c_str());
	}
	else
	 	OSTrace(msg.c_str());

	GSTD_LOG(msg);
}

void GUtils::Trace(gchar * msg,
				   gchar * psFile,
				   int nLine)
{
	cppstring cppmsg = msg;
	GUtils::Trace(cppmsg, psFile, nLine);
}
/*
int GUtils::GetDecimalPlacesUsed(real fValue)
{ // RETURN the number of places used by fValue
	int nPlaces = 0;
	fValue = fabs(fValue);

	real fTempValue = fabs(fValue - floor(fValue));
	while (fTempValue > GUtils::kRealMin)
	{
		nPlaces++;
		fValue = fValue * 10;
		fTempValue = fabs(fValue - floor(fValue));
	}

	return nPlaces;
}

size_t GUtils::CalculateNumberOfDivisions(real fStart,		// start value (as in manual column dialog)
										  real fEnd,		// end value
										  real fIncrement)	// increment
{ // calculate the number of divisions between start and end (inclusive)
	size_t nNum = 0;

	// Fix up increment if it's the wrong sign
	if (fStart < fEnd)
		fIncrement = fabs(fIncrement);
	if (fStart > fEnd)
		fIncrement = -fabs(fIncrement);

	// Get best guess approx at number of divisions.  If way too big, report it and get out of here...
	real fNum = fabs((fStart - fEnd) / fIncrement) + 1;
	if (fNum >= GColumn::kTooManyCells)
		nNum = GColumn::kTooManyCells;
	else
	if ((fEnd - fStart) * fIncrement > 0)	// check for all sorts of pathological cases (e.g. fIncrement == 0, fIncrement wrong sign, etc.
	{ // Do actual divisions -- this needs to be done because floating point math sucks.
		int nMag = max(GUtils::GetDecimalPlacesUsed(fStart), GUtils::GetDecimalPlacesUsed(fIncrement));
		nNum = (size_t)fabs((fEnd - fStart) / fIncrement);
		real fValue;
		if (fIncrement > 0)
			do
			{
				nNum++;
				fValue = fStart + nNum * fIncrement;
				cppsstream ss;
				ss << fixed << setprecision(nMag) << fValue;
				if (!GTextUtils::IsStringRealNumber(ss.str(), &fValue))	// converts the string
					break; 	// bullet-proof.  Should never happen.
			}
			while (fValue < fEnd + GUtils::kRealMin);
		else
			do
			{
				nNum++;
				fValue = fStart + nNum * fIncrement;
				cppsstream ss;
				ss << fixed << setprecision(nMag) << fValue;
				if (!GTextUtils::IsStringRealNumber(ss.str(), &fValue))	// converts the string
					break; 	// bullet-proof.  Should never happen.
			}
			while (fValue > fEnd - GUtils::kRealMin);
	}

	return nNum;
}

cppstring GUtils::GetUniqueIDString(void)
{ // RETURN a string that will be unique (or very, very close) to use to identify this file for clipboard operations
	cppsstream ss;
	ss << GetCurrentStdDateTimeText() << OSGetUserName() << rand() << rand();	
	return ss.str();
}

real GUtils::RestrictReal(real fValue)
{ // RETURN a real number that has been restricted to the range we define as valid for opus
	if (fValue > kRealMax)
		fValue = kRealMax;
	else
	if (fValue < -kRealMax)
		fValue = -kRealMax;
	else
	if (fabs(fValue) < kRealMin)
		fValue = 0;

	return fValue;
}

bool GUtils::RealEquivalence(real fTolerance, real fValue1, real fValue2)
{// returns true if two floats are equiv, that is equal within the tolerance

	if (((fValue1 - fTolerance) < fValue2) && (fValue2 < (fValue1 + fTolerance)))
		return true;
	else 
		return false;
}

cppstring GUtils::GetStringFromTimeUnit(ETimeUnit eTimeUnit,	// what unit
										bool bAbbreviation,		// true ==> use abbreviation, false ==> use full name
										bool bPlural)	// true ==> full name has "s" on end
{ // RETURN a string based on a time unit (either a fuill string or an abbreviation)
	cppstring sUnits;

	if (bAbbreviation)
	{
		switch(eTimeUnit)
		{
			case kTimeUnit_millisecond:
				sUnits = GSTD_STRING(IDSX_MILLISECONDS_ABBREVIATION);
				break;
				
			case kTimeUnit_second:
				sUnits = GSTD_STRING(IDSX_SECONDS_ABBREVIATION);
				break;
				
			case kTimeUnit_minute:
				sUnits = GSTD_STRING(IDSX_MINUTES_ABBREVIATION);
				break;
				
			case kTimeUnit_hour:
				sUnits = GSTD_STRING(IDSX_HOURS_ABBREVIATION);
				break;
				
			case kTimeUnit_day:
				sUnits = GSTD_STRING(IDSX_DAYS_ABBREVIATION);
				break;
				
			case kTimeUnit_year:
				sUnits = GSTD_STRING(IDSX_YEARS_ABBREVIATION);
				break;
		}
	}
	else
	{
		if (bPlural)
		{ // Full Name
			switch(eTimeUnit)
			{
				case kTimeUnit_millisecond:
					sUnits = GSTD_STRING(IDSX_MILLISECONDS);
					break;
					
				case kTimeUnit_second:
					sUnits = GSTD_STRING(IDSX_SECONDS);
					break;
					
				case kTimeUnit_minute:
					sUnits = GSTD_STRING(IDSX_MINUTES);
					break;
					
				case kTimeUnit_hour:
					sUnits = GSTD_STRING(IDSX_HOURS);
					break;
					
				case kTimeUnit_day:
					sUnits = GSTD_STRING(IDSX_DAYS);
					break;
					
				case kTimeUnit_year:
					sUnits = GSTD_STRING(IDSX_YEARS);
					break;
			}
		}
		else
		{
			switch(eTimeUnit)
			{
				case kTimeUnit_millisecond:
					sUnits = GSTD_STRING(IDSX_MILLISECOND);
					break;
					
				case kTimeUnit_second:
					sUnits = GSTD_STRING(IDSX_SECOND);
					break;
					
				case kTimeUnit_minute:
					sUnits = GSTD_STRING(IDSX_MINUTE);
					break;
					
				case kTimeUnit_hour:
					sUnits = GSTD_STRING(IDSX_HOUR);
					break;
					
				case kTimeUnit_day:
					sUnits = GSTD_STRING(IDSX_DAY);
					break;
					
				case kTimeUnit_year:
					sUnits = GSTD_STRING(IDSX_YEAR);
					break;
			}
		}
	}

	return sUnits;
}

StringVector GUtils::GetTimeUnitStrings(bool bAbbreviation,		// true ==> use abbreviation, false ==> use full name
										bool bPlural)	// true ==> full name has "s" on end
{ // RETURN a vector of all supported time strings
	StringVector vsNames;

	vsNames.push_back(GUtils::GetStringFromTimeUnit(kTimeUnit_millisecond, bAbbreviation, bPlural));
	vsNames.push_back(GUtils::GetStringFromTimeUnit(kTimeUnit_second, bAbbreviation, bPlural));
	vsNames.push_back(GUtils::GetStringFromTimeUnit(kTimeUnit_minute, bAbbreviation, bPlural));
	vsNames.push_back(GUtils::GetStringFromTimeUnit(kTimeUnit_hour, bAbbreviation, bPlural));
	
	return vsNames;
}
*/

//ETimeUnit GUtils::GetTimeUnitFromString(const cppstring & sUnit, bool /*bStrict*/)
/*
{
	// REVISIT - if bStrict is false, do case-insensitive comparison against both
	// the abbreviated & non-abbreviated strign; the code below assumes strict
	// (sUnit must exactly match our string resource, which -should- be SI).
	ETimeUnit eUnit = kTimeUnit_second;
	
	if (sUnit == GSTD_STRING(IDSX_MILLISECONDS_ABBREVIATION))
		eUnit = kTimeUnit_millisecond;
	else
	if (sUnit == GSTD_STRING(IDSX_SECONDS_ABBREVIATION))
		eUnit = kTimeUnit_second;
	else
	if (sUnit == GSTD_STRING(IDSX_MINUTES_ABBREVIATION))
		eUnit = kTimeUnit_minute;
	else
	if (sUnit == GSTD_STRING(IDSX_HOURS_ABBREVIATION))
		eUnit = kTimeUnit_hour;
	else
	if (sUnit == GSTD_STRING(IDSX_DAYS_ABBREVIATION))
		eUnit = kTimeUnit_day;
	else
	if (sUnit == GSTD_STRING(IDSX_YEARS_ABBREVIATION))
		eUnit = kTimeUnit_year;
	
	return eUnit;
}

SColumnHeader GUtils::GetTimeColumnHeader(ETimeUnit eTime)
{ // return a column header (name, short name, and units) based on the passed-in time units
	SColumnHeader timeCol;
	timeCol.sName = GSTD_STRING(IDSX_TIME);
	timeCol.sShortName = GSTD_STRING(IDSX_SHORT_TIME);
	switch(eTime)
	{
		case kTimeUnit_millisecond:
			timeCol.sUnits = GSTD_STRING(IDSX_MILLISECONDS_ABBREVIATION);
			break;
			
		case kTimeUnit_second:
			timeCol.sUnits = GSTD_STRING(IDSX_SECONDS_ABBREVIATION);
			break;
			
		case kTimeUnit_minute:
			timeCol.sUnits = GSTD_STRING(IDSX_MINUTES_ABBREVIATION);
			break;
			
		case kTimeUnit_hour:
			timeCol.sUnits = GSTD_STRING(IDSX_HOURS_ABBREVIATION);
			break;
			
		case kTimeUnit_day:
			timeCol.sUnits = GSTD_STRING(IDSX_DAYS_ABBREVIATION);
			break;
			
		case kTimeUnit_year:
			timeCol.sUnits = GSTD_STRING(IDSX_YEARS_ABBREVIATION);
			break;
	}

	return timeCol;
}

StdIDVector GUtils::GetIDVectorIntersection(StdIDVector const &vIDs1, 
											StdIDVector const &vIDs2)
{ // RETURN the IDs that are in both v1 and v2
	StdIDVector vIDs;

	for (size_t ix=0; ix<vIDs1.size(); ix++)
		if ((std::find(vIDs2.begin(), vIDs2.end(), vIDs1[ix]) != vIDs2.end()) &&
			(std::find(vIDs.begin(), vIDs.end(), vIDs1[ix]) == vIDs.end()))
			vIDs.push_back(vIDs1[ix]);

	return vIDs;
}

real GUtils::ConvertTimeToSeconds(real fTime,	// Time	
								 ETimeUnit eTimeUnit)	// Units
{ // Based on the time and the units, RETURN a time in seconds
	real fTimeInSeconds = fTime;
	switch (eTimeUnit)
	{
		case kTimeUnit_millisecond:
			fTimeInSeconds /= 1000.0;
			break;
		case kTimeUnit_minute:
			fTimeInSeconds *= 60.0;
			break;
		case kTimeUnit_hour:
			fTimeInSeconds *= 60.0*60.0;
			break;
	}

	return fTimeInSeconds;
}

real GUtils::ConvertSecondsToTime(real fTimeInSeconds,	// Time	in seconds
								 ETimeUnit eTimeUnit)	// Units
{ // Based on the time in seconds and the units, RETURN a time (undo above conversion)
	real fTime = fTimeInSeconds;
	switch (eTimeUnit)
	{
		case kTimeUnit_millisecond:
			fTime *= 1000.0;
			break;
		case kTimeUnit_minute:
			fTime /= 60.0;
			break;
		case kTimeUnit_hour:
			fTime /= 60.0*60.0;
			break;
	}

	return fTime;
}

real GUtils::ConvertRateInSecondsToTime(real fRateInSeconds, // rate per second
										ETimeUnit eTimeUnit)	// Units
{ // RETURN a new rate in eTimeUnits
	real fRate = fRateInSeconds;
	switch (eTimeUnit)
	{
		case kTimeUnit_millisecond:
			fRate /= 1000.0;
			break;
		case kTimeUnit_minute:
			fRate *= 60.0;
			break;
		case kTimeUnit_hour:
			fRate *= 60.0*60.0;
			break;
	}

	return fRate;
}
		
cppstring GUtils::ToLower(const cppstring &s)
{ // RETURN s, in lower case
	cppstring sLower = s;
	for (size_t ix=0; ix<sLower.size(); ix++)
	{
		sLower[ix] = tolower(sLower[ix]);
	}

	return sLower;
}
narrowstring GUtils::ToNarrowLower(const narrowstring &s)
{ // RETURN s, in lower case
	narrowstring sLower = s;
	for (size_t ix=0; ix<sLower.size(); ix++)
		sLower[ix] = tolower(sLower[ix]);

	return sLower;
}
*/
void GUtils::AssertDialog(const gchar * cFile, // Source (cpp) file where assert happened
						  int nLine, // Line number within above file
						  const cppstring &sStackTrace) // Stack Trace when Assert happend
{
	SAppError appError;
	cppsstream ss;
	ss << "Assert in file " << cFile << " at line " << nLine;
	appError.sErrorMessage = ss.str();
	GUtils::OSAssertDialog(&appError);

/*
	cppstring sFile = cFile;
	GSTD_LOG(GSTD_S("Assert Failed! ") + sFile + GSTD_S(" ") + GTextUtils::LongToCPPString(nLine));

	cppstring sMsg = GSTD_STRING(IDSX_ASSERT);
	cppstring sURL = GSTD_STRING(IDSX_ASSERT_LINK);
	cppstring sLine = GTextUtils::LongToCPPString(nLine);
	if (!sMsg.empty() && !sURL.empty() && !sLine.empty())
	{
		// Replace Stack Trace
		sURL = GTextUtils::StringReplace(sURL, GSTD_STRING(IDSX_ASSERT_STACKTRACE_SUB), sStackTrace);

		// Replace file
		sMsg = GTextUtils::StringReplace(sMsg, GSTD_STRING(IDSX_ASSERT_FILE_SUB), sFile);
		sURL = GTextUtils::StringReplace(sURL, GSTD_STRING(IDSX_ASSERT_FILE_SUB), sFile);
		
		// Replace line
		if (nLine > -1)
		{
			sMsg = GTextUtils::StringReplace(sMsg, GSTD_STRING(IDSX_ASSERT_LINE_SUB), sLine);
			sURL = GTextUtils::StringReplace(sURL, GSTD_STRING(IDSX_ASSERT_LINE_SUB), sLine);
		}
		
		// Replace app name and version
		sURL = GTextUtils::StringReplace(sURL, GSTD_STRING(IDSX_ASSERT_APP_SUB), GApplicationBrain::GetApplicationName());
		if (GetAppBrain() != NULL)
			sURL = GTextUtils::StringReplace(sURL, GSTD_STRING(IDSX_ASSERT_VERS_SUB), GetAppBrain()->GetVersion());
		else
			sURL = GTextUtils::StringReplace(sURL, GSTD_STRING(IDSX_ASSERT_VERS_SUB), GSTD_S(""));

		SAppError appError;
		appError.sErrorMessage = sMsg;
		appError.sURL = sURL;
		GUtils::OSAssertDialog(&appError);
	}
*/
}


bool GUtils::IsLogOpen(void)
{ // RETURN true if the log file is open and can be written to
	return GUtils::pLogOStream != NULL;
}

/*
cppstring GUtils::OpenLog(void)
{ // Open the log file and RETURN the name of the file opewned (or empty string if error)
	CloseLog(); // but first, close any existing logfile

	// Use app name and std date/time for logfile name
	cppstring sFileName = GApplicationBrain::GetApplicationName() + GSTD_S(" Log (") + GUtils::GetCurrentStdDateTimeText() + GSTD_S(").txt");
	sFileName = GTextUtils::StringReplace(sFileName, GSTD_STRING(IDSX_SEPARATOR), GSTD_S(" "));
	sFileName = GTextUtils::StringReplace(sFileName, GSTD_S(" "), GSTD_S("_"));
	sFileName = GTextUtils::StringReplace(sFileName, GSTD_S("/"), GSTD_S("_"));

	narrowstring sNarrowFileName;
#ifdef USE_WIDE_CHARS
	sNarrowFileName = GFileRef::ConvertWideToNarrow(sFileName);
#else
	sNarrowFileName = sFileName;
#endif
	try 
	{
		GSTD_NEW(pLogOStream,(cppofstream *),cppofstream(sNarrowFileName.c_str() ,ios_base::out|ios_base::app));//GUtils::pLogOStream = new std::ofstream(sFileName.c_str());
	}
	catch (OSFileExceptionPtr pFileExp)
	{ // catch any file errors
		cppstring sError = GUtils::OSGetExceptionErrorString(pFileExp);
		GUtils::OSDeleteExceptionPtr(pFileExp);

		// Tell user what happened
		GUtils::MessageBox(sError + sFileName);
		sFileName = GSTD_S("");
	}

	return sFileName;
}

void GUtils::CloseLog(void)
{ // Close any open logfile
	if (GUtils::pLogOStream != NULL)
	{ 
		GUtils::pLogOStream->close();
		delete GUtils::pLogOStream;
	}
	GUtils::pLogOStream = NULL;
}
*/
void GUtils::WriteToLog(cppstring sText,	// The text to write
						cppstring sPath,	// Path and Filename of calling method
						int nLine,			// line number of calling method
						cppstring /*sFunction*/)	// Function name of calling	method (supported only on Mac)
{ // Write some passed-in text to the logfile
	if (GUtils::pLogOStream != NULL)
	{
		// Strip the pathname from file
//		cppstring sFile = GTextUtils::StripPath(sPath);
//		cppstring sLine = GTextUtils::LongToCPPString(nLine);
//
//		*GUtils::pLogOStream << GUtils::GetCurrentStdTimeText() << GSTD_S(" ") << sFile << GSTD_S(":") << sLine << " " << sText << kOSNewlineString;
//		GUtils::pLogOStream->flush();
	}
}

/*
void GUtils::WriteAddressToLog(void * pointer,	// pointer value to write
							   cppstring sPath,	// file and path name	
							   int nLine)		// line in file
{
	// Write passed in address  to the logfile
	if (GUtils::pLogOStream != NULL)
	{
		// Strip the pathname from file
		cppstring sFile = GTextUtils::StripPath(sPath);
		cppstring sLine = GTextUtils::LongToCPPString(nLine);

		*GUtils::pLogOStream << GUtils::GetCurrentStdTimeText() << GSTD_S(" Allocated memory at:") << pointer   << GSTD_S(" in ") << sFile << GSTD_S(" ") << sLine << GSTD_S(" data: ");
		*GUtils::pLogOStream << kOSNewlineString;
		GUtils::pLogOStream->flush();
	}
}
*/
/*
realvector GUtils::ConvertStringToRealVector(const cppstring & sText)
{
	realvector vfVals;

	cppsstream ss;
	real fValue;
	int numItems = 0;
	
	ss << sText;
	ss >> numItems;
	for (int i = 0; i < numItems; i++)
	{
		ss >> fValue;
		vfVals.push_back(fValue);
	}

	return vfVals;
}

cppstring GUtils::ConvertRealVectorToString(const realvector & vfVals)
{
	cppsstream ss;

	ss << vfVals.size() << " ";
	for (size_t ix = 0; ix < vfVals.size(); ix++)
		ss << vfVals[ix] << " ";

	return ss.str();
}

intVector GUtils::ConvertStringToIntVector(const cppstring & sText)
{
	intVector vnVals;

	cppsstream ss;
	int nValue;
	int numItems = 0;
	
	ss << sText;
	ss >> numItems;
	for (int i = 0; i < numItems; i++)
	{
		ss >> nValue;
		vnVals.push_back(nValue);
	}

	return vnVals;
}

cppstring GUtils::ConvertIntVectorToString(const intVector & vnVals)
{
	cppsstream ss;

	ss << vnVals.size() << " ";
	for (size_t ix = 0; ix < vnVals.size(); ix++)
		ss << vnVals[ix] << " ";

	return ss.str();
}

cppstring GUtils::ConvertBytesToString(unsigned char *pBytes, // pointer to an array of bytes
									   int nNumBytes)	// the number of bytes to convert
{
static char cHexDigits[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	cppsstream ss;
	for (int i= 0; i < nNumBytes; i++)
	{
		ss << cHexDigits[(pBytes[i] & 0xF0) >> 4] << cHexDigits[(pBytes[i] & 0x0F)];
	}

	return ss.str();
}

// routines for getting the correct application name (and all other strings that would contain loggerPro, loggerlite etc.
cppstring GUtils::GetApplicationString(const cppstring & sKey)
{
	cppstring sFullKey = sKey + GSTD_S("_");
	#ifdef OPUS_LITE
		sFullKey += GSTD_S("LL");
	#else
		sFullKey += GSTD_S("LP");
	#endif
	return GTextUtils::GetStringByKey(sFullKey.c_str());
}

cppstring GUtils::GetApplicationISBN(void)
{
#ifdef OPUS_LITE
	return GSTD_STRING(IDSX_LL_APPLICATION_ISBN);
#else
	return GSTD_STRING(IDSX_LP_APPLICATION_ISBN);
#endif
}

cppstring GUtils::GetNativeFileExtension(void)
{
#ifdef OPUS_LITE
	return GSTD_STRING(IDSX_NATIVE_FILE_EXTENSION_LL);
#else
	return GSTD_STRING(IDSX_NATIVE_FILE_EXTENSION_LP);
#endif
}

cppstring GUtils::GetNativeFileFilter(bool bOpen)
{ // This is a windows specific string
#ifdef OPUS_LITE
	if (bOpen) // both LP and LL use the same open filter
		return GSTD_STRING(IDSX_NATIVE_FILE_OPEN_FILTER_LL);
	else
		return GSTD_STRING(IDSX_NATIVE_FILE_SAVE_FILTER_LL);
#else
	if (bOpen) // both LP and LL use the same open filter
		return GSTD_STRING(IDSX_NATIVE_FILE_OPEN_FILTER_LP);
	else
		return GSTD_STRING(IDSX_NATIVE_FILE_SAVE_FILTER_LP);
#endif
}		

// Calculate the size of the string vector by turning it into a string and drawing it.  Don't forget to add the 
//  size of the seperator that will seperate each of the strings.
int  GUtils::CalculateStringVectorSize(OSPtr pOSData, int nSeperatorSize, const StringVector & vsStrings)	
{
	int nSize = 0;
#ifdef TARGET_OS_WIN
	GSTD_ASSERT(pOSData != NULL);
#endif
	if (vsStrings.size() > 0) 
	{
		cppstring sConcatString;
		int nSepTotal = nSeperatorSize * (vsStrings.size() - 1);
		for (size_t ix = 0; ix < vsStrings.size(); ix++)
			sConcatString += vsStrings[ix];
		nSize = GDrawing::OSCalcTextWidth(pOSData, sConcatString) + nSepTotal;
	}
	
	return nSize;
}

// With one string vector full of long strings and one full of short string start make a string vector that uses
//  as many of the short strings as needed to stay under the ideal size.  Yet use as many of the long strings as you can.
StringVector  GUtils::GenerateIdealStringVector(OSPtr pOSData, int nIdealSize, int *nActualSize, const StringVector & vsLongStrings, const StringVector & vsShortStrings, int nLongSize, int nShortSize)	
{
	StringVector	vsMixedStrings;
	*nActualSize = nLongSize;

	if (nIdealSize != -1 && nLongSize > nIdealSize)
	{
		if (nShortSize > nIdealSize)
		{	//Just return the short size because we cannot get any shorter than that.
			*nActualSize = nShortSize;
			vsMixedStrings = vsShortStrings;
		}
		else
		{
			//Go back through the vectors using the shorter of the two strings until we get enough space back.
			int nOverage = nLongSize - nIdealSize;
			for (size_t ix = 0; ix < vsLongStrings.size(); ix++)
			{
				cppstring sLong = vsLongStrings[ix];
				cppstring sShort = vsShortStrings[ix];

				if (nOverage > 0)
				{	// Use the short version and subtract off the difference
					vsMixedStrings.push_back(sShort);
					int nMax = GDrawing::OSCalcTextWidth(pOSData, sLong);
					int nMin = GDrawing::OSCalcTextWidth(pOSData, sShort);

					nOverage -= nMax - nMin;
					*nActualSize -= nMax - nMin;
				}
				else
				{
					vsMixedStrings.push_back(sLong);
				}
			}
		}
	}
	else
		vsMixedStrings = vsLongStrings; 

	return vsMixedStrings;
}


//======================== MBL Live Readouts class ======================//
StStopMBLActivity::StStopMBLActivity(void)  
{
	m_bRestartLiveReadouts = false;
	if ((GetAppBrain() != NULL) && (GetAppBrain()->GetMBLBrain() != NULL))
	{
		m_bRestartLiveReadouts = GetMBLBrain()->IsGettingLiveReadouts();
		if (m_bRestartLiveReadouts)
			GetMBLBrain()->StopLiveReadouts();
	}
}

StStopMBLActivity::~StStopMBLActivity()
{
	if ((GetAppBrain() != NULL) && 
		(GetAppBrain()->GetMBLBrain() != NULL) &&
		!GetMBLBrain()->IsUserCollecting())
	{
		// Restart live readouts if appropriate
		if (m_bRestartLiveReadouts &&
			!GetMBLBrain()->IsGettingLiveReadouts() &&
			GetMBLBrain()->IsLiveReadoutsWanted())
			GetMBLBrain()->StartLiveReadouts();
	}
}
*/