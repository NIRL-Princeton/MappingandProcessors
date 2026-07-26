//
// Created by Matthew McWeeney on 7/15/26.
//

#ifndef ELECTORSYNTH_PERLINNOISEMODULE_H
#define ELECTORSYNTH_PERLINNOISEMODULE_H
#include "leaf.h"
#include "defs.h"
#include "leaf-mempool.h"
#include "leaf-oscillators.h"

typedef enum
{
    PerlNoiseEventWatchFlag,
    PerlNoiseAudioIn,
    PerlNoiseGain,
    PerlNoiseRate,
    PerlNoiseEnergy

} PerlNosParams;

typedef struct _tPerlNoiseModule
{
    ModuleHeader header;

    void* thePerlNoise;

    float gain;
    float rateMs;

    tMempool* mempool;

} _tPerlNoiseModule;

typedef _tPerlNoiseModule* tPerlNoiseModule;

//init module
void tPerlNoiseModule_init(void** const perlNoise, float* const params, float id, LEAF* const leaf);
void tPerlNoiseModule_initToPool(void** const perlNoise, float* const params, float id, tMempool** const mempool);
void tPerlNoiseModule_free(void** const perlNoise);
void tPerlNoiseModule_setParameter(tPerlNoiseModule const perlNoise, PerlNosParams param_type, float input);
// Modulatable setters
void tPerlNoiseModule_tick (tPerlNoiseModule const perlNoise);

void tPerlNoiseModule_setRate(LEAF* const leaf, tPerlNoiseModule const perlNoise, float rate);
void tPerlNoiseModule_setEnergy(LEAF* const leaf, tPerlNoiseModule const perlNoise, float energy);

#endif // ELECTORSYNTH_PERLINNOISEMODULE_H
