The **KVStore** implementation uses **LSM trees** to store data both in memory and on disk, combining the advantages of both. Three key components play a crucial role: **SkipLists**, **Bloom Filters**, and **LSM trees**.

##### **LSM Tree**
The system uses an in-memory **Memtable** and **SSTables** stored on disk. It follows a leveled storage structure, inspired by **RocksDB** and **LevelDB**. Each level has a fixed size; once this maximum size is reached, **compaction** is triggered. During compaction, random entries are moved from the current level to the next one (from level 1 to level 2, and so on). This process stops at the last level, where no further promotion occurs. Both the **Memtable** and **SSTables** have specialized implementations to optimize performance.

#### **MemTable**
A **SkipList** is used to store data in memory (although an **RBTree** implementation could also have been used). **Skip lists** provide efficient insertion, deletion, and search operations. Additionally, a **Write-Ahead Log (WAL)** is employed to ensure fault tolerance.

##### **SSTable**
**Sorted String Tables (SSTables)** are used to store data on disk. Since the data is sorted, it allows for faster querying. Each entry is saved in a key-value format (with a **#** suffix if the key was soft-deleted). Every entry is allocated a fixed size (configured to 30 bytes in the code). Because of this fixed-size layout, **binary search** can be used to significantly speed up lookups.

##### **Compaction**
When the **Memtable** reaches a certain size `n`, keeping all data in memory becomes inefficient. At this point, the data is flushed to **SSTables** on disk. Specifically, the data is written to a **Level 1 SSTable**. If **Level 1** is full, a portion of its data is pushed to **Level 2**, and this process continues recursively through higher levels until the last level (**Level n**) is reached.

##### **Bloom Filters**
**Bloom filters** are probabilistic data structures used to check whether a given key is part of a set. If a **Bloom filter** returns "**yes**", the key might be present; if it returns "**no**", the key is definitely not present.  
Since reading from disk is slow, a **Bloom filter** is maintained for each level to quickly rule out missing keys and avoid unnecessary disk lookups.

##### **Insert**
Inserts are first made to the **Memtable**, which may later be flushed to the **SSTables**.

##### **Delete**
Instead of being immediately deleted, keys are **soft-deleted** (also known as **tombstoned**). These tombstoned entries are propagated during **compaction** and are permanently removed only at the last level of the storage hierarchy.

##### **Search** 
The search always starts from the **Memtable** and proceeds down the levels from 1 to `n`. If the key is found at any level, it is returned, provided it hasn’t been **soft-deleted** (**tombstoned**). If the key is **soft-deleted**, a "**not found**" result is returned.
