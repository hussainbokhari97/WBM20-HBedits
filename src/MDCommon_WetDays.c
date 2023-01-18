/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

MDCommon_WetDays.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_PrecipID      = MFUnset;
static int _MDInParam_WetDaysAlphaID = MFUnset;
static int _MDInParam_WetDaysBetaID  = MFUnset;
// Output
static int _MDOutCommon_WetDaysID = MFUnset;

static void _MDWetDays (int itemID) {
// Model
	int nDays = MFDateGetMonthLength ();
// Input
	float precip = MFVarGetFloat (_MDInCommon_PrecipID,      itemID, 0.0);
	float alpha  = MFVarGetFloat (_MDInParam_WetDaysAlphaID, itemID, 1.0);
	float beta   = MFVarGetFloat (_MDInParam_WetDaysBetaID,  itemID, 0.0);
// Output 
	int wetDays;
   
	wetDays = (int) ((float) nDays * alpha * (1.0 - exp ((double) (beta * precip))));
	if (wetDays > nDays) wetDays = nDays;
	if (wetDays < 1)     wetDays = 1;

	MFVarSetInt (_MDOutCommon_WetDaysID,itemID,wetDays);
}

enum { MDhelp, MDinput, MDlbg };

int MDCommon_WetDaysDef ()
	{
	int optID = MDinput;
	const char *optStr;
	const char *options [] = { MFhelpStr, MFinputStr, "LBG", (char *) NULL };

	if (_MDOutCommon_WetDaysID != MFUnset) return (_MDOutCommon_WetDaysID);

	MFDefEntering ("Wet Days");
	if ((optStr = MFOptionGet (MDVarCommon_WetDays)) != (char *) NULL) optID = CMoptLookup (options,optStr,true);
	switch (optID) {
		default:
		case MDhelp:  MFOptionMessage (MDVarCommon_WetDays, optStr, options); return (CMfailed);
		case MDinput: _MDOutCommon_WetDaysID = MFVarGetID (MDVarCommon_WetDays, MFNoUnit, MFInput, MFState, MFBoundary); break;
		case MDlbg:
			if (((_MDInCommon_PrecipID      = MFVarGetID (MDVarCommon_PrecipMonthly, "mm",     MFInput,  MFFlux,  MFBoundary)) == CMfailed) ||
                ((_MDInParam_WetDaysAlphaID = MFVarGetID (MDVarParam_WetDaysAlpha,   MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDInParam_WetDaysBetaID  = MFVarGetID (MDVarParam_WetDaysBeta,    MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutCommon_WetDaysID    = MFVarGetID (MDVarCommon_WetDays,       MFNoUnit, MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
				(MFModelAddFunction (_MDWetDays) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Wet Days");
	return (_MDOutCommon_WetDaysID);
	}
