//
// Created by Davis Polito on 4/29/25.
//

#ifndef STRINGBMODULE_H
#define STRINGBMODULE_H
#include "defs.h"
#include "leaf-mempool.h"
#include "leaf-physical.h"
#include "leaf.h"

typedef enum {
    StringBEventWatchFlag,
    StringBOversample,
    StringBMIDIPitch,
    StringBFreq,
    StringBWaveLength,
    StringBDampFreq,
    StringBDecay,
    StringBTargetLevel,
    StringBLevelSmooth,
    StringBLevelStrength,
    StringBPickupPoint,
    StringBLevelMode,
    StringBRippleGain,
    StringBRippleDelay,
    StringBPluckPosition,
    StringBNumParams
} StringBModuleParams;

typedef struct _tStringBModule {
    ModuleHeader header;
    tSimpleLivingString3 *theString;
    float pluckPosition;
    float inputNote;
    tMempool* mempool;

} _tStringBModule;

typedef _tStringBModule* tStringBModule;
void tStringBModule_tick (tStringBModule const filt);
//init module
void tStringBModule_init(void** const module, float* const params, float id, LEAF* const leaf);
void tStringBModule_initToPool(void** const module, float* const params, float id, tMempool** const mempool);
void tStringBModule_free(void** const env);
//init processors
// Non-modulatable setters for tStringBModule
void tStringBModule_setParameter(tStringBModule const s, StringModelParams param, float input);

void tStringBModule_onNoteOn(tStringBModule const s, float velocity);
void tStringBModule_onNoteOn(tStringBModule const env, float vel);

#endif //STRINGBMODULE_H
