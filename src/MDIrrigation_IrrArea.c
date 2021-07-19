/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDIrrigation_IrrArea.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInIrrAreaFracSeason1ID = MFUnset;
static int _MDInIrrAreaFracSeason2ID = MFUnset;
static int _MDInGrowingSeason1ID     = MFUnset;
static int _MDInGrowingSeason2ID     = MFUnset;
// Output
static int _MDOutIrrigatedAreaFracID = MFUnset; 

static void _MDIrrigatedAreaIWMI (int itemID) {
	float irrAreaFrac;	
	float irrAreaFracSeason1;
	float irrAreaFracSeason2;
	float Season1Doy;
	float Season2Doy;

	Season1Doy= MFVarGetFloat(_MDInGrowingSeason1ID,      itemID, 100);
	Season2Doy= MFVarGetFloat(_MDInGrowingSeason2ID,      itemID, 250);
	irrAreaFracSeason1 = MFVarGetFloat(_MDInIrrAreaFracSeason1ID, itemID, 0.0);
	irrAreaFracSeason2 = MFVarGetFloat(_MDInIrrAreaFracSeason2ID, itemID, 0.0);

	if (Season1Doy < Season2Doy) {
		if      (MFDateGetDayOfYear () < Season1Doy) irrAreaFrac = irrAreaFracSeason2;
		else if (MFDateGetDayOfYear () < Season2Doy) irrAreaFrac = irrAreaFracSeason1;
		else irrAreaFrac = irrAreaFracSeason2;
	} else {
		if (MFDateGetDayOfYear() <= Season1Doy && MFDateGetDayOfYear() > Season2Doy) irrAreaFrac = irrAreaFracSeason1;
		else irrAreaFrac = irrAreaFracSeason2;
	}
	MFVarSetFloat(_MDOutIrrigatedAreaFracID, itemID, irrAreaFrac);
}

enum { MDfao, MDiwmi, MDhelp};

int MDIrrigation_IrrAreaDef () {
	int optID = MDfao;
	const char *optStr, *optName = MDOptIrrigation_AreaMap;
	const char *options [] = { "FAO", "IWMI", MFhelpStr, (char *) NULL };

	if (_MDOutIrrigatedAreaFracID != MFUnset) return (_MDOutIrrigatedAreaFracID);

	MFDefEntering ("Irrigated Area");
	if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);

	switch (optID) {
		case MDhelp:  MFOptionMessage (optName, optStr, options);
        case MDfao:
            if ((_MDOutIrrigatedAreaFracID = MFVarGetID (MDVarIrrigation_AreaFraction, MFNoUnit, MFInput, MFState, MFBoundary)) == CMfailed) return (CMfailed);
            break;
		case MDiwmi:
		    if (((_MDInIrrAreaFracSeason1ID = MFVarGetID (MDVarIrrigation_AreaFractionSeason1, MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDInIrrAreaFracSeason2ID = MFVarGetID (MDVarIrrigation_AreaFractionSeason2, MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDInGrowingSeason1ID     = MFVarGetID (MDVarIrrigation_GrowingSeason1Start, "DoY",    MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDInGrowingSeason2ID     = MFVarGetID (MDVarIrrigation_GrowingSeason2Start, "DoY",    MFInput,  MFState, MFBoundary)) == CMfailed) ||
				((_MDOutIrrigatedAreaFracID = MFVarGetID (MDVarIrrigation_AreaFraction,        MFNoUnit, MFOutput, MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDIrrigatedAreaIWMI) == CMfailed)) return (CMfailed);
		    break;
        default: MFOptionMessage (optName, optStr, options); return (CMfailed);
	}
	MFDefLeaving ("IrrigatedArea");
	return (_MDOutIrrigatedAreaFracID);
}
