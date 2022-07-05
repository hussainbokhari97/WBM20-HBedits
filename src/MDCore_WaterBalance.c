/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDWaterBalance.c

dominik.wisser@unh.edu
This is meant to check the vertical water balance for each grid cell. It does 
NOT include any water that is flowing laterally and should not be used to call BCG....
*******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_PrecipID             = MFUnset;
static int _MDInEvaptrsID                   = MFUnset;
static int _MDInSnowPackChgID               = MFUnset;
static int _MDInSoilMoistChgID              = MFUnset;
static int _MDInAux_GrdWatChgID             = MFUnset;
static int _MDInCore_RunoffID               = MFUnset;
static int _MDInRouting_DischargeID         = MFUnset;

static int _MDInIrrEvapotranspID            = MFUnset;
static int _MDInIrrSoilMoistChgID           = MFUnset;
static int _MDInIrrigation_AreaFracID       = MFUnset;
static int _MDInIrrigation_GrossDemandID    = MFUnset;
static int _MDInIrrigation_ReturnFlowID     = MFUnset;
static int _MDInIrrigation_RunoffID         = MFUnset;
static int _MDInIrrigation_UptakeRiverID    = MFUnset;
static int _MDInIrrigation_UptakeGrdWaterID = MFUnset;
static int _MDInIrrigation_UptakeExcessID   = MFUnset;

// Output
static int _MDOutWaterBalanceID             = MFUnset;
static int _MDOutIrrUptakeBalanceID         = MFUnset;
static int _MDOutIrrWaterBalanceID          = MFUnset;

static void _MDWaterBalance(int itemID) {
// Input
	float precip       = MFVarGetFloat(_MDInCommon_PrecipID,  itemID, 0.0);
	float evap          = MFVarGetFloat(_MDInEvaptrsID,        itemID, 0.0);	
	float snowPackChg  = MFVarGetFloat(_MDInSnowPackChgID,    itemID, 0.0);	
	float soilMoistChg = MFVarGetFloat(_MDInSoilMoistChgID,   itemID, 0.0);
	float grdWaterChg  = MFVarGetFloat(_MDInAux_GrdWatChgID,  itemID, 0.0);
	float runoff       = MFVarGetFloat(_MDInCore_RunoffID,    itemID, 0.0);
// Output
	float balance;

	balance = precip - evap - runoff - grdWaterChg - snowPackChg - soilMoistChg;
	if (_MDInIrrigation_GrossDemandID != MFUnset) { 
	// Input
		float irrAreaFrac       = MFVarGetFloat (_MDInIrrigation_AreaFracID,       itemID, 0.0);
		if (irrAreaFrac > 0.0) {
		// Input
			float irrGrossDemand    = MFVarGetFloat (_MDInIrrigation_GrossDemandID,    itemID, 0.0);
    		float irrReturnFlow     = MFVarGetFloat (_MDInIrrigation_ReturnFlowID,     itemID, 0.0);
			float irrRunoff         = MFVarGetFloat (_MDInIrrigation_RunoffID,         itemID, 0.0);
			float irrEvapotransp    = MFVarGetFloat (_MDInIrrEvapotranspID,            itemID, 0.0);
			float irrSoilMoistChg   = MFVarGetFloat (_MDInIrrSoilMoistChgID,           itemID, 0.0);
			float irrUptake         = MFVarGetFloat (_MDInIrrigation_UptakeExcessID,   itemID, 0.0);
		// Local
			float irrBalance, irrUptakeBalance = irrGrossDemand - irrUptake;

			irrBalance = irrGrossDemand - irrEvapotransp - irrSoilMoistChg - irrReturnFlow - irrRunoff;
			if (_MDInIrrigation_UptakeGrdWaterID  != MFUnset) 
				irrUptakeBalance -= MFVarGetFloat (_MDInIrrigation_UptakeGrdWaterID, itemID, 0.0);
			if (_MDInIrrigation_UptakeRiverID     != MFUnset) {
				irrUptake = MFVarGetFloat (_MDInIrrigation_UptakeRiverID,    itemID, 0.0);
				irrUptakeBalance -= irrUptake;
				balance += irrUptake;
			}
			MFVarSetFloat (_MDOutIrrWaterBalanceID,  itemID, irrBalance);
			MFVarSetFloat (_MDOutIrrUptakeBalanceID, itemID, irrUptakeBalance);
		}
		else {
			MFVarSetFloat (_MDOutIrrWaterBalanceID,  itemID, 0.0);
			MFVarSetFloat (_MDOutIrrUptakeBalanceID, itemID, 0.0);
		}
	}
	MFVarSetFloat (_MDOutWaterBalanceID, itemID , balance);
}

