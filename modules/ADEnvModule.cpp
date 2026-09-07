//
// Created by Matthew McWeeney on 9/6/26.
//

#include "ADEnvModule.h"
#include <cstdio>
#include <assert.h>

void tADEnvModule_init (void** const env, float* params, float id, LEAF* const leaf)
{
    tADEnvModule_initToPool (env, params, id, &leaf->mempool);
}

void tADEnvModule_free (void** const env)
{
    _tADEnvModule* ADEnvModule = (_tADEnvModule*) (*env);
    mpool_free ((char*) ADEnvModule, ADEnvModule->mempool);
}

//tick function
void tADEnvModule_tick (tADEnvModule const env)
{
    CPPDEREF env->header.params[ADEnvVelocitySense] = tSlopeRamp_tick(&env->velSenseSmoother);
    float out = tAD_tick (&env->theEnv);
    env->header.outputs[0] = LEAF_clip(0, out, 1);
    //printf("%f\n", out);
}

//special noteOnFunction
void tADEnvModule_onNoteOn (tADEnvModule const env, float velocity)
{
    float envVel = velocity;
    if (velocity > 0.0001f)
    {
        float velSense = CPPDEREF env->header.params[ADEnvVelocitySense];
        envVel = envVel * velSense + (1.0f - velSense);
        tAD_on (&env->theEnv, envVel);
        //printf("On\n");
    }
}

void tADEnvModule_setTimeScalingTableLocation (tADEnvModule const env, const float* tableAddress, uint32_t const tableSize)
{
    env->envTimeTableAddress = tableAddress;
    env->envTimeTableSizeMinusOne = (float) (tableSize - 1);
}

void tADEnvModule_setSampleRate (tADEnvModule const env, float sr)
{
    //how to handle this? if then cases for different types?
}
void tADEnvModule_setParameter (tADEnvModule const env, int parameter_id, float input)
{
    switch (parameter_id)
    {
        case ADEnvEventWatchFlag:
        {
            // handled by onnoteon listener
            break;
        }

        case ADEnvAttack:
        {
            input *= env->envTimeTableSizeMinusOne;
            int const inputInt = (int) input;
            float const inputFloat = input - (float)inputInt;
            int const nextPos = LEAF_clip (0.0f, inputInt + 1.0f, env->envTimeTableSizeMinusOne);
            float const theValue = LEAF_clip (0.1f, (env->envTimeTableAddress[inputInt] * (1.0f - inputFloat)) + (env->envTimeTableAddress[nextPos] * inputFloat), 20000.0f);
            tAD_setAttack (&env->theEnv, theValue + 0.001f);
            //printf("Attack: %f\n", theValue + 0.001f);
            break;
        }

        case ADEnvDecay:
        {
            input *= env->envTimeTableSizeMinusOne;
            int const inputInt = (int) input;
            float const inputFloat = input - (float) inputInt;
            int const nextPos = LEAF_clip (0.0f, inputInt + 1.0f, env->envTimeTableSizeMinusOne);
            float const theValue = LEAF_clip (0.1f, (env->envTimeTableAddress[inputInt] * (1.0f - inputFloat)) + (env->envTimeTableAddress[nextPos] * inputFloat), 20000.0f);
            tAD_setDecay (&env->theEnv, theValue + 0.001f);
            //printf("Decay: %f\n", theValue + 0.001f);
            break;
        }

        case ADEnvVelocitySense:
        {
            tSlopeRamp_setDest (&env->velSenseSmoother, input);
            break;
        }
        default:
            break;
    }
}

void tADEnvModule_initToPool (void** const env, float* const params, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tADEnvModule* ADEnvModule = (_tADEnvModule*) (*env = (_tADEnvModule*) mpool_alloc (sizeof (_tADEnvModule), m));

    ADEnvModule->mempool = m;

    ADEnvModule->header.uniqueID = id;

    tAD_init(m->leaf, &ADEnvModule->theEnv, 1.0f, 10.f);
    tAD_setSampleRate (&ADEnvModule->theEnv, m->leaf->sampleRate);

    if ((*mempool)->leaf->envTimeTable == NULL)
    {
        tLookupTable_create (&m, &(*mempool)->leaf->envTimeTable);
        tLookupTable_init ((*mempool)->leaf, (*mempool)->leaf->envTimeTable, 0.0001f, 20000.f, 4000.f, 2048);
    }
    tADEnvModule_setTimeScalingTableLocation (ADEnvModule, (*mempool)->leaf->envTimeTable->table, 2048);

    tSlopeRamp_init(m->leaf, &ADEnvModule->velSenseSmoother, SMOOTH_SLOPE_MULTIPLIER, 1.f);
    ADEnvModule->header.moduleType = ModuleTypeADEnvModule;
}


