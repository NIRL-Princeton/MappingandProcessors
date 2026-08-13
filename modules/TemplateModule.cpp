//
// Created by Myra Norton on 8/13/26.
//

#include "TemplateModule.h"
#include <assert.h>

// TODO: Change every instance of "Template" in this file to the capitalized name of your module
// TODO: Change every instance of "template" and "template_" in this file to the name of your module (not capitalized)

/* init() ensures that the resTable has been initialized and calls initToPool() with the mempool and resTable
 * associated with the LEAF instance passed into it. No changes needed here. */
void tTemplateModule_init(void** const template_, float* params, float id, LEAF* const leaf) {
	if(leaf->resTable == NULL) {
		tLookupTable_create(&leaf->mempool,&leaf->resTable);
		tLookupTable_init(leaf, leaf->resTable,  0.0f, 10.0f, 0.5f, 2048);
	}
    tTemplateModule_initToPool(template_, params, id, &leaf->mempool, leaf->resTable);
}

/* initToPool() are the functions that initialize your module. init() will call initToPool() with the default
 * values, but a user can call initToPool() directly to specify the mempool and resTable. */
void tTemplateModule_initToPool(void** const template_, float* const params, float id, tMempool** const mempool, tLookupTable* resTable)
{
    tMempool* m = *mempool;
    _tTemplateModule* TemplateModule =(_tTemplateModule *) ( *template_ = (_tTemplateModule*) mpool_alloc(sizeof(_tTemplateModule), m));
    #ifndef __cplusplus
        memcpy(TemplateModule->params, params, TemplateNumParams*sizeof(float));
    #endif

    // initialize attributes of the module
    TemplateModule->header.uniqueID = id;
    TemplateModule->mempool = m;
    TemplateModule->sr = m->leaf->sampleRate;
    TemplateModule->invSr = m->leaf->invSampleRate;
    // TODO: Add ModuleTypeTemplateModule to the ModuleType enum in defs.h
    TemplateModule->header.moduleType = ModuleTypeTemplateModule;

    // tTemplate_create() and tTemplate_init() are functions that should exist in LEAF for your module
    tTemplate_create(mempool, (tTemplate**)&TemplateModule->theTemplate);
    tTemplate_init(m->leaf,(tTemplate*)TemplateModule->theTemplate, 500.0f, 50000.f);

    #ifndef __cplusplus
        for (int i = 0; i < TemplateNumParams; i++)
        {
    	    tTemplateModule_setParameter(TemplateModule, i, TemplateModule->params[i]);

        }
    #endif
}

/* calls tTemplate_free(), which is a function that should exist in LEAF for your module*/
void tTemplateModule_free(void** const template_)
{
    _tTemplateModule* TemplateModule =(_tTemplateModule*) *template_;
    // tTemplate_free() and tTemplate should exist in LEAF for your module
    tTemplate_free((tTemplate**)TemplateModule->theTemplate);
    mpool_free((char*)TemplateModule, TemplateModule->mempool);

    // TODO: uncomment below if your module has multiple types
    //int type = roundf(CPPDEREF TemplateModule->header.params[TemplateType]);
}

void tDelayModule_free(void** const delay)
{
    _tDelayModule* DelayModule =(_tDelayModule*) *delay;
    int type = roundf(CPPDEREF DelayModule->header.params[DelayType]);
    tDelay_free((tDelay**)DelayModule->theDelay);
    mpool_free((char*)DelayModule, DelayModule->mempool);
}

/* sets the audio sample to the output of tTemplate_tick(), which is a function that should exist in LEAF for your
 * module*/
void tTemplateModule_tick (tTemplateModule const template_, float* buffer)
{
    buffer[0] = template_->header.outputs[0] = tTemplate_tick((tTemplate*)template_->theTemplate,  buffer[0]) * template_->amp + buffer[0];
}

/* private helper function for setting a parameter */
static void tTemplateModule_setParam1(tTemplateModule const template_, float const time) {
    // LEAF should have a function for setting a parameter. Make sure you change the function inputs to be what it needs
    tTemplate_setParam1((tTemplate*)template_->theTemplate, time*template_->sr);
}

/* changes the modules internal parameters when a user changes it */
void tTemplateModule_setParameter(tTemplateModule const template_, TemplateParams param_type,float input)
{
    switch (param_type) {
        //TODO: create a case for each param type. Create a helper function as shown above.

        // case TemplateParam1:
        //     tTemplateModule_setParam1(template_, input);
        //     break;
        // case TemplateParam2:
        //     tTemplateModule_setParam2(template_, input);
        //     break;
        default:
            break;
    }
}

/* sets the module's attributes related to the decibel table  */
void tTemplateModule_setDBtoATableLocation (tTemplateModule const template_, float* const tableAddress, uint32_t const tableSize)
{
    template_->dbTableAddress = tableAddress;
    template_->dbTableSizeMinusOne = (float)(tableSize - 1);
    template_->dbTableScalar = template_->dbTableSizeMinusOne/(4.0f-0.00001f);
    template_->dbTableOffset = 0.00001f * template_->dbTableScalar;
}

/* calls the relevant LEAF function for setting the sample rate  */
void tTemplateModule_setSampleRate (tTemplateModule const template_, float const sr)
{
    //tCycle_setSampleRate(template_->template_s[0], sr);
}