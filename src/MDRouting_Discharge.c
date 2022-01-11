/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDRouting_Discharge.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInRouting_DischargeID       = MFUnset;
static int _MDInRouting_RiverWidthID      = MFUnset;
static int _MDInDataAssim_DischObservedID = MFUnset;

// Output
static int _MDOutRouting_DischargeID      = MFUnset;

static void _MDRouting_Discharge (int itemID) {
	float discharge; // Discharge [m3/s]

	discharge = MFVarGetFloat (_MDInRouting_DischargeID,   itemID, 0.0);

	if (_MDInDataAssim_DischObservedID != MFUnset)
		 discharge = MFVarGetFloat (_MDInDataAssim_DischObservedID, itemID, discharge);

	MFVarSetFloat (_MDOutRouting_DischargeID, itemID, discharge);
}

enum { MDhelp, MDinput, MDcalculate, MDcorrected };

int MDRouting_DischargeDef() {
	int optID = MDinput;
	const char *optStr;
	const char *options [] = { MFhelpStr, MFinputStr, MFcalculateStr, "corrected", (char *) NULL };

	if (_MDOutRouting_DischargeID != MFUnset) return (_MDOutRouting_DischargeID);

	MFDefEntering ("Discharge");
	if ((optStr = MFOptionGet (MDOptConfig_Discharge)) != (char *) NULL) optID = CMoptLookup (options,optStr,true);
	switch (optID) {
		default:      MFOptionMessage (MDOptConfig_Discharge, optStr, options); return (CMfailed);
		case MDhelp:  MFOptionMessage (MDOptConfig_Discharge, optStr, options);
		case MDinput: _MDOutRouting_DischargeID = MFVarGetID (MDVarRouting_Discharge, "m3/s", MFInput, MFState, MFInitial); break;
		case MDcalculate:
			if (((_MDOutRouting_DischargeID = MFVarGetID (MDVarRouting_Discharge, "m3/s", MFRoute, MFState, MFInitial)) == CMfailed) ||
				((_MDInRouting_DischargeID  = MDRouting_DischargeUptakeDef ()) == CMfailed) ||
				((_MDInRouting_RiverWidthID = MDRouting_RiverWidthDef ())      == CMfailed) ||
                (MFModelAddFunction(_MDRouting_Discharge) == CMfailed)) return (CMfailed);
			break;
		case MDcorrected:
			if ((_MDInDataAssim_DischObservedID = MFVarGetID (MDVarDataAssim_DischObserved, "m3/s", MFInput, MFState, MFBoundary)) == CMfailed)
				return (CMfailed);
	}
	MFDefLeaving  ("Discharge");
	return (_MDOutRouting_DischargeID);
}
