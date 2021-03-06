/**
 * @file   module.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2014
 * @date   Wed Feb  1 18:56:27 2017
 *
 * @brief  Header file for murmurhash.
 *
 *
 */

#ifndef RINOO_STRUCT_MURMURHASH3_H_
#define RINOO_STRUCT_MURMURHASH3_H_

void murmurhash3_x86_32(const void *key, int len, uint32_t seed, void *out);
void murmurhash3_x86_128(const void *key, int len, uint32_t seed, void *out);
void murmurhash3_x64_128(const void *key, int len, uint32_t seed, void *out);

#endif /* !RINOO_STRUCT_MURMURHASH3_H_ */
