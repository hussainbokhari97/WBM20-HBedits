/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

WBMMain.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include "wbm.h"

typedef enum { MDpet,
               MDsurplus,
               MDinfiltration,
               MDrunoff,
               MDdischarge,
               MDbalance,
               MDgeometry,
               MDRiverbedShape,
               MDwatertemp,
               MDthermal,
               MDbankfullQcalc,
               MDsedimentflux,
               MDbedloadflux,
               MDBQARTpreprocess,
               MDparticulatenutrients,
               MDwaterdensity} MDoption;

int main (int argc,char *argv []) {
    int argNum;
    int optID = MDbalance;
    const char *optStr, *optName = MDOptConfig_Model;
    const char *options[] = { "pet",
                              "surplus",
                              "infiltration",
                              "runoff",
                              "discharge", 
                              "balance",
                              "watertemp",
                              "thermal",
                              "bankfullQcalc",
                              "sedimentflux",
                              "bedloadflux",
                              "BQARTpreprocess",
                              "particulatenutrients",
                              "waterdensity", (char *) NULL};

    argNum = MFOptionParse(argc, argv);

    if ((optStr = MFOptionGet(optName)) != (char *) NULL) optID = CMoptLookup(options, optStr, true);

    switch (optID) {
        case MDpet:                       return (MFModelRun(argc, argv, argNum, MDCore_RainPotETDef));
        case MDsurplus:                   return (MFModelRun(argc, argv, argNum, MDCore_RainWaterSurplusDef));
        case MDinfiltration:              return (MFModelRun(argc, argv, argNum, MDCore_RainInfiltrationDef));
        case MDrunoff:                    return (MFModelRun(argc, argv, argNum, MDCore_RunoffDef));
        case MDdischarge:                 return (MFModelRun(argc, argv, argNum, MDRouting_DischargeDef));
        case MDbalance:                   return (MFModelRun(argc, argv, argNum, MDCore_WaterBalanceDef));
        case MDwatertemp:                 return (MFModelRun(argc, argv, argNum, MDWTemp_RiverDef));
        case MDthermal:                   return (MFModelRun(argc, argv, argNum, MDWTemp_ThermalInputsDef));
        case MDbankfullQcalc:             return (MFModelRun(argc, argv, argNum, MDRouting_BankfullQcalcDef));
        case MDsedimentflux:              return (MFModelRun(argc, argv, argNum, MDSediment_FluxDef));
        case MDbedloadflux:               return (MFModelRun(argc, argv, argNum, MDSediment_BedloadFluxDef));
        case MDBQARTpreprocess:           return (MFModelRun(argc, argv, argNum, MDSediment_BQARTpreprocessDef));
        case MDparticulatenutrients:      return (MFModelRun(argc,argv,argNum,   MDSediment_ParticulateNutrientsDef));
        case MDwaterdensity:              return (MFModelRun(argc,argv,argNum,   MDSediment_WaterDensityDef));
    }
    MFOptionMessage(optName, optStr, options);
    return (CMfailed);
}
