//
// Created by Matthew McWeeney on 9/3/26.
//

#include "SimpleEnvModule.h"

#include <cstdio>

#include <assert.h>

void tSimpleEnvModule_init (void** const env, float* params, float id, LEAF* const leaf)
{
    tSimpleEnvModule_initToPool (env, params, id, &leaf->mempool);
}

void tSimpleEnvModule_free (void** const env)
{
    _tSimpleEnvModule* SimpleEnvModule = static_cast<_tSimpleEnvModule*> (*env);
    mpool_free ((char*) SimpleEnvModule, SimpleEnvModule->mempool);
}
//tick function
void tSimpleEnvModule_tick (tSimpleEnvModule const env)
{
    CPPDEREF env->header.params[EnvVelocitySense] = tSlopeRamp_tick(&env->velSenseSmoother);
    env->header.outputs[0] = tADSRT_tick (&env->theEnv);
}

//special noteOnFunction
void tSimpleEnvModule_onNoteOn (tSimpleEnvModule const env, float velocity)
{
    float envVel = velocity;
    if (velocity > 0.0001f)
    {
        float velSense = CPPDEREF env->header.params[EnvVelocitySense];
        envVel = envVel * velSense + (1.0f - velSense);
        tADSRT_on (&env->theEnv, envVel);
    }
    else
    {
        tADSRT_off (&env->theEnv);
    }
}

// Non-modulatable setters
void tSimpleEnvModule_setExpTableLocation (tSimpleEnvModule const env, const float* tableAddress, uint32_t const tableSize)
{
    env->theEnv.exp_buff = tableAddress;
    env->theEnv.buff_size = tableSize;
    env->theEnv.buff_sizeMinusOne = tableSize - 1;
    env->theEnv.bufferSizeDividedBySampleRateInMs = env->theEnv.buff_size / (env->theEnv.sampleRate * 0.001f);
}

void tSimpleEnvModule_setTimeScalingTableLocation (tSimpleEnvModule const env, const float* tableAddress, uint32_t const tableSize)
{
    env->envTimeTableAddress = tableAddress;
    env->envTimeTableSizeMinusOne = (float) (tableSize - 1);
}

void tSimpleEnvModule_setSampleRate (tSimpleEnvModule const env, float sr)
{
    //how to handle this? if then cases for different types?
}
void tSimpleEnvModule_setParameter (tSimpleEnvModule const env, int parameter_id, float input)
{
    switch (parameter_id)
    {
        case EnvEventWatchFlag:
        {
            // handled by onnoteon listener
            break;
        }

        case EnvAttack:
        {
            input *= env->envTimeTableSizeMinusOne;
            int const inputInt = (int) input;
            float const inputFloat = input - (float)inputInt;
            int const nextPos = LEAF_clip (0.0f, inputInt + 1.0f, env->envTimeTableSizeMinusOne);
            float const theValue = LEAF_clip (0.1f, (env->envTimeTableAddress[inputInt] * (1.0f - inputFloat)) + (env->envTimeTableAddress[nextPos] * inputFloat), 20000.0f);
            tADSRT_setAttack (&env->theEnv, theValue + 0.001f);
            break;
        }

        case EnvDecay:
        {
            input *= env->envTimeTableSizeMinusOne;
            int const inputInt = (int) input;
            float const inputFloat = input - (float) inputInt;
            int const nextPos = LEAF_clip (0.0f, inputInt + 1.0f, env->envTimeTableSizeMinusOne);
            float const theValue = LEAF_clip (0.1f, (env->envTimeTableAddress[inputInt] * (1.0f - inputFloat)) + (env->envTimeTableAddress[nextPos] * inputFloat), 20000.0f);
            tADSRT_setDecay (&env->theEnv, theValue + 0.001f);
            break;
        }

        case EnvSustain:
        {
            tADSRT_setSustain (&env->theEnv, input);
            break;
        }

        case EnvRelease:
        {
            input *= env->envTimeTableSizeMinusOne;
            int const inputInt = (int) input;
            float const inputFloat = input - (float) inputInt;
            int const nextPos = LEAF_clip (0.0f, inputInt + 1.0f, env->envTimeTableSizeMinusOne);
            float const theValue = LEAF_clip (0.1f, (env->envTimeTableAddress[inputInt] * (1.0f - inputFloat)) + (env->envTimeTableAddress[nextPos] * inputFloat), 20000.0f);
            tADSRT_setRelease (&env->theEnv, theValue + 0.001f);

            //printf("Env set to: %f\n", theValue + 0.001f);
            break;
        }

        case EnvLeak:
        {
            tADSRT_setLeakFactor (&env->theEnv, 0.99995f + 0.00005f * (1.f - input));
            break;
        }

        // case EnvShape:
        // {
        //     tADSRT_setShape (&env->theEnv, input);
        //     break;
        // }

        case EnvVelocitySense:
        {
            tSlopeRamp_setDest (&env->velSenseSmoother, input);
            break;
        }

        default:
            break;
    }
}

void tSimpleEnvModule_initToPool (void** const env, float* const params, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tSimpleEnvModule* SimpleEnvModule = static_cast<_tSimpleEnvModule*> (*env = (_tSimpleEnvModule*) mpool_alloc (sizeof (_tSimpleEnvModule), m));

    SimpleEnvModule->mempool = m;

    SimpleEnvModule->header.uniqueID = id;

    // exponential decay buffer falling from 1 to
    LEAF_generate_exp (SimpleEnvModule->decayExpBuffer, 0.001f, 0.0f, 1.0f, -0.0008f, DECAY_EXP_BUFFER_SIZE);
    SimpleEnvModule->expBufferSizeMinusOne = EXP_BUFFER_SIZE - 1;

    SimpleEnvModule->decayExpBufferSizeMinusOne = DECAY_EXP_BUFFER_SIZE - 1;
    tADSRT_set (&SimpleEnvModule->theEnv, 1.0f, 1000.0f, 1.0f, 1000.0f, SimpleEnvModule->decayExpBuffer, DECAY_EXP_BUFFER_SIZE, (*mempool)->leaf);
    tADSRT_setSampleRate (&SimpleEnvModule->theEnv, m->leaf->sampleRate);
    SimpleEnvModule->header.setterFunctions[EnvEventWatchFlag] = (tSetter) &tSimpleEnvModule_onNoteOn;

    tSimpleEnvModule_setExpTableLocation (SimpleEnvModule, SimpleEnvModule->decayExpBuffer, DECAY_EXP_BUFFER_SIZE);
    if ((*mempool)->leaf->envTimeTable == NULL)
    {
        tLookupTable_create (&m, &(*mempool)->leaf->envTimeTable);
        tLookupTable_init ((*mempool)->leaf, (*mempool)->leaf->envTimeTable, 0.0001f, 20000.f, 4000.f, 2048);
    }
    tSimpleEnvModule_setTimeScalingTableLocation (SimpleEnvModule, (*mempool)->leaf->envTimeTable->table, 2048);

    tSlopeRamp_init(m->leaf, &SimpleEnvModule->velSenseSmoother, SMOOTH_SLOPE_MULTIPLIER, 1.f);
    SimpleEnvModule->header.moduleType = ModuleTypeEnvModule;
}

