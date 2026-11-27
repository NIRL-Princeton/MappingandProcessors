//
// Created by Matthew McWeeney on 9/6/26.
//

#include "ADEnvModule.h"
#include "string.h"

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
    float out = tADSRT_tick (&env->theEnv);
    env->header.outputs[0] = out;
    //printf("%f\n", out);
}

//special noteOnFunction
void tADEnvModule_onNoteOn (tADEnvModule const env, float velocity)
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
}


void tADEnvModule_setSampleRate (tADEnvModule const env, float sr)
{

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

            float const theValue = LEAF_interpolateTableLookupFunction(input, env->envTimeTable);
            tADSRT_setAttack (&env->theEnv, theValue);
            break;
        }

        case ADEnvDecay:
        {
        	 float const theValue = LEAF_interpolateTableLookupFunction(input, env->envTimeTable);
             tADSRT_setDecay (&env->theEnv, theValue);
            //printf("Decay: %f\n", theValue + 0.001f);
            break;
        }

        case ADEnvVelocitySense:
        {
            env->velSense =  input;
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
#ifndef __cplusplus
    for (int i = 0; i < ADEnvNumParams; i++)
    {
    	ADEnvModule->header.params[i] = params[i];
    }
#endif __cplusplus
    ADEnvModule->mempool = m;

    ADEnvModule->header.uniqueID = id;

    if ((*mempool)->leaf->decayExpBuffer == NULL)
    {
        // exponential decay buffer falling from 1 to
        tLookupTable_create (&m, &(*mempool)->leaf->decayExpBuffer);
        tLookupTable_init ((*mempool)->leaf, (*mempool)->leaf->decayExpBuffer, 0.0001f, 20000.f, 4000.f, DECAY_EXP_BUFFER_SIZE);
        LEAF_generate_exp ((*mempool)->leaf->decayExpBuffer->table, 0.001f, 0.0f, 1.0f, -0.0008f, DECAY_EXP_BUFFER_SIZE);
    }
    tADSRT_set (&ADEnvModule->theEnv, 1.0f, 1000.0f, 0.0f, 1000.0f, (*mempool)->leaf->decayExpBuffer->table, DECAY_EXP_BUFFER_SIZE, (*mempool)->leaf);
    tADSRT_setSampleRate (&ADEnvModule->theEnv, m->leaf->sampleRate);


    if ((*mempool)->leaf->envTimeTable == NULL)
    {
        tLookupTable_create (&m, &(*mempool)->leaf->envTimeTable);
        tLookupTable_init ((*mempool)->leaf, (*mempool)->leaf->envTimeTable, 0.0001f, 20000.f, 4000.f, 2048);
    }
    ADEnvModule->envTimeTable = (*mempool)->leaf->envTimeTable;

    ADEnvModule->header.moduleType = ModuleTypeADEnvModule;
#ifndef __cplusplus
for (int i = 0; i < ADEnvNumParams; i++)
{
    tADEnvModule_setParameter(ADEnvModule, i, ADEnvModule->header.params[i]);
}
#endif
}

