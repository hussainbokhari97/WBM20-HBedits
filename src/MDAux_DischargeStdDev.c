/******************************************************************************
GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 4-2023, UNH - ASRC/CUNY

MDAux_DischargeStdDev.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>
#include <math.h> // Include math library for pow

static int _MDInAux_StepCounterID       = MFUnset;
static int _MDInAux_AccumRunoffID       = MFUnset;
static int _MDInAux_DischargeMeanID     = MFUnset;
static int _MDOutAux_DischargeStdDevID  = MFUnset;
static int _MDOutAux_SumSqDiffID        = MFUnset; // Add variable for sum of squared differences

static void _MDAux_DischargeStdDev(int itemID) {
    int tStep = MFVarGetInt(_MDInAux_StepCounterID, itemID, 0);
    float accumDisch = MFVarGetFloat(_MDInAux_AccumRunoffID, itemID, 0.0);
    float dischMean = MFVarGetFloat(_MDInAux_DischargeMeanID, itemID, 0.0);
    double sumSqDiff = MFVarGetFloat(_MDOutAux_SumSqDiffID, itemID, 0.0); // Retrieve sum of squared differences as double

    if (tStep > 0) { // Ensure there are at least two data points to avoid division by zero
        // Update sum of squared differences
        sumSqDiff += (double)(accumDisch - dischMean) * (accumDisch - dischMean);
        
        // Calculate standard deviation using pow for square root
        double stdDev = pow(sumSqDiff / (double)tStep, 0.5); // Using tStep as n-1 since tStep is zero-based (starts from 0)

        // Set updated sum of squared differences and standard deviation
        MFVarSetFloat(_MDOutAux_SumSqDiffID, itemID, (float)sumSqDiff); // Store sumSqDiff back as float
        MFVarSetFloat(_MDOutAux_DischargeStdDevID, itemID, (float)stdDev); // Store stdDev back as float
    }
}

int MDAux_DischargeStdDevDef() {
    int optID = MFcalculate;
    const char *optStr;

    if (_MDOutAux_DischargeStdDevID != MFUnset) return (_MDOutAux_DischargeStdDevID);

    MFDefEntering("Discharge Std Dev");
    if ((optStr = MFOptionGet(MDVarAux_DischargeStdDev)) != (char *)NULL) optID = CMoptLookup(MFsourceOptions, optStr, true);
    switch (optID) {
        default:
        case MFhelp:  MFOptionMessage(MDVarAux_DischargeStdDev, optStr, MFsourceOptions); return (CMfailed);
        case MFinput: _MDOutAux_DischargeStdDevID = MFVarGetID(MDVarAux_DischargeStdDev, "m3/s", MFInput, MFState, MFBoundary); break;
        case MFcalculate:
            if (((_MDInAux_StepCounterID = MDAux_StepCounterDef()) == CMfailed) ||
                ((_MDInAux_AccumRunoffID = MDAux_AccumRunoffDef()) == CMfailed) ||
                ((_MDInAux_DischargeMeanID = MDAux_DischargeMeanDef()) == CMfailed) || // Get mean discharge
                ((_MDOutAux_DischargeStdDevID = MFVarGetID(MDVarAux_DischargeStdDev, "m3/s", MFOutput, MFState, MFInitial)) == CMfailed) || // Add variable definition for standard deviation
                ((_MDOutAux_SumSqDiffID = MFVarGetID("SumSqDiff", "m3/s", MFOutput, MFState, MFInitial)) == CMfailed) || // Add variable definition for sum of squared differences
                (MFModelAddFunction(_MDAux_DischargeStdDev) == CMfailed)) return (CMfailed);
            break;
    }
    MFDefLeaving("Discharge Std Dev");
    return (_MDOutAux_DischargeStdDevID);
}
