/******************************************************************************
GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

MDAux_BankfullDischarge.c

bfekete@gc.cuny.edu
*******************************************************************************/

#include <MF.h>
#include <MD.h>
#include <stdlib.h>

static int _MDInAux_StepCounterID        = MFUnset;
static int _MDInAux_AccumRunoffID        = MFUnset;
static int _MDInAux_MeanDischargeID      = MFUnset;

static int _MDOutAux_BankfullDischargeID = MFUnset;

#define PERCENTILE 99

static void _MDAux_BankfullDischarge(int itemID) {
    int   tStep      = MFVarGetInt   (_MDInAux_StepCounterID,    itemID,   0);
    float accumDisch = MFVarGetFloat (_MDInAux_AccumRunoffID,    itemID, 0.0);
    float meanDisch  = MFVarGetFloat (_MDInAux_MeanDischargeID,  itemID, 0.0);
    float *dischArray = (float *) MFVarGetUserData (_MDOutAux_BankfullDischargeID);

    if (tStep == 0) {
        dischArray = (float *) malloc(sizeof(float) * 365); // Assuming a maximum of 365 daily values
        MFVarSetUserData(_MDOutAux_BankfullDischargeID, dischArray);
    }

    dischArray[tStep % 365] = accumDisch;

    if (tStep >= 365) {
        float tempArray[365];
        memcpy(tempArray, dischArray, sizeof(float) * 365);
        qsort(tempArray, 365, sizeof(float), compareFloats);
        float bankfullDischarge = tempArray[(int)((PERCENTILE / 100.0) * 365) - 1];
        MFVarSetFloat (_MDOutAux_BankfullDischargeID, itemID, bankfullDischarge);
    } else {
        float bankfullDischarge = 10 * meanDisch;
        MFVarSetFloat (_MDOutAux_BankfullDischargeID, itemID, bankfullDischarge);
    }
}

int compareFloats(const void *a, const void *b) {
    return (*(float*)a > *(float*)b) - (*(float*)a < *(float*)b);
}

int MDAux_BankfullDischargeDef () {
    int  optID = MFcalculate;
    const char *optStr;

    if (_MDOutAux_BankfullDischargeID != MFUnset) return (_MDOutAux_BankfullDischargeID);

    MFDefEntering ("Bankfull Discharge");
    if ((optStr = MFOptionGet (MDVarAux_BankfullDischarge)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
    switch (optID) {
        default:
        case MFhelp:  MFOptionMessage (MDVarAux_BankfullDischarge, optStr, MFsourceOptions); return (CMfailed);
        case MFinput: _MDOutAux_BankfullDischargeID = MFVarGetID (MDVarAux_BankfullDischarge, "m3/s", MFInput, MFState, MFBoundary); break;
        case MFcalculate:
            if (((_MDInAux_StepCounterID        = MDAux_StepCounterDef()) == CMfailed) ||
                ((_MDInAux_AccumRunoffID        = MDAux_AccumRunoffDef()) == CMfailed) ||
                ((_MDInAux_MeanDischargeID      = MDAux_DischargeMeanDef()) == CMfailed) ||
                ((_MDOutAux_BankfullDischargeID = MFVarGetID (MDVarAux_BankfullDischarge, "m3/s", MFOutput, MFState, MFInitial)) == CMfailed) ||
                (MFModelAddFunction(_MDAux_BankfullDischarge) == CMfailed)) return (CMfailed);
            break;
    }
    MFDefLeaving ("Bankfull Discharge");
    return (_MDOutAux_BankfullDischargeID);
}
