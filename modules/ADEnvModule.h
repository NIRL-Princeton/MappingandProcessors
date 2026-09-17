//
// Created by Matthew McWeeney on 9/6/26.
//

#ifndef ELECTORSYNTH_ADENVMODULE_H
#define ELECTORSYNTH_ADENVMODULE_H

#pragma once
#include "leaf.h"
#include "defs.h"
// #include "processor.h"
#include "leaf-mempool.h"
#include "leaf-envelopes.h"

#define EXP_BUFFER_SIZE 2048
#define DECAY_EXP_BUFFER_SIZE 2048
typedef enum {
    ADEnvEventWatchFlag, //all of them need this
    ADEnvAttack,
    ADEnvDecay,
    ADEnvVelocitySense,
    ADEnvNumParams //all of them need this
} ADEnvParams;

typedef struct _tADEnvModule {
    ModuleHeader header;

    tAD theEnv;

    tSlopeRamp velSenseSmoother;
    const float* envTimeTableAddress;
    float envTimeTableSizeMinusOne;
    uint32_t tableSize;

    tMempool* mempool;
} _tADEnvModule;

typedef _tADEnvModule* tADEnvModule;

//init module
void tADEnvModule_init(void** const env, float* const params, float id, LEAF* const leaf);
void tADEnvModule_initToPool(void** const env, float* const params, float id, tMempool** const mempool);
void tADEnvModule_free(void** const env);
void tADEnvModule_tick (tADEnvModule const env);

//note on action
void tADEnvModule_onNoteOn(tADEnvModule const env, float vel);

// Modulatable setters
void tADEnvModule_setParameter(tADEnvModule const env, int parameter_id, float input);

// Non-modulatable setters
void tADEnvModule_setSampleRate (tADEnvModule const env, float sr);

#endif // ELECTORSYNTH_ADENVMODULE_H
