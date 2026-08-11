//
// Created by Matthew McWeeney on 8/3/26.
//

#ifndef ELECTORSYNTH_SINEMODULE_H
#define ELECTORSYNTH_SINEMODULE_H

#include "leaf.h"
#include "defs.h"
// #include "processor.h"
#include "leaf-mempool.h"
#include "leaf-oscillators.h"
#include "leaf-envelopes.h"
typedef enum {
    SineEventWatchFlag,
    SinePitch,
    SineGain,
    SineNumParams
} SineParams;

typedef struct _tSineModule {
    ModuleHeader header;

    tCycle theSine;

    // float fine;
    // float harmonicMultiplier;
    // float pitchOffset;
    // float freqOffset;
    // tRamp pitchSmooth;
    // float octaveOffset;
    // float inputNote;
    // float finalFreq;
    float note;
    float amp;
    tSlopeRamp ampSmoother;
    float* mtofTable;
    float sr;
    float invSr;
    // int hStepped;
    // int pStepped;
    // int syncMode;
    //int portaType;

    tMempool* mempool;
} _tSineModule;

typedef _tSineModule* tSineModule;

//init module
void tSineModule_init(void** const osc, float* const params, float id, LEAF* const leaf);
void tSineModule_initToPool(void** const osc, float* const params, float id, tMempool** const mempool);
void tSineModule_free(void** const osc);
void tSineModule_setParameter(tSineModule const osc, SineParams param_type, float input);
// Modulatable setters
//void tSineModule_setInputNote (tSineModule const osc, float inputNote);
void tSineModule_tick (tSineModule const osc, float*);


// Non-modulatable setters
void tSineModule_setMTOFTableLocation (tSineModule const osc, float* tableAddress);
void tSineModule_setSampleRate (tSineModule const osc, float sr);
//void tSineModule_setGlideOrigin (tSineModule const osc, float originNote);


#endif // ELECTORSYNTH_SINEMODULE_H
