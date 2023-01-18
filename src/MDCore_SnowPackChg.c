/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

MDCore_SnowPackChg.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <string.h>
#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_AtMeanID   = MFUnset;
static int _MDInCommon_PrecipID   = MFUnset;

// Output
static int _MDOutSnowPackID       = MFUnset;
static int _MDOutSPackChgID       = MFUnset;
static int _MDOutSnowMeltID       = MFUnset;
static int _MDOutSnowFallID       = MFUnset;

static float _MDSnowMeltThreshold =  1.0;
static float _MDFallThreshold     = -1.0;

static void _MDSPackChg (int itemID) {
// Input
	float airT   = MFVarGetFloat (_MDInCommon_AtMeanID, itemID, 0.0);
	float precip = MFVarGetFloat (_MDInCommon_PrecipID, itemID, 0.0);
// Initial
	float sPack  = MFVarGetFloat (_MDOutSnowPackID,     itemID, 0.0);
// Output
	float sMelt = 0.0;

	if (airT < _MDFallThreshold) {  /* Accumulating snow pack */
		MFVarSetFloat (_MDOutSnowFallID, itemID, precip);
		MFVarSetFloat (_MDOutSnowMeltID, itemID, 0.0);
		MFVarSetFloat (_MDOutSnowPackID, itemID, sPack + precip);
		MFVarSetFloat (_MDOutSPackChgID, itemID, precip);
	}
	else if (airT > _MDSnowMeltThreshold) { /* Melting snow pack */
		sMelt = 2.63 + 2.55 * airT + 0.0912 * airT * precip;
		sMelt = sMelt < sPack ? sMelt : sPack;
		MFVarSetFloat (_MDOutSnowFallID, itemID, 0.0);
		MFVarSetFloat (_MDOutSnowMeltID, itemID,  sMelt);
		MFVarSetFloat (_MDOutSnowPackID, itemID,  sPack - sMelt);
		MFVarSetFloat (_MDOutSPackChgID, itemID, -sMelt);
	}
	else { /* No change when air temperature is in [-1.0,1.0] range */
		MFVarSetFloat (_MDOutSnowFallID, itemID, 0.0);
		MFVarSetFloat (_MDOutSnowMeltID, itemID, 0.0);
		MFVarSetFloat (_MDOutSnowPackID, itemID, sPack);	
		MFVarSetFloat (_MDOutSPackChgID, itemID, 0.0);
	}
}

int MDCore_SnowPackChgDef () {
	const char *optStr;
	float par;

	if (_MDOutSPackChgID != MFUnset) return (_MDOutSPackChgID);

	MFDefEntering ("Snow Pack Change");
	if ((optStr = MFOptionGet (MDParSnowMeltThreshold))  != (char *) NULL) {
		if (strcmp(optStr,MFhelpStr) == 0) CMmsgPrint (CMmsgInfo,"%s = %f", MDParSnowMeltThreshold, _MDSnowMeltThreshold);
		_MDSnowMeltThreshold = sscanf (optStr,"%f",&par) == 1 ? par : _MDSnowMeltThreshold;
	}
	if ((optStr = MFOptionGet (MDParSnowFallThreshold))  != (char *) NULL) {
		if (strcmp(optStr,MFhelpStr) == 0) CMmsgPrint (CMmsgInfo,"%s = %f", MDParSnowFallThreshold, _MDFallThreshold);
		_MDFallThreshold = sscanf (optStr,"%f",&par) == 1 ? par : _MDFallThreshold;
	}
	if (((_MDInCommon_PrecipID = MDCommon_PrecipitationDef ())  == CMfailed) ||
        ((_MDInCommon_AtMeanID = MDCommon_AirTemperatureDef ()) == CMfailed) ||
        ((_MDOutSnowFallID     = MFVarGetID (MDVarCommon_SnowFall,     "mm", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDOutSnowMeltID     = MFVarGetID (MDVarCore_SnowMelt,       "mm", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDOutSnowPackID     = MFVarGetID (MDVarCore_SnowPack,       "mm", MFOutput, MFState, MFInitial))  == CMfailed) ||
        ((_MDOutSPackChgID     = MFVarGetID (MDVarCore_SnowPackChange, "mm", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDSPackChg) == CMfailed)) return (CMfailed);
	MFDefLeaving ("Snow Pack Change");
	return (_MDOutSPackChgID);
}

int MDCore_SnowPackDef () {

	if (_MDOutSnowPackID != MFUnset) return (_MDOutSnowPackID);

	if ((MDCore_SnowPackChgDef () == CMfailed) ||
        ((_MDOutSnowPackID = MFVarGetID (MDVarCore_SnowPack,       "mm", MFOutput, MFState, MFInitial))  == CMfailed))
		return (CMfailed);
	return (_MDOutSnowPackID);
}

int MDCore_SnowPackMeltDef () {

	if (_MDOutSnowMeltID != MFUnset) return (_MDOutSnowMeltID);

	if ((MDCore_SnowPackChgDef () == CMfailed) ||
        ((_MDOutSnowMeltID = MFVarGetID (MDVarCore_SnowMelt, "mm", MFInput, MFFlux, MFBoundary)) == CMfailed))
		return (CMfailed);
	return (_MDOutSnowMeltID);
}
