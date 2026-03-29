# 🌲 KvStoreCpp: High-Performance LSM-Tree Storage Engine

**KvStoreCpp** is a persistent, disk-resident Key-Value store built from scratch in C++. It utilizes a **Log-Structured Merge-Tree (LSM-Tree)** architecture to provide high write throughput and features a **Go/CGO integration layer** to bridge low-level systems performance with modern web scalability.

I built this project to master the internals of storage engines like RocksDB—specifically focusing on memory-to-disk staging, data durability (WAL), and optimizing disk I/O with probabilistic filters.

---

## 🏗️ System Architecture

The engine converts random application writes into efficient, sequential disk I/O through a tiered architecture.

### 1. The Write Path (Durability & Speed)
* **Write-Ahead Log (WAL):** Every mutation is appended to a disk-resident log before being applied to memory, ensuring **Atomicity and Durability**.
* **SkipList MemTable:** An in-memory, sorted data structure. I chose a SkipList over a Red-Black Tree for its $O(\log N)$ performance and easier transition to lock-free concurrency.

### 2. The Read Path (Optimization)
* **Bloom Filters:** A probabilistic layer that allows the engine to skip checking SSTables that definitely do not contain a specific key, reducing disk seeks by >90%.
* **Sparse Index:** Each SSTable contains an in-memory index mapping key ranges to byte offsets, allowing for fast binary searches on disk.

### 3. Storage & Maintenance
* **SSTables (Sorted String Tables):** Immutable binary files flushed from memory once the MemTable reaches its size threshold.
* **Leveled Compaction:** A background process that merges overlapping SSTables, removes **Tombstones** (logical deletes), and deduplicates keys to manage read/write amplification.

---

## 🌐 Go Integration (CGO Bridge)

To make the engine "cloud-ready," I implemented a **CGO wrapper** that exposes the C++ core to the Go ecosystem. This allows the engine to power high-level web services while maintaining C++ performance.

* **C-Compatible ABI:** A thin `extern "C"` layer wraps the C++ logic.
* **Go Bindings:** Idiomatic Go code handles pointer translation and memory safety across the language boundary.

---

## 🛠️ Build & Execution Guide

The project is split into the **C++ Core** (shared library) and the **Go Server**.

### 1. Build the Shared Library (`libkvstore`)
The Go wrapper requires a compiled shared object (`.so` or `.dylib`) to talk to the C++ engine.

```bash
mkdir build && cd build
cmake ..
make

# Move the library to the server directory for linking
cp libkvstore.so ../go-server/
```

### 2. Run the Go Server
The Go server acts as the entry point, exposing the engine via a REST/CLI interface.

```bash
cd ../go-server

# Tell the linker where to find the C++ library
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)

# Run the server
go run main.go
```

### 3. Interact with the Store
Once the server is running, you can use `curl` to interact with the LSM-Tree:

```bash
# Put a key
curl -X POST http://localhost:8080/put -d '{"key": "user:1", "value": "Pranav"}'

# Get a key
curl http://localhost:8080/get/user:1
```

---

## 📊 Technical Trade-offs & Challenges
* **Tombstone Management:** Learned that "deleting" in an LSM-tree is actually a write operation. Deletes are only physically purged during a **Major Compaction**.
* **CGO Overhead:** Navigating the "CGO is not Go" philosophy by minimizing transitions across the language boundary to maintain performance.
* **Memory Safety:** Implementing RAII in C++ to ensure the WAL and SSTable file descriptors are managed correctly without leaks.

---

## 🚀 Roadmap
- [x] **Core Engine:** LSM-Tree, WAL, SSTable, Sparse Index.
- [x] **Optimization:** Bloom Filters and Binary Search.
- [x] **Go Wrapper:** CGO implementation for language interop.
- [ ] **Concurrent Compaction:** Moving merge logic to a background thread.
- [ ] **Metrics Dashboard:** Visualizing Write Amplification and MemTable health.

---

## 👨‍💻 Author
**Pranav Jayachandran** *Software Development Engineer* [GitHub](https://github.com/PranavJayachandran) | [LinkedIn](https://linkedin.com/in/pranav-jayachandran-b971bb1b8)
