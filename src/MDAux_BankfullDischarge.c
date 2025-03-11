#include <MF.h>
#include <MD.h>

static int _MDInAux_MeanDischargeID = MFUnset;
static int _MDOutAux_BankfullDischargeID = MFUnset;

static void _MDAux_BankfullDischarge(int itemID) {
    float meanDischarge = MFVarGetFloat(_MDInAux_MeanDischargeID, itemID, 0.0);
    float factor;

    if (meanDischarge >= 1000) {
        factor = 5;
    } else if (meanDischarge >= 100) {
        factor = 8;
    } else if (meanDischarge >= 10) {
        factor = 9;
    } else {
        factor = 25;
    }

    float bankfullDischarge = factor * meanDischarge; // Include standard deviation in calculation
    MFVarSetFloat(_MDOutAux_BankfullDischargeID, itemID, bankfullDischarge);
}

int MDAux_BankfullDischargeDef() {
    if (_MDOutAux_BankfullDischargeID != MFUnset) return _MDOutAux_BankfullDischargeID;

    MFDefEntering("Bankfull Discharge");

    if (((_MDInAux_MeanDischargeID = MDAux_DischargeMeanDef()) == CMfailed) ||
        ((_MDOutAux_BankfullDischargeID = MFVarGetID(MDVarAux_BankfullDischarge, "m3/s", MFOutput, MFState, MFInitial)) == CMfailed) ||
        (MFModelAddFunction(_MDAux_BankfullDischarge) == CMfailed)) {
        return CMfailed;
    }

    MFDefLeaving("Bankfull Discharge");
    return _MDOutAux_BankfullDischargeID;
}
