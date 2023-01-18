/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

MDCore_BaseFlow.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <string.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCore_InfiltrationID          = MFUnset;
static int _NDInRouting_SoilMoistureID       = MFUnset;
static int _MDInIrrigation_GrossDemandID     = MFUnset;
static int _MDInIrrigation_ReturnFlowID      = MFUnset;
static int _MDInIrrigation_RunoffID          = MFUnset;

// Output
static int _MDOutCore_GrdWatID               = MFUnset;
static int _MDOutCore_GrdWatChgID            = MFUnset;
static int _MDOutCore_GrdWatRechargeID       = MFUnset;
static int _MDOutCore_BaseFlowID             = MFUnset;
static int _MDOutCore_IrrUptakeGrdWaterID    = MFUnset;
static int _MDOutCore_Irrigation_UptakeExternalID = MFUnset;

static float _MDGroundWatBETA = 0.016666667;

static void _MDCore_BaseFlow (int itemID) {
// Input
	float grdWaterRecharge;        // Groundwater recharge [mm/dt]
// Initial
	float grdWater;                // Groundwater size   [mm]
// Output
	float baseFlow;                // Base flow from groundwater [mm/dt]
// Local
	float grdWater0;
                     
	grdWater0 = grdWater         = MFVarGetFloat (_MDOutCore_GrdWatID,      itemID, 0.0);
	grdWater += grdWaterRecharge = MFVarGetFloat (_MDInCore_InfiltrationID, itemID, 0.0);

	if ((_MDInIrrigation_GrossDemandID != MFUnset) &&
	    (_MDInIrrigation_ReturnFlowID  != MFUnset)) {
	// Input
		float irrDemand     = MFVarGetFloat (_MDInIrrigation_GrossDemandID, itemID, 0.0); // Irrigation demand [mm/dt]
		float irrReturnFlow = MFVarGetFloat (_MDInIrrigation_ReturnFlowID,  itemID, 0.0); // Irrigational return flow [mm/dt]
    	float irrRunoff     = MFVarGetFloat (_MDInIrrigation_RunoffID,      itemID, 0.0); // Irrigational runoff [mm/dt]
	// Output
		float irrUptakeGrdWater = 0.0; // Irrigational water uptake from shallow groundwater [mm/dt]
		float irrUptakeExt      = 0.0; // Unmet irrigational water demand [mm/dt]

		grdWater         += irrReturnFlow + irrRunoff;
		grdWaterRecharge += irrReturnFlow + irrRunoff;

		if (_MDOutCore_IrrUptakeGrdWaterID   != MFUnset) {
			if (grdWater > irrDemand) { // Irrigation demand is satisfied from groundwater storage 
				irrUptakeGrdWater = irrDemand;
				grdWater -= irrUptakeGrdWater;
			}
			else { // Irrigation demand needs external source
				irrUptakeGrdWater = grdWater;
				irrUptakeExt      = irrDemand - irrUptakeGrdWater;
				grdWater = 0.0;
			}
			MFVarSetFloat (_MDOutCore_IrrUptakeGrdWaterID, itemID, irrUptakeGrdWater);
		}
		else irrUptakeExt = irrDemand;
		MFVarSetFloat (_MDOutCore_Irrigation_UptakeExternalID, itemID, irrUptakeExt);
	}
	baseFlow = grdWater * _MDGroundWatBETA;
	if (grdWater > baseFlow) grdWater -= baseFlow;
	else { baseFlow = grdWater; grdWater = 0.0; }

	MFVarSetFloat (_MDOutCore_GrdWatID,         itemID, grdWater);
    MFVarSetFloat (_MDOutCore_GrdWatChgID,      itemID, grdWater - grdWater0);
    MFVarSetFloat (_MDOutCore_GrdWatRechargeID, itemID, grdWaterRecharge);
	MFVarSetFloat (_MDOutCore_BaseFlowID,       itemID, baseFlow);
}

int MDCore_BaseFlowDef () {
	float par;
	const char *optStr;

	if (_MDOutCore_BaseFlowID != MFUnset) return (_MDOutCore_BaseFlowID);

	MFDefEntering ("Base flow");;
	if ((optStr = MFOptionGet (MDParGroundWatBETA))  != (char *) NULL) {
		if (strcmp(optStr,MFhelpStr) == 0) CMmsgPrint (CMmsgInfo,"%s = %f", MDParGroundWatBETA, _MDGroundWatBETA);
		_MDGroundWatBETA = sscanf (optStr,"%f",&par) == 1 ? par : _MDGroundWatBETA;
	}
	if (((_MDInCore_InfiltrationID         = MDCore_RainInfiltrationDef ())  == CMfailed) ||
		((_NDInRouting_SoilMoistureID      = MDCore_SoilMoistChgDef ())      == CMfailed) ||
		((_MDInIrrigation_GrossDemandID    = MDIrrigation_GrossDemandDef ()) == CMfailed) ||
		((_MDInIrrigation_GrossDemandID != MFUnset) &&
		 (((_MDInIrrigation_ReturnFlowID   = MDIrrigation_ReturnFlowDef ())     == CMfailed) ||
          ((_MDInIrrigation_RunoffID       = MDIrrigation_RunoffDef ())         == CMfailed) ||
          ((_MDOutCore_IrrUptakeGrdWaterID = MDIrrigation_UptakeGrdWaterDef ()) == CMfailed) ||
          ((_MDOutCore_Irrigation_UptakeExternalID = MFVarGetID (MDVarIrrigation_UptakeExternal, "mm", MFOutput, MFFlux,  MFBoundary)) == CMfailed))) ||
		((_MDOutCore_GrdWatID                      = MFVarGetID (MDVarCore_GroundWater,          "mm", MFOutput, MFState, MFInitial))  == CMfailed)   ||
        ((_MDOutCore_GrdWatChgID                   = MFVarGetID (MDVarCore_GroundWaterChange,    "mm", MFOutput, MFFlux,  MFBoundary)) == CMfailed)   ||
        ((_MDOutCore_GrdWatRechargeID              = MFVarGetID (MDVarCore_GroundWaterRecharge,  "mm", MFOutput, MFFlux,  MFBoundary)) == CMfailed)   ||
        ((_MDOutCore_BaseFlowID                    = MFVarGetID (MDVarCore_BaseFlow,             "mm", MFOutput, MFFlux,  MFBoundary)) == CMfailed)   ||
        (MFModelAddFunction(_MDCore_BaseFlow) == CMfailed)) return (CMfailed);
	MFDefLeaving ("Base flow ");
	return (_MDOutCore_BaseFlowID);
}

int MDCore_GroundWaterChangeDef () {
	if (_MDOutCore_GrdWatChgID != MFUnset)   return (_MDOutCore_GrdWatChgID);
	if (MDCore_BaseFlowDef () == (CMfailed)) return (CMfailed);	
	return (_MDOutCore_GrdWatChgID);
}
