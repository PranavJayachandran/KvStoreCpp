#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void engine_init();

void engine_put(const char *key, const char *value);

int engine_get(const char *key, char *buffer, int buffer_size);

void engine_delete(const char *key);

#ifdef __cplusplus
}
#endif
