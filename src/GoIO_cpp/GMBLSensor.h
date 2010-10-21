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
// GMBLSensor.h
//
// A GMBLSensor object represents a "live" sensor, i.e., a sensor
// that has been configured for a channel in an interface.

#ifndef _GMBLSENSOR_H_
#define _GMBLSENSOR_H_

#include "GTypes.h"
#include "GSensorDDSMem.h"
#include "NonSmartSensorDDSRecs.h"

class GMBLSensor
{
public:
							GMBLSensor(void);
	virtual					~GMBLSensor();

	GSensorDDSRec			*GetDDSRecPtr() { return &m_sensorDDSRec ; }

	void					SetDDSRec(const GSensorDDSRec &rec, bool bUnMarshall)
							{
								if (bUnMarshall)
									UnMarshallDDSRec(&m_sensorDDSRec, rec);
								else
									m_sensorDDSRec = rec;
							}
	void					GetDDSRec(GSensorDDSRec *pRec)
							{
								*pRec = m_sensorDDSRec;
							}

	static void				UnMarshallDDSRec(GSensorDDSRec *pPlatformEndianOutputRec, const GSensorDDSRec &littleEndianInputRec);
	static void				MarshallDDSRec(GSensorDDSRec *pLittleEndianOutputRec, const GSensorDDSRec &platformEndianInputRec);

	int						GetID(void)
							{
								return m_sensorDDSRec.SensorNumber;
							}
	void					SetID(int id)
							{
								if (id < 0)
									id = 0;
								else
								if (id > 255)
									id = 0;
								if (id != m_sensorDDSRec.SensorNumber)
								{
									if (0 == id)
										m_sensorDDSRec.SensorNumber = 0;//Leave the rest of the record alone.
									else if (id >= kSensorIdNumber_FirstSmartSensor)
										m_sensorDDSRec.SensorNumber = id;//Caller needs to read DDS memory!
									else	//Load default values that describe the non smart sensor.
										m_sensorDDSRec = g_analogSensorDefaultDDSRecs[id];
								}
							}
	bool					IsSmartSensor(void) { return (GetID() >= kSensorIdNumber_FirstSmartSensor); }


	EProbeType				GetProbeType() 
							{
								EProbeType probeType;
								if (kSensorIdNumber_GoMotion == m_sensorDDSRec.SensorNumber)
									probeType = kProbeTypeMD;
								else
								if (kSensorIdNumber_GoTemp == m_sensorDDSRec.SensorNumber)
									probeType = kProbeTypeAnalog5V;
								else
								if (2 == m_sensorDDSRec.OperationType)
									probeType = kProbeTypeAnalog10V;
								else
									probeType = kProbeTypeAnalog5V;

								return probeType;
							}
//	void					SetProbeType(EProbeType probeType)
//							{
//								if ((probeType != GetProbeType()) && (kSensorIdNumber_GoMotion != m_sensorDDSRec.SensorNumber) 
//									&& (kSensorIdNumber_GoTemp != m_sensorDDSRec.SensorNumber))
//									//Make sure probeType is consistent with m_pSensorDDSRec->OperationType.
//									m_sensorDDSRec.OperationType = (kProbeTypeAnalog10V == probeType) ? 2 : 14;
//							}
	cppstring				GetUnits(void);

	real					CalibrateData(real fRawVolts);

	
	// DDS burning/reading methods
	static unsigned char	CalculateDDSDataChecksum(const GSensorDDSRec &rec);
	static bool				VerifyDDSChecksum(const GSensorDDSRec &rec, bool bStrictChecksumRequired);
	
private:
	GSensorDDSRec			m_sensorDDSRec;
};

#endif // _GMBLSENSOR_H_
