// GPortRef.cpp
#include "stdafx.h"
#include "GPortRef.h"
#include "GUtils.h"
#include "GTextUtils.h"

#ifdef _DEBUG
#include "GPlatformDebug.h" // for DEBUG_NEW definition
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace {

const cppstring k_sPortRefCode = "PortRef";
const cppstring k_sPortTypeCode = "PortType";
const cppstring k_sLocationCode = "Location";
const cppstring k_sDisplayNameCode = "DisplayName";

} // local namespace

GPortRef &GPortRef::operator=(const GPortRef &source)
{
	m_ePortType = source.GetPortType();
	m_sLocation = source.GetLocation();
	m_sDisplayName = source.GetDisplayName();
	m_USBVendorID = source.GetUSBVendorID(); 
	m_USBProductID = source.GetUSBProductID(); 

	return *this;
}

GPortRef::GPortRef(const GPortRef & source)
{
	m_ePortType = source.GetPortType();
	m_sLocation = source.GetLocation();
	m_sDisplayName = source.GetDisplayName();
	m_USBVendorID = source.GetUSBVendorID(); 
	m_USBProductID = source.GetUSBProductID(); 
}

GPortRef & GPortRef::Assign(const GPortRef & src)
{
	SetPortType(src.GetPortType());
	SetLocation(src.GetLocation());
	SetDisplayName(src.GetDisplayName());
	SetUSBVendorID(src.GetUSBVendorID()); 
	SetUSBProductID(src.GetUSBProductID()); 

	return (*this);
}

void GPortRef::EncodeToString(cppstring * pOutString)
{
	if (pOutString != NULL)
	{
		cppsstream ss;
		EncodeToStream(&ss);
		*pOutString = ss.str();
	}
}

void GPortRef::EncodeToStream(cppostream * pOutStream)
{
	// We use a single XML tag to encode the port ref to the stream.
	// The tag is self-terminating; all fields are attributes.
	// Note that the DECODE method does not require all fields to be
	// present and does not care about their order.
	
	// REVISIT - use GXMLUtils to just build an element an write it.
	if (pOutStream != NULL)
	{
		*pOutStream << GSTD_S("<") << k_sPortRefCode << GSTD_S(" ");
		*pOutStream << k_sPortTypeCode << GSTD_S("=\"") << GetPortType() << GSTD_S("\" ");
		*pOutStream << k_sLocationCode << GSTD_S("=\"") << GetLocation() << GSTD_S("\" ");
		*pOutStream << k_sDisplayNameCode << GSTD_S("=\"") << GetDisplayName() << GSTD_S("\" ");
		*pOutStream << "/>";
	}
}

int GPortRef::DecodeFromString(const cppstring & sInString)
{
	int nResult = kResponse_Error;
	
	cppstring::size_type nSearchStartPos = 0;
	cppstring::size_type nCurrentPos = 0;
	cppstring::size_type nNextPos = 0;
	cppstring::size_type nTestPos = 0;
	
	// ensure that we have a GPortRef tag... 
	nSearchStartPos = sInString.find(k_sPortRefCode, 0);
	if (nSearchStartPos != cppstring::npos)
		nSearchStartPos += k_sPortRefCode.length();
	if (nSearchStartPos >= sInString.length())
		nSearchStartPos = cppstring::npos;
			
	if (nSearchStartPos != cppstring::npos)
		nTestPos = sInString.find_last_of('>');
	
	if (nTestPos != cppstring::npos)
	{
		cppstring sSubStr;
		
		// decode port-type
		nCurrentPos = sInString.find(k_sPortTypeCode, nSearchStartPos);
		nCurrentPos += k_sPortTypeCode.length();
		if (nCurrentPos < sInString.length())
			nCurrentPos = sInString.find('\"', nCurrentPos);
		else
			nCurrentPos = cppstring::npos;
			
		if (nCurrentPos != cppstring::npos)
			nNextPos = nCurrentPos + 1;	
		if (nNextPos != cppstring::npos)
			nNextPos = sInString.find('\"', nNextPos);
		if (	nCurrentPos != cppstring::npos &&
			nNextPos != cppstring::npos &&
			nCurrentPos < nNextPos)
			sSubStr = sInString.substr(nCurrentPos + 1, (nNextPos - nCurrentPos - 1));
		if (sSubStr.length() > 0)
			m_ePortType = (EPortType) GTextUtils::CPPStringToLong(sSubStr.c_str());
		sSubStr = GSTD_S("");
		
		// decode location
		nCurrentPos = sInString.find(k_sLocationCode, nSearchStartPos);
		nCurrentPos += k_sLocationCode.length();
		if (nCurrentPos < sInString.length())
			nCurrentPos = sInString.find('\"', nCurrentPos);
		else
			nCurrentPos = cppstring::npos;
			
		if (nCurrentPos != cppstring::npos)
			nNextPos = nCurrentPos + 1;	
		if (nNextPos != cppstring::npos)
			nNextPos = sInString.find('\"', nNextPos);
		if (	nCurrentPos != cppstring::npos &&
			nNextPos != cppstring::npos &&
			nCurrentPos < nNextPos)
			sSubStr = sInString.substr(nCurrentPos + 1, (nNextPos - nCurrentPos - 1));
		if (sSubStr.length() > 0)
			m_sLocation = sSubStr;
		sSubStr = GSTD_S("");
			
		// decode display-name
		nCurrentPos = sInString.find(k_sDisplayNameCode, nSearchStartPos);
		nCurrentPos += k_sDisplayNameCode.length();
		if (nCurrentPos < sInString.length())
			nCurrentPos = sInString.find('\"', nCurrentPos);
		else
			nCurrentPos = cppstring::npos;
			
		if (nCurrentPos != cppstring::npos)
			nNextPos = nCurrentPos + 1;	
		if (nNextPos != cppstring::npos)
			nNextPos = sInString.find('\"', nNextPos);
		if (nCurrentPos != cppstring::npos &&
			nNextPos != cppstring::npos &&
			nCurrentPos < nNextPos)
			sSubStr = sInString.substr(nCurrentPos + 1, (nNextPos - nCurrentPos - 1));
		if (sSubStr.length() > 0)
			m_sDisplayName = sSubStr;
		sSubStr = GSTD_S("");
	}
	return nResult;
}

int GPortRef::DecodeFromStream(cppistream * pInStream)
{
	// REVISIT - use GXMLUtils to extract the element
	int nResult = kResponse_Error;
	if (pInStream != NULL)
	{
		// Get the text from the in-stream up until the next ">"...
		cppstring sText;
		std::getline(*pInStream, sText, GSTD_S('>'));
		if (sText.length() > 0)
		{
			sText += GSTD_S('>');
			nResult = DecodeFromString(sText);
		}
	}
	return nResult;
}
