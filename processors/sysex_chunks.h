//
// Created by Davis Polito on 2/13/25.
//

#ifndef LEAF_MODULES_SYSEX_CHUNKS_H
#define LEAF_MODULES_SYSEX_CHUNKS_H
#ifdef __cplusplus
#include <stdio.h>

#endif
#include <stdint.h>
void splitUint8To7bit(uint8_t byte, uint8_t chunked[2]);
void splitFloatTo7bit(float val,uint8_t chunked[5]);
uint8_t unsplitUint8(const uint8_t chunks[2]);
                     float unsplitFloat(const uint8_t chunks[5]);
uint32_t unsplitUint32(const uint8_t chunks[5]);
void splitInt32To7bit (int32_t val, uint8_t chunks[5]);
void splitUint32To7bit (uint32_t val, uint8_t chunks[5]);
int32_t unsplitInt32(const uint8_t chunks[5]);

#endif //LEAF_MODULES_SYSEX_CHUNKS_H
