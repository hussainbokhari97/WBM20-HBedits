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
static int _MDOutResExtractableReleaseID = MFUnset;

int MDReservoir_ReleaseDef () {
	int optID = MFoff;
	const char *optStr;

	if (_MDOutResReleaseID != MFUnset) return (_MDOutResReleaseID);

	MFDefEntering ("Reservoir release");
	if ((optStr = MFOptionGet (MDOptConfig_Reservoirs)) != (char *) NULL) optID = CMoptLookup (MFswitchOptions, optStr, true);
 	switch (optID) {
		default:     MFOptionMessage (MDOptConfig_Reservoirs, optStr, MFswitchOptions); return (CMfailed);
		case MFhelp: MFOptionMessage (MDOptConfig_Reservoirs, optStr, MFswitchOptions);
		case MFoff:
			if ((_MDOutResReleaseID = MDRouting_DischargeInChannelDef()) == CMfailed) return (CMfailed);
			break; 
		case MFon:
			if ((_MDOutResReleaseID = MDReservoir_OperationDef ()) == CMfailed) return (CMfailed);
			break;
	}
	MFDefLeaving ("Reservoir release");
	return (_MDOutResReleaseID); 
}

int MDReservoir_ExtractableReleaseDef () {
	int optID = MFoff;
	const char *optStr;

	if (_MDOutResExtractableReleaseID != MFUnset) return (_MDOutResExtractableReleaseID);

	MFDefEntering ("Extractable release");
	if ((optStr = MFOptionGet (MDOptConfig_Reservoirs)) != (char *) NULL) optID = CMoptLookup (MFswitchOptions, optStr, true);
 	switch (optID) {
		default:     MFOptionMessage (MDOptConfig_Reservoirs, optStr, MFswitchOptions); return (CMfailed);
		case MFhelp: MFOptionMessage (MDOptConfig_Reservoirs, optStr, MFswitchOptions); break;
		case MFoff: break;
		case MFon:
			if ((_MDOutResReleaseID = MDReservoir_OperationDef ()) == CMfailed) return (CMfailed);
			else if (_MDOutResReleaseID != MFUnset)
				_MDOutResExtractableReleaseID = MFVarGetID (MDVarReservoir_ExtractableRelease, "m3/s", MFRoute, MFState,  MFBoundary);
			break;
	}
	MFDefLeaving ("Extractable release");
	return (_MDOutResExtractableReleaseID);
}
