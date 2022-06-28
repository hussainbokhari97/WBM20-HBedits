/******************************************************************************

GHAAS Water Balance Model Library V1.0
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDIrrigation_ReferenceET-Hamon.c

dominik.wisser@unh.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDInDayLengthID     = MFUnset;
static int _MDInCommon_AtMeanID = MFUnset;
static int _MDOutPetID          = MFUnset;

static void _MDIrrRefEvapotransHamon (int itemID) {
// Hamon (1963) PE in mm for day
// Input
	float dayLen = MFVarGetFloat (_MDInDayLengthID,     itemID, 12.0); // daylength in fraction of day
	float airT   = MFVarGetFloat (_MDInCommon_AtMeanID, itemID,  0.0); // air temperatur [degree C]
// Output
	float pet;
// Local
	float rhoSat;	// saturated vapor density [kg/m3]

	rhoSat = 2.167 * MDPETlibVPressSat (airT) / (airT + 273.15);
	pet = 165.1 * 2.0 * dayLen * rhoSat; // 2 * DAYLEN = daylength as fraction of 12 hours
	MFVarSetFloat (_MDOutPetID,itemID,pet);
}

int MDIrrigation_Reference_ETHamonDef () {
	if (_MDOutPetID != MFUnset) return (_MDOutPetID);

	MFDefEntering ("Hamon as ReferenceETP");
	if (((_MDInDayLengthID      = MDCommon_SolarRadDayLengthDef()) == CMfailed) ||
        ((_MDInCommon_AtMeanID  = MDCommon_AirTemperatureDef ())   == CMfailed) ||
        ((_MDOutPetID           = MFVarGetID (MDVarIrrigation_RefEvapotrans, "mm", MFOutput, MFFlux, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDIrrRefEvapotransHamon) == CMfailed)) return (CMfailed);
	MFDefLeaving ("Hamon as ReferenceETP");
	return (_MDOutPetID);
}
