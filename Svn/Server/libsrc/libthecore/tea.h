#pragma once
#include <cstdint>

extern int32_t TEA_Encrypt(uint32_t *dest, const uint32_t *src, const uint32_t *key, int32_t size);
extern int32_t TEA_Decrypt(uint32_t *dest, const uint32_t *src, const uint32_t *key, int32_t size);

