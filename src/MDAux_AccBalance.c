/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDAux_AccBalance.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

//Input;
static int _MDInAux_AccPrecipID        = MFUnset;
static int _MDInAux_AccEvapID          = MFUnset;
static int _MDInAux_AccSnowPackChgID   = MFUnset;
static int _MDInAux_AccSMoistChgID     = MFUnset;
static int _MDInAux_AccGrdWatChgID     = MFUnset;
static int _MDInAux_AccRunoffID        = MFUnset;
static int _MDInAux_AccRiverStorageChg = MFUnset;

//Output
static int _MDOutAux_AccBalanceID  = MFUnset;

static void _MDAux_AccumBalance (int itemID)
{
// Local
    float dt              = MFModelGet_dt ();
// Input
	float precip          = MFVarGetFloat (_MDInAux_AccPrecipID,        itemID, 0.0); // Precipitation [m3/s]
	float evap            = MFVarGetFloat (_MDInAux_AccEvapID,          itemID, 0.0); // Evapotranspiration [m3/s]
	float snowPackChg     = MFVarGetFloat (_MDInAux_AccSnowPackChgID,   itemID, 0.0); // Snowpack Change [m3/s] 
	float sMoistChg       = MFVarGetFloat (_MDInAux_AccSMoistChgID,     itemID, 0.0); // Soil moisture change [m3/s]
	float grdWatChg       = MFVarGetFloat (_MDInAux_AccGrdWatChgID,     itemID, 0.0); // Groundwater change [m3/s]
	float runoff          = MFVarGetFloat (_MDInAux_AccRunoffID,        itemID, 0.0); // Runoff [m3/s]
	float riverStorageChg = MFVarGetFloat (_MDInAux_AccRiverStorageChg, itemID, 0.0); // River storage Change [m3/dt]

	MFVarSetFloat(_MDOutAux_AccBalanceID, itemID, precip - evap - snowPackChg - sMoistChg - grdWatChg - runoff);
}

int MDAux_AccumBalanceDef() {

	if (_MDOutAux_AccBalanceID != MFUnset) return (_MDOutAux_AccBalanceID);

	MFDefEntering ("Accumulated Balance");

	if (((_MDInAux_AccPrecipID        = MDAux_AccumPrecipDef ())       == CMfailed) ||
        ((_MDInAux_AccEvapID          = MDAux_AccumEvapDef ())         == CMfailed) ||
        ((_MDInAux_AccSnowPackChgID   = MDAux_AccumSnowPackChgDef ())  == CMfailed) ||
        ((_MDInAux_AccSMoistChgID     = MDAux_AccumSMoistChgDef ())    == CMfailed) ||
        ((_MDInAux_AccGrdWatChgID     = MDAux_AccumGrdWatChgDef ())    == CMfailed) ||
        ((_MDInAux_AccRunoffID        = MDAux_AccumRunoffDef ())       == CMfailed) ||
		((_MDInAux_AccRiverStorageChg = MDAux_AccumRiverStorageChg ()) == CMfailed) ||
        ((_MDOutAux_AccBalanceID      = MFVarGetID (MDVarAux_AccBalance, "m3/s", MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction(_MDAux_AccumBalance) == CMfailed)) return CMfailed;

	MFDefLeaving ("Accumulated Balance");
	return (_MDOutAux_AccBalanceID);
}
