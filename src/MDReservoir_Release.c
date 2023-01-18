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

	if ((optStr = MFOptionGet (MDOptConfig_Reservoirs)) != (char *) NULL) optID = CMoptLookup (MFswitchOptions, optStr, true);
 	switch (optID) {
		default:
		case MFoff:
			if ((_MDOutResReleaseID = MDRouting_ChannelDischargeDef()) == CMfailed) return (CMfailed);
			break; 
		case MFon:
			MFDefEntering ("Reservoir release");
			if ((_MDOutResReleaseID = MDReservoir_OperationDef ()) == CMfailed) return (CMfailed);
			MFDefLeaving ("Reservoir release");
			break;
	}
	return (_MDOutResReleaseID); 
}

int MDReservoir_ReleaseBottomDef () {
	int optID = MFoff;
	const char *optStr;

	if (_MDOutResReleaseBottomID != MFUnset) return (_MDOutResReleaseBottomID);
	if ((optStr = MFOptionGet (MDOptConfig_Reservoirs)) != (char *) NULL) optID = CMoptLookup (MFswitchOptions, optStr, true);
	if (optID == MFon) {
		MFDefEntering ("Bottom release");
		if (((_MDOutResReleaseID = MDReservoir_OperationDef ()) == CMfailed) ||
		    ((_MDOutResReleaseBottomID = MFVarGetID (MDVarReservoir_ReleaseBottom, "m3/s", MFOutput, MFState, MFBoundary)) == CMfailed))
			return (CMfailed);
		MFDefLeaving ("Bottom release");

	}
	return (_MDOutResReleaseBottomID);
}

int MDReservoir_ReleaseSpillwayDef () {
	int optID = MFoff;
	const char *optStr;

	if (_MDOutResReleaseSpillwayID != MFUnset) return (_MDOutResReleaseSpillwayID);
	if ((optStr = MFOptionGet (MDOptConfig_Reservoirs)) != (char *) NULL) optID = CMoptLookup (MFswitchOptions, optStr, true);
	if (optID == MFon) {
		MFDefEntering ("Spillway release");
		if (((_MDOutResReleaseID = MDReservoir_OperationDef ()) == CMfailed) ||
			((_MDOutResReleaseSpillwayID = MFVarGetID (MDVarReservoir_ReleaseSpillway, "m3/s", MFOutput, MFState, MFBoundary)) == CMfailed))
			return (CMfailed);
		MFDefLeaving ("Spillway release");
	}
	return (_MDOutResReleaseSpillwayID);
}

int MDReservoir_ReleaseExtractableDef () {
	int optID = MFoff;
	const char *optStr;

	if (_MDOutResReleaseExtractableID != MFUnset) return (_MDOutResReleaseExtractableID);
	if ((optStr = MFOptionGet (MDOptConfig_Reservoirs)) != (char *) NULL) optID = CMoptLookup (MFswitchOptions, optStr, true);
	if (optID == MFon) {
		MFDefEntering ("Extractable release");
		if (((_MDOutResReleaseID = MDReservoir_OperationDef ()) == CMfailed) ||
		    ((_MDOutResReleaseExtractableID = MFVarGetID (MDVarReservoir_ReleaseExtractable, "m3/s", MFRoute, MFState,  MFBoundary)) == CMfailed))
			return (CMfailed);
		MFDefLeaving ("Extractable release");
	}
	return (_MDOutResReleaseExtractableID);
}
