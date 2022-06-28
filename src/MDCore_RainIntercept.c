/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDIntercept.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDInSnowPackChgID      = MFUnset;
static int _MDInPetID           = MFUnset;
static int _MDInCommon_PrecipID = MFUnset;
static int _MDInCParamCHeightID = MFUnset;
static int _MDInLeafAreaIndexID = MFUnset;
static int _MDInStemAreaIndexID = MFUnset;

static int _MDOutInterceptID    = MFUnset;

static void _MDRainIntercept (int itemID) {
// Input
	float precip   = MFVarGetFloat (_MDInCommon_PrecipID, itemID, 0.0); // daily precipitation [mm/dt]
	float pet      = MFVarGetFloat (_MDInPetID,           itemID, 0.0); // daily potential evapotranspiration [mm/dt]
// Output
	float intercept = 0.0; // estimated interception [mm/dt]

	if ((pet > 0.0) && (precip > 0.0)) {
	// Input
	 	float lai      = MFVarGetFloat (_MDInLeafAreaIndexID, itemID, 0.0); // projected leaf area index
		float sai      = MFVarGetFloat (_MDInStemAreaIndexID, itemID, 0.0); // projected stem area index
	// Local
		float c;   // canopy storage capacity [mm]
		
		c = MDConstInterceptCI * (lai + sai) / 2.0;
		if (c > 0.0) {
		// Input
			float sPackChg = MFVarGetFloat (_MDInSnowPackChgID,   itemID, 0.0); // snow pack change [mm/day]
			float height   = MFVarGetFloat (_MDInCParamCHeightID, itemID, 0.0); // canopy height [m]
		// Local
			float epi; // daily potential interception [mm/day]
			float eis; // maximum amount of evaporated interception during "storm" [mm]
			
			if (sPackChg > 0.0) precip = precip - sPackChg;
			epi = pet * (height < MDConstInterceptCH ? 1.0 + height / MDConstInterceptCH : 2.0);
			eis = MDConstInterceptD * epi;
			intercept = precip < (eis + c) ? /* capacity is not reached */ precip : /* capacity exceeded */ (eis + c);
			if (intercept > pet) intercept = pet; // FBM Addition
		}
	}
	MFVarSetFloat (_MDOutInterceptID,itemID, intercept);	
}

int MDCore_RainInterceptDef () {
	int optID = MFnone;
	const char *optStr;

	if (_MDOutInterceptID != MFUnset) return (_MDOutInterceptID);

	MFDefEntering ("Rainfed Intercept");
	if ((optStr = MFOptionGet (MDVarCore_RainInterception)) != (char *) NULL) optID = CMoptLookup (MFcalcOptions,optStr,true);
	switch (optID) {
		default:      MFOptionMessage (MDVarCore_RainInterception, optStr, MFcalcOptions); return (CMfailed);
		case MFhelp:  MFOptionMessage (MDVarCore_RainInterception, optStr, MFcalcOptions);
		case MFnone:  break;
		case MFinput: _MDOutInterceptID = MFVarGetID (MDVarCore_RainInterception, "mm", MFInput, MFFlux, MFBoundary); break;
		case MFcalculate:
			if (((_MDInCommon_PrecipID = MDCommon_PrecipitationDef()) == CMfailed) ||
                ((_MDInSnowPackChgID      = MDCore_SnowPackChgDef()) == CMfailed) ||
                ((_MDInLeafAreaIndexID = MDParam_LeafAreaIndexDef()) == CMfailed) ||
                ((_MDInStemAreaIndexID = MDParam_LCStemAreaIndexDef()) == CMfailed) ||
                ((_MDInPetID           = MDCore_RainPotETDef()) == CMfailed) ||
                ((_MDOutInterceptID    = MFVarGetID (MDVarCore_RainInterception, "mm", MFOutput, MFFlux, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDRainIntercept) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Rainfed Intercept");
	return (_MDOutInterceptID); 
}
