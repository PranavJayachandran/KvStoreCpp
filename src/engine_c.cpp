#include "../include/kvstore/engine/engine_c.h"
#include "../include/kvstore/engine/Engine.h"
#include <cstring>

using namespace kvstore::engine;

static Engine<std::string, std::string> *engine = nullptr;

void engine_init() {
  if (!engine) {
    engine = new Engine<std::string, std::string>();
  }
}

void engine_put(const char *key, const char *value) {
  engine->Add(std::string(key), std::string(value));
}

int engine_get(const char *key, char *buffer, int buffer_size) {
  std::string value;

  if (engine->Get(std::string(key), value)) {
    if (value.size() >= buffer_size)
      return -1;

    memcpy(buffer, value.c_str(), value.size() + 1);
    return value.size();
  }

  return 0;
}

void engine_delete(const char *key) { engine->Delete(std::string(key)); }
