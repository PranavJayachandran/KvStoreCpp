package main

import (
	"encoding/json"
	"log"
	"net/http"
	"strings"

	"kvstore/internal/kv"
)

var store *kv.Store

func main() {
	store = kv.NewStore()

	http.HandleFunc("/key/", keyHandler)

	log.Println("Server running on :8080")
	log.Fatal(http.ListenAndServe(":8080", nil))
}

func keyHandler(w http.ResponseWriter, r *http.Request) {
	key := strings.TrimPrefix(r.URL.Path, "/key/")

	if key == "" {
		http.Error(w, "Key required", http.StatusBadRequest)
		return
	}

	switch r.Method {

	case http.MethodPut:
		value := r.URL.Query().Get("value")
		store.Put(key, value)
		w.WriteHeader(http.StatusOK)

	case http.MethodGet:
		value, ok := store.Get(key)
		if !ok {
			http.NotFound(w, r)
			return
		}

		json.NewEncoder(w).Encode(map[string]string{
			"key":   key,
			"value": value,
		})

	case http.MethodDelete:
		store.Delete(key)
		w.WriteHeader(http.StatusOK)

	default:
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
	}
}
