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
// GPortRef
//
// GPortRef is a class which allows a reference to
// any sort of "port" to be stored; the "port" may be a
// device, a connection, or any other i/o portal.
//
// On many systems, devices such as serial ports and USB ports
// can be sufficiently identified by a string representing the
// driver name or the path to a device file; however this varies
// across systems and also does not address more abstract kinds
// of ports that may be desired, such as a  TCP/IP socket.
// GPortRef allows additional data to be used in specifying a port.
// It also allows separate storage for a string-based port location
// and a string for display of the port name to users.
//
// A GPortRef object does not provide any actual i/o facilities;
// see GDeviceIO and its subclasses.  GDeviceIO owns a GPortRef
// which it uses to locate the system representation of the port
// when opening, closing, or configuring it.
//
// The default implementation of GPortRef simply uses a string to
// store the port location.  As the need arises GPortRef may be
// subclassed for other types of ports, or its OSData field may
// be used to store a pointer to an OS-specific structure.

#ifndef _GPORTREF_H_
#define _GPORTREF_H_

#include "GTypes.h"
#include "GTextUtils.h"

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
#endif

typedef enum
{
	kPortType_Invalid = -1,
	kPortType_Serial,
	kPortType_USB
} EPortType;
typedef std::vector<EPortType> EPortTypeVector;

class GPortRef OS_STANDARD_BASE_CLASS
{
public:
				GPortRef() { m_ePortType = kPortType_Invalid; m_USBVendorID = 0; m_USBProductID = 0; }
				GPortRef(EPortType eType, cppstring sLoc = GSTD_S(""), cppstring sDis = GSTD_S(""), unsigned int vID = 0, unsigned int pID = 0) { m_ePortType = eType; m_sLocation = sLoc; m_sDisplayName = sDis; m_USBVendorID = vID; m_USBProductID = pID; }
				GPortRef(const GPortRef & source);	// copy constructor

	virtual		~GPortRef(void) {;}
	
	GPortRef	&operator=(const GPortRef &source);
	virtual GPortRef & Assign(const GPortRef & src);
	
	bool		IsValidPortRef(void) const { return ((GetPortType() != kPortType_Invalid) && (m_sDisplayName.length() > 0)); }
	EPortType	GetPortType(void) const { return m_ePortType; }
	void		SetPortType(EPortType eType) { m_ePortType = eType; }
	
	cppstring	GetLocation(void) const { return m_sLocation; }
	void		SetLocation(const cppstring & sLocation) { m_sLocation = sLocation; }
	
	cppstring *	GetDisplayNamePtr(void) const { return const_cast<cppstring *>(&m_sDisplayName); }
	cppstring	GetDisplayName(void) const { return m_sDisplayName; }
	void		SetDisplayName(const cppstring & sDisplayName) { m_sDisplayName = sDisplayName; }
	
	void		EncodeToString(cppstring * pOutString);
	void		EncodeToStream(cppostream * pOutStream);
	
	int			DecodeFromString(const cppstring & sInString);
	int			DecodeFromStream(cppistream * pInStream);

	unsigned int	GetUSBVendorID(void) const { return m_USBVendorID; }
	void			SetUSBVendorID(unsigned int vendorID) { m_USBVendorID = vendorID; }
	
	unsigned int	GetUSBProductID(void) const { return m_USBProductID; }
	void			SetUSBProductID(unsigned int productID) { m_USBProductID = productID; }
	
	unsigned int	GetUniqueDeviceID(void) const { return GTextUtils::CPPStringToLong(m_sDisplayName); }

private:

	// Used for all devices (sometimes just the display name)
	EPortType		m_ePortType;
	cppstring		m_sLocation;
	cppstring		m_sDisplayName;

	// Used only for USB devices
	unsigned int	m_USBVendorID;
	unsigned int	m_USBProductID;
};

#ifdef LIB_NAMESPACE
}
#endif

#endif // _GPORTREF_H_

