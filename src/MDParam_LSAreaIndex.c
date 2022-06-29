/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDParam_LSAreaIndex.c

dominik.wisser@unh.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDInCommon_CoverID         = MFUnset;
static int _MDInCommon_AtMeanID        = MFUnset;
static int _MDInParam_LPMaxID          = MFUnset;
static int _MDOutParam_LeafAreaIndexID = MFUnset;

static void _MDParam_LeafAreaIndex (int itemID) { // projected leaf area index (lai) pulled out from cover dependent PET functions
// Input
	int   cover = MFVarGetInt   (_MDInCommon_CoverID,  itemID, 7); 
	float lpMax = MFVarGetFloat (_MDInParam_LPMaxID,   itemID, 0.0); // maximum projected leaf area index
	float airT  = MFVarGetFloat (_MDInCommon_AtMeanID, itemID, 0.0);
// Local
	float lai;

	if (cover == 0) lai = lpMax;
	else if (airT > 8.0) lai = lpMax;
	else lai = 0.0;

	MFVarSetFloat (_MDOutParam_LeafAreaIndexID,itemID,0.001 > lai ? 0.001 : lai);
}

int MDParam_LeafAreaIndexDef () {
	int optID = MFinput;
	const char *optStr;

	if (_MDOutParam_LeafAreaIndexID != MFUnset) return (_MDOutParam_LeafAreaIndexID);

	MFDefEntering ("Leaf Area");
	if ((optStr = MFOptionGet (MDVarCore_LeafAreaIndex)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions,optStr,true);
	switch (optID) {
		default:
		case MFhelp:   MFOptionMessage (MDVarCore_LeafAreaIndex, optStr, MFsourceOptions); return (CMfailed);
		case MFinput:  _MDOutParam_LeafAreaIndexID = MFVarGetID (MDVarCore_LeafAreaIndex, MFNoUnit, MFInput, MFState, MFBoundary); break;
		case MFcalculate:
			if (((_MDInParam_LPMaxID          = MDParam_LCLPMaxDef ())          == CMfailed) ||
                ((_MDInCommon_CoverID         = MDParam_LandCoverMappingDef ()) == CMfailed) ||
                ((_MDInCommon_AtMeanID        = MDCommon_AirTemperatureDef ())  == CMfailed) ||
                ((_MDOutParam_LeafAreaIndexID = MFVarGetID (MDVarCore_LeafAreaIndex,    MFNoUnit, MFOutput, MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction(_MDParam_LeafAreaIndex) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Leaf Area");
	return (_MDOutParam_LeafAreaIndexID);
}

static int _MDInCParamCHeightID;
static int _MDOutStemAreaIndexID = MFUnset;

static void _MDStemAreaIndex (int itemID) {
// Projected Stem area index (sai) pulled out from McNaugthon and Black PET function
// Input
 	float lpMax   = MFVarGetFloat (_MDInParam_LPMaxID,   itemID, 0.0); // maximum projected leaf area index
	float cHeight = MFVarGetFloat (_MDInCParamCHeightID, itemID, 0.0); // canopy height [m]
// Local
	float sai;

	sai = lpMax > MDConstLPC ? MDConstCS * cHeight : (lpMax / MDConstLPC) * MDConstCS * cHeight;
	MFVarSetFloat (_MDOutStemAreaIndexID,itemID,sai);
}

int MDParam_LCStemAreaIndexDef () {
	int optID = MFinput;
	const char *optStr;

	if (_MDOutStemAreaIndexID != MFUnset) return (_MDOutStemAreaIndexID);

	MFDefEntering ("Stem Area Index");
	if ((optStr = MFOptionGet (MDVarCore_StemAreaIndex)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions,optStr,true);
	switch (optID) {
		default:
		case MFhelp:  MFOptionMessage (MDVarCore_StemAreaIndex, optStr, MFsourceOptions); return (CMfailed);
		case MFinput:  _MDOutStemAreaIndexID = MFVarGetID (MDVarCore_StemAreaIndex, MFNoUnit, MFInput, MFState, MFBoundary); break;
		case MFcalculate:
			if (((_MDInParam_LPMaxID    = MDParam_LCLPMaxDef ())  == CMfailed) ||
                ((_MDInCParamCHeightID  = MDParam_LCHeightDef ()) == CMfailed) ||
                ((_MDOutStemAreaIndexID = MFVarGetID (MDVarCore_StemAreaIndex, MFNoUnit, MFOutput, MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDStemAreaIndex) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Stem Area Index");
	return (_MDOutStemAreaIndexID);
}
