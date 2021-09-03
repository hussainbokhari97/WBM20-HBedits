/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDCore_Runoff.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInSurfCore_RunoffID = MFUnset;
static int _MDInBaseFlowID   = MFUnset;
static int _MDInRunoffCorrID = MFUnset;
// Output
static int _MDOutCore_RunoffID    = MFUnset;

static void _MDRunoff (int itemID) {
// Input
	float baseFlow;
	float surfaceRO;
	float runoffCorr;

	baseFlow  = MFVarGetFloat (_MDInBaseFlowID,   itemID, 0.0);
	surfaceRO = MFVarGetFloat (_MDInSurfCore_RunoffID, itemID, 0.0);
	runoffCorr = _MDInRunoffCorrID != MFUnset ? MFVarGetFloat (_MDInRunoffCorrID, itemID, 1.0) : 1.0;
	MFVarSetFloat (_MDOutCore_RunoffID, itemID, (baseFlow + surfaceRO) * runoffCorr);
}
 
enum { MDhelp, MDinput, MDcalculate, MDcorrected };

int MDCore_RunoffDef () {
	int  optID = MDinput;
	const char *optStr;
	const char *options [] = { MFhelpStr, MFinputStr, MFcalculateStr, "corrected", (char *) NULL };

	if (_MDOutCore_RunoffID != MFUnset) return (_MDOutCore_RunoffID);

	MFDefEntering ("Runoff");
	if ((optStr = MFOptionGet (MDVarCore_Runoff)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
	switch (optID) {
		default:      MFOptionMessage (MDVarCore_Runoff, optStr, options); return (CMfailed);
		case MDhelp:  MFOptionMessage (MDVarCore_Runoff, optStr, options);
		case MDinput: _MDOutCore_RunoffID = MFVarGetID (MDVarCore_Runoff, "mm", MFInput, MFFlux, MFBoundary); break;
		case MDcorrected:
			if ((_MDInRunoffCorrID  = MFVarGetID (MDVarDataAssim_RunoffCorretion, MFNoUnit, MFInput, MFState, MFBoundary)) == CMfailed)
				return (CMfailed);
			break;
		case MDcalculate:		
			if (((_MDInBaseFlowID        = MDCore_BaseFlowDef ())   == CMfailed) ||
                ((_MDInSurfCore_RunoffID = MDCore_SurfRunoffDef ()) == CMfailed) ||
                ((_MDOutCore_RunoffID    = MFVarGetID (MDVarCore_Runoff, "mm", MFOutput, MFFlux, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDRunoff) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving  ("Runoff");
	return (_MDOutCore_RunoffID);
}
