//
// Created by Matthew McWeeney on 7/15/26.
//

#ifndef ELECTORSYNTH_PERLINNOISEMODULE_H
#define ELECTORSYNTH_PERLINNOISEMODULE_H
#include "leaf.h"
#include "defs.h"
#include "leaf-mempool.h"
#include "leaf-oscillators.h"
#include "leaf-envelopes.h"

typedef enum
{
    PerlNoiseEventWatchFlag,
    PerlNoiseGain,
    PerlNoiseRate,
    PerlNoiseEnergy

} PerlNosParams;

typedef struct _tPerlNoiseModule
{
    ModuleHeader header;

    tPerlinNoise thePerlNoise;

    float gain;
    tSlopeRamp gainSmoother;
    float inputRateHz;
    float rateHz;
    float energy;

    tLookupTable* table;

    tMempool* mempool;

} _tPerlNoiseModule;

typedef _tPerlNoiseModule* tPerlNoiseModule;

//init module
void tPerlNoiseModule_init(void** const perlNoise, float* const params, float id, LEAF* const leaf);
void tPerlNoiseModule_initToPool(void** const perlNoise, float* const params, float id, tMempool** const mempool, tLookupTable* const table);
void tPerlNoiseModule_free(void** const perlNoise);
void tPerlNoiseModule_setParameter(tPerlNoiseModule const perlNoise, PerlNosParams param_type, float input);
// Modulatable setters
void tPerlNoiseModule_tick (tPerlNoiseModule const perlNoise);

void tPerlNoiseModule_setRate(tPerlNoiseModule const perlNoise, float rate);
void tPerlNoiseModule_setEnergy(tPerlNoiseModule const perlNoise, float energy);
void tPerlNoiseModule_setGain(tPerlNoiseModule const perlNoise, float gain);

#endif // ELECTORSYNTH_PERLINNOISEMODULE_H
