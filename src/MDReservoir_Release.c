/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDReservoir_Release.c

bfekete@gc.cuny.edu

******************************************************************************/

#include <MF.h>
#include <MD.h>
 
// Output
static int _MDOutResReleaseID            = MFUnset;
static int _MDOutResReleaseExtractableID = MFUnset;
static int _MDOutResReleaseBottomID      = MFUnset;
static int _MDOutResReleaseSpillwayID    = MFUnset;

int MDReservoir_ReleaseDef () {
	int optID = MFoff;
	const char *optStr;

	if (_MDOutResReleaseID != MFUnset) return (_MDOutResReleaseID);

	MFDefEntering ("Reservoir release");
	if ((optStr = MFOptionGet (MDOptConfig_Reservoirs)) != (char *) NULL) optID = CMoptLookup (MFswitchOptions, optStr, true);
 	switch (optID) {
		default:
		case MFhelp: MFOptionMessage (MDOptConfig_Reservoirs, optStr, MFswitchOptions); return (CMfailed);
		case MFoff:
			if ((_MDOutResReleaseID = MDRouting_ChannelDischargeDef()) == CMfailed) return (CMfailed);
			break; 
		case MFon:
			if ((_MDOutResReleaseID = MDReservoir_OperationDef ()) == CMfailed) return (CMfailed);
			break;
	}
	MFDefLeaving ("Reservoir release");
	return (_MDOutResReleaseID); 
}

int MDReservoir_ReleaseBottomDef () {
	int optID = MFoff;
	const char *optStr;

	if (_MDOutResReleaseBottomID != MFUnset) return (_MDOutResReleaseBottomID);
	if ((_MDOutResReleaseID = MDReservoir_OperationDef ()) == CMfailed) return (CMfailed);
	else if (_MDOutResReleaseID != MFUnset) {
		MFDefEntering ("Bottom release");
		_MDOutResReleaseBottomID = MFVarGetID (MDVarReservoir_ReleaseBottom, "m3/s", MFOutput, MFState, MFBoundary);
		MFDefLeaving ("Bottom release");
	}
	return (_MDOutResReleaseBottomID);
}

int MDReservoir_ReleaseSpillwayDef () {
	int optID = MFoff;
	const char *optStr;

	if (_MDOutResReleaseSpillwayID != MFUnset) return (_MDOutResReleaseSpillwayID);
	if ((_MDOutResReleaseID = MDReservoir_OperationDef ()) == CMfailed) return (CMfailed);
	else if (_MDOutResReleaseID != MFUnset) {
		MFDefEntering ("Spillway release");
		_MDOutResReleaseSpillwayID = MFVarGetID (MDVarReservoir_ReleaseSpillway, "m3/s", MFOutput, MFState, MFBoundary);
		MFDefLeaving ("Spillway release");
	}
	return (_MDOutResReleaseSpillwayID);
}

int MDReservoir_ReleaseExtractableDef () {
	int optID = MFoff;
	const char *optStr;

	if (_MDOutResReleaseExtractableID != MFUnset) return (_MDOutResReleaseExtractableID);
	if ((_MDOutResReleaseID = MDReservoir_OperationDef ()) == CMfailed) return (CMfailed);
	else if (_MDOutResReleaseID != MFUnset) {
		MFDefEntering ("Extractable release");
		_MDOutResReleaseExtractableID = MFVarGetID (MDVarReservoir_ReleaseExtractable, "m3/s", MFRoute, MFState,  MFBoundary);
		MFDefLeaving ("Extractable release");
	}
	return (_MDOutResReleaseExtractableID);
}
