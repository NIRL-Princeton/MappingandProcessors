//
// Created by Matthew McWeeney on 9/3/26.
//

#ifndef ELECTORSYNTH_SIMPLEENVMODULE_H
#define ELECTORSYNTH_SIMPLEENVMODULE_H

#pragma once
#include "leaf.h"
#include "defs.h"
// #include "processor.h"
#include "leaf-mempool.h"
#include "leaf-envelopes.h"

#define EXP_BUFFER_SIZE 2048
#define DECAY_EXP_BUFFER_SIZE 2048
typedef enum {
    SimpEnvEventWatchFlag, //all of them need this
    SimpEnvAttack,
    SimpEnvDecay,
    SimpEnvSustain,
    SimpEnvRelease,
    SimpEnvLeak,
    SimpEnvVelocitySense,
    SimpEnvNumParams //all of them need this
} SimpEnvParams;



typedef struct _tSimpleEnvModule {
    //start boilerplate  - processor represents all of these///
    ModuleHeader header;
    //if its a combo/multi object put them all here
    tADSRT theEnv;
    //end boilerplate  - processor represents all of these///
    //specific other variables
    //float velocitySense;
    tSlopeRamp velSenseSmoother;
    const float* envTimeTableAddress;
    float envTimeTableSizeMinusOne;
    uint32_t tableSize;
    float expBuffer[EXP_BUFFER_SIZE];
    float expBufferSizeMinusOne;

    float decayExpBuffer[DECAY_EXP_BUFFER_SIZE];
    float decayExpBufferSizeMinusOne;
    //mempool
    tMempool* mempool;
} _tSimpleEnvModule;

typedef _tSimpleEnvModule* tSimpleEnvModule;

//init module
void tSimpleEnvModule_init(void** const env, float* const params, float id, LEAF* const leaf);
void tSimpleEnvModule_initToPool(void** const env, float* const params, float id, tMempool** const mempool);
void tSimpleEnvModule_free(void** const env);
void tSimpleEnvModule_tick (tSimpleEnvModule const env);

//note on action
void tSimpleEnvModule_onNoteOn(tSimpleEnvModule const env, float vel);

// Modulatable setters
void tSimpleEnvModule_setParameter(tSimpleEnvModule const env, int parameter_id, float input);

// Non-modulatable setters
void tSimpleEnvModule_setRateTableLocation (tSimpleEnvModule const env, float* tableAddress);
void tSimpleEnvModule_setSampleRate (tSimpleEnvModule const env, float sr);

#endif // ELECTORSYNTH_SIMPLEENVMODULE_H