int MDCore_WaterBalanceDef() {
	int optID = MFnone, ret;
	const char *optStr;
 
	MFDefEntering ("WaterBalance");
	if ((MDAux_AccumBalanceDef() == CMfailed) ||
        ((_MDInCommon_PrecipID     = MDCommon_PrecipitationDef ())    == CMfailed) ||
        ((_MDInEvaptrsID           = MDCore_EvapotranspirationDef ()) == CMfailed) ||
        ((_MDInSnowPackChgID       = MDCore_SnowPackChgDef ())        == CMfailed) ||
        ((_MDInSoilMoistChgID      = MDCore_SoilMoistChgDef ())       == CMfailed) ||
        ((_MDInCore_RunoffID       = MDCore_RunoffDef ())             == CMfailed) ||
        ((_MDInRouting_DischargeID = MDRouting_DischargeDef ())       == CMfailed) ||
        ((_MDInAux_GrdWatChgID     = MFVarGetID (MDVarCore_GroundWaterChange, "mm", MFInput,  MFFlux, MFBoundary)) == CMfailed) ||
        ((_MDOutWaterBalanceID     = MFVarGetID (MDVarCore_WaterBalance,      "mm", MFOutput, MFFlux, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction(_MDWaterBalance) == CMfailed))
	    return (CMfailed);
	
	if ((optStr = MFOptionGet (MDOptConfig_Irrigation)) != (char *) NULL) optID = CMoptLookup (MFcalcOptions, optStr, true);
	switch (optID) {
		case MFnone:  break;
		default:      
		case MFhelp: MFOptionMessage (MDVarCore_Runoff, optStr, MFcalcOptions); return (CMfailed);
		case MFinput:
			if (((_MDInIrrigation_GrossDemandID    = MDIrrigation_GrossDemandDef ())    == CMfailed) ||
                ((_MDInIrrigation_UptakeRiverID    = MDIrrigation_UptakeRiverDef ())    == CMfailed) ||
            	((_MDInIrrigation_UptakeGrdWaterID = MDIrrigation_UptakeGrdWaterDef ()) == CMfailed))
				return (CMfailed);
			break;
		case MFcalculate:                 
			if (((_MDInIrrigation_GrossDemandID    = MDIrrigation_GrossDemandDef ())    == CMfailed) ||
            	((_MDInIrrigation_UptakeRiverID    = MDIrrigation_UptakeRiverDef ())    == CMfailed) ||
            	((_MDInIrrigation_UptakeGrdWaterID = MDIrrigation_UptakeGrdWaterDef ()) == CMfailed) ||
            	((_MDInIrrSoilMoistChgID           = MDIrrigation_SoilMoistChgDef ())   == CMfailed) ||
				((_MDInIrrigation_AreaFracID       = MDIrrigation_IrrAreaDef())         == CMfailed) ||
            	((_MDInIrrEvapotranspID            = MFVarGetID (MDVarIrrigation_Evapotranspiration, "mm", MFInput,  MFFlux, MFBoundary)) == CMfailed) ||
            	((_MDInIrrigation_ReturnFlowID     = MFVarGetID (MDVarIrrigation_ReturnFlow,         "mm", MFInput,  MFFlux, MFBoundary)) == CMfailed) ||
            	((_MDInIrrigation_RunoffID         = MFVarGetID (MDVarIrrigation_Runoff,             "mm", MFInput,  MFFlux, MFBoundary)) == CMfailed) ||
            	((_MDInIrrigation_UptakeExcessID   = MFVarGetID (MDVarIrrigation_UptakeExcess,       "mm", MFInput,  MFFlux, MFBoundary)) == CMfailed) ||
            	((_MDOutIrrUptakeBalanceID         = MFVarGetID (MDVarIrrigation_UptakeBalance,      "mm", MFOutput, MFFlux, MFBoundary)) == CMfailed) ||
            	((_MDOutIrrWaterBalanceID          = MFVarGetID (MDVarIrrigation_WaterBalance,       "mm", MFOutput, MFFlux, MFBoundary)) == CMfailed))
	    		return (CMfailed);
			break;
	}
	MFDefLeaving ("WaterBalance");
	return (_MDOutWaterBalanceID);	
}
