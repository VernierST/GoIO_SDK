// GMiniGCDevice.cpp

#include "stdafx.h"
#include "GMiniGCDevice.h"

#include "GUtils.h"

#ifdef _DEBUG
#include "GPlatformDebug.h" // for DEBUG_NEW definition
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

StringVector GMiniGCDevice::m_snapshotOfAvailableMiniGCDevices;

GMiniGCDevice::GMiniGCDevice(GPortRef *pPortRef)
: TBaseClass(pPortRef)
{
	if (!OSInitialize())
		GUtils::Trace(GSTD_S("Error - GMiniGCDevice constructor, OSInitialize() returned false."));
}

