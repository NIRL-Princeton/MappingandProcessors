//
// Created by Matthew McWeeney on 7/9/26.
//

#ifndef ELECTORSYNTH_NOISEMODULE_H
#define ELECTORSYNTH_NOISEMODULE_H
#include "leaf.h"
#include "defs.h"
#include "leaf-mempool.h"
#include "leaf-oscillators.h"
#include "leaf-filters.h"

typedef enum
{
    NoiseEventWatchFlag,
    NoiseGain,
    NoiseTilt,
    NoisePeakGain,
    NoisePeakFreq,
    NoisePeakBandwidth

} NosParams;

typedef struct _tNoiseModule
{
    ModuleHeader header;

    void* theNoise;

    // float* dbTableAddress;
    // uint32_t dbTableScalar;
    // float dbTableOffset;
    // float dbTableSizeMinusOne;

    //float* mtofTable;

    float gain;
    float tilt;
    float peakGain;
    float peakFreq;
    float peakBandwidth;
    float sr;
    float invSr;
    tTiltFilter theTilter;
    tVZFilterBell theBellter;

    tMempool* mempool;
    //float(*rand)(void);
    //tLookupTable* table;
} _tNoiseModule;

typedef _tNoiseModule* tNoiseModule;

//init module
void tNoiseModule_init(void** const noise, float* const params, float id, LEAF* const leaf);
void tNoiseModule_initToPool(void** const noise, float* const params, float id, tMempool** const mempool, tLookupTable* resTable);
void tNoiseModule_free(void** const noise);
void tNoiseModule_setParameter(tNoiseModule const noise, NosParams param_type,float input);
// Modulatable setters
void tNoiseModule_tick (tNoiseModule const noise, float*);

// Non-modulatable setters
//void tNoiseModule_setSampleRate (tNoiseModule const noise, float sr);
//void tNoiseModule_setMTOFTableLocation (tNoiseModule const noise, float* tableAddress);
//void tNoiseModule_setDBtoATableLocation (tNoiseModule const noise, float* tableAddress, uint32_t tableSize);
//float dbToATableLookupFunctionNos(float const in, float const sizeMinusOne, float* const tableAddress);

#endif // ELECTORSYNTH_NOISEMODULE_H
