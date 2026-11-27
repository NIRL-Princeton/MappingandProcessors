//
// Created by Matthew McWeeney on 9/3/26.
//

#include "SimpleEnvModule.h"

//#include <cstdio>

#include "string.h"
#include <assert.h>

void tSimpleEnvModule_init (void** const env, float* params, float id, LEAF* const leaf)
{
    tSimpleEnvModule_initToPool (env, params, id, &leaf->mempool);
}

void tSimpleEnvModule_free (void** const env)
{
    _tSimpleEnvModule* SimpleEnvModule = (_tSimpleEnvModule*) (*env);
    mpool_free ((char*) SimpleEnvModule, SimpleEnvModule->mempool);
}
//tick function
void tSimpleEnvModule_tick (tSimpleEnvModule const env)
{

    float out = tADSRT_tick (&env->theEnv);
    env->header.outputs[0] = out;
    //printf("%f\n", out);
}

//special noteOnFunction
void tSimpleEnvModule_onNoteOn (tSimpleEnvModule const env, float velocity)
{
    float envVel = velocity;
    if (velocity > 0.005f)
    {
        //velocity sensitivity from mystran on KVR forum
        // this would be a parameter from 0 to some maximum
        // I just tried 20 and it feels nice

        float velocity_dB_scale = 40.0f * env->velSense;

        // map midi velocities [0, 127] to [0, 1] doubles
        // MIDI velocity 0 has already been converted to note off
        // but it doesn't really make any huge difference

        // velo_level is the constant to scale the note with
        float velo_level = powf(10.0f, velocity_dB_scale*(velocity - 0.7f) * 0.025f); //divide by 20 equals  * 0.05f

        tADSRT_on (&env->theEnv, velo_level);
    }
    else
    {
        tADSRT_off (&env->theEnv);
    }
}


void tSimpleEnvModule_setSampleRate (tSimpleEnvModule const env, float sr)
{
    //how to handle this? if then cases for different types?
}
void tSimpleEnvModule_setParameter (tSimpleEnvModule const env, int parameter_id, float input)
{
    switch (parameter_id)
    {
        case SimpEnvEventWatchFlag:
        {
            // handled by onnoteon listener
            break;
        }

        case SimpEnvAttack:
        {

            float const theValue = LEAF_interpolateTableLookupFunction(input, env->envTimeTable);
            tADSRT_setAttack (&env->theEnv, theValue);
            //printf("Attack: %f\n", theValue + 0.001f);
            break;

        }

        case SimpEnvDecay:
        {

            float const theValue = LEAF_interpolateTableLookupFunction(input, env->envTimeTable);
            tADSRT_setDecay (&env->theEnv, theValue);
            break;

        }

        case SimpEnvSustain:
        {
            tADSRT_setSustain (&env->theEnv, input);
            //printf("Sustain: %f\n", input);
            break;
        }

        case SimpEnvRelease:
        {

            float const theValue = LEAF_interpolateTableLookupFunction(input, env->envTimeTable);
            tADSRT_setRelease (&env->theEnv, theValue);
            break;

        }

        case SimpEnvLeak:
        {
            tADSRT_setLeakFactor (&env->theEnv, 0.99995f + 0.00005f * (1.f - input));
            //printf("Leak: %f\n", 0.99995f + 0.00005f * (1.f - input));
            break;
        }

        // case EnvShape:
        // {
        //     tADSRT_setShape (&env->theEnv, input);
        //     break;
        // }

        case SimpEnvVelocitySense:
        {
            env->velSense = input;
            break;
        }

        default:
            break;
    }
}

void tSimpleEnvModule_initToPool (void** const env, float* const params, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tSimpleEnvModule* SimpleEnvModule = (_tSimpleEnvModule*) (*env = (_tSimpleEnvModule*) mpool_alloc (sizeof (_tSimpleEnvModule), m));
    #ifndef __cplusplus
    memcpy (SimpleEnvModule->header.params, params, SimpEnvNumParams * sizeof(float));
    #endif __cplusplus
    SimpleEnvModule->mempool = m;

    SimpleEnvModule->header.uniqueID = id;

    if ((*mempool)->leaf->decayExpBuffer == NULL)
    {
        // exponential decay buffer falling from 1 to
        tLookupTable_create (&m, &(*mempool)->leaf->decayExpBuffer);
        tLookupTable_init ((*mempool)->leaf, (*mempool)->leaf->decayExpBuffer, 0.0001f, 20000.f, 4000.f, DECAY_EXP_BUFFER_SIZE);
        LEAF_generate_exp ((*mempool)->leaf->decayExpBuffer->table, 0.001f, 0.0f, 1.0f, -0.0008f, DECAY_EXP_BUFFER_SIZE);
    }
    tADSRT_set (&SimpleEnvModule->theEnv, 1.0f, 1000.0f, 1.0f, 1000.0f, (*mempool)->leaf->decayExpBuffer->table, DECAY_EXP_BUFFER_SIZE, (*mempool)->leaf);
    tADSRT_setSampleRate (&SimpleEnvModule->theEnv, m->leaf->sampleRate);
    //EnvModule->header.setterFunctions[EnvEventWatchFlag] = (tSetter) &tEnvModule_onNoteOn;
    if ((*mempool)->leaf->envTimeTable == NULL)
    {
        tLookupTable_create (&m, &(*mempool)->leaf->envTimeTable);
        tLookupTable_init ((*mempool)->leaf, (*mempool)->leaf->envTimeTable, 0.0001f, 20000.f, 4000.f, 2048);
    }
    SimpleEnvModule->envTimeTable = (*mempool)->leaf->envTimeTable;
    //tEnvModule_setTimeScalingTableLocation (EnvModule, (*mempool)->leaf->envTimeTable->table, 2048);

    SimpleEnvModule->header.moduleType = ModuleTypeEnvModule;
    #ifndef __cplusplus
    for (int i = 0; i < SimpEnvNumParams; i++)
    {
        tSimpleEnvModule_setParameter(SimpleEnvModule, i, SimpleEnvModule->header.params[i]);
    }
    #endif
}

