//
// Created by Davis Polito on 4/29/25.
//

#ifndef STRINGCMODULE_H
#define STRINGCMODULE_H
#include "defs.h"
#include "leaf-mempool.h"
#include "leaf-physical.h"
#include "leaf.h"

typedef enum {
    StringCEventWatchFlag,
    StringCOversample,
    StringCMIDIPitch,
    StringCFreq,
    StringCWaveLength,
    StringCDampFreq,
    StringCDecay,
    StringCTargetLevel,
    StringCLevelSmooth,
    StringCLevelStrength,
    StringCPickupPoint,
    StringCLevelMode,
    StringCRippleGain,
    StringCRippleDelay,
    StringCPluckPosition,
    StringCNumParams
} StringCModuleParams;

typedef struct _tStringCModule {
    ModuleHeader header;
    tSimpleLivingString3 *theString;
    float pluckPosition;
    float inputNote;
    tMempool* mempool;

} _tStringCModule;

typedef _tStringCModule* tStringCModule;
void tStringCModule_tick (tStringCModule const filt);
//init module
void tStringCModule_init(void** const module, float* const params, float id, LEAF* const leaf);
void tStringCModule_initToPool(void** const module, float* const params, float id, tMempool** const mempool);
void tStringCModule_free(void** const env);
//init processors
// Non-modulatable setters for tStringCModule
void tStringCModule_setParameter(tStringCModule const s, StringModelParams param, float input);

void tStringCModule_onNoteOn(tStringCModule const s, float velocity);
void tStringCModule_onNoteOn(tStringCModule const env, float vel);

#endif //StringCMODULE_H
