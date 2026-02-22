package kv

/*
#cgo CFLAGS: -I.
#cgo LDFLAGS: -L. -lengine -lstdc++
#include "engine_c.h"
#include <stdlib.h>
*/
import "C"

import (
	"sync"
	"unsafe"
)

type Store struct {
	mu sync.Mutex
}

func NewStore() *Store {
	C.engine_init()
	return &Store{}
}

func (s *Store) Put(key, value string) {
	s.mu.Lock()
	defer s.mu.Unlock()

	ckey := C.CString(key)
	cvalue := C.CString(value)

	C.engine_put(ckey, cvalue)

	C.free(unsafe.Pointer(ckey))
	C.free(unsafe.Pointer(cvalue))
}

func (s *Store) Get(key string) (string, bool) {
	s.mu.Lock()
	defer s.mu.Unlock()

	ckey := C.CString(key)
	defer C.free(unsafe.Pointer(ckey))

	bufferSize := 1024
	buffer := C.malloc(C.size_t(bufferSize))
	defer C.free(buffer)

	result := C.engine_get(
		ckey,
		(*C.char)(buffer),
		C.int(bufferSize),
	)

	if result <= 0 {
		return "", false
	}

	value := C.GoString((*C.char)(buffer))
	return value, true
}

func (s *Store) Delete(key string) {
	s.mu.Lock()
	defer s.mu.Unlock()

	ckey := C.CString(key)
	C.engine_delete(ckey)
	C.free(unsafe.Pointer(ckey))
}
