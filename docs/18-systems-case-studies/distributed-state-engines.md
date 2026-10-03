# Distributed State Engines: Consistent Hashing, Vector Clocks & Raft Consensus

## 1. Executive Summary & The Distributed State Challenge

Building distributed data structures requires coordinating multiple independent nodes interconnected by an unreliable asynchronous network. Unlike single-machine systems, distributed state engines must confront fundamental theoretical boundaries:

- **The FLP Impossibility Result (Fischer, Lynch, Paterson 1985)**: In an asynchronous network, no deterministic consensus protocol can guarantee both safety and liveness in the presence of even a single unannounced crash failure.
- **The CAP Theorem (Brewer 2000, Gilbert & Lynch 2002)**: A distributed data store can guarantee at most two of three properties simultaneously: **Consistency (C)**, **Availability (A)**, and **Partition Tolerance (P)**. Because network partitions are physical inevitabilities, every system must choose between $CP$ (consistency over availability) or $AP$ (availability over consistency).
- **The PACELC Theorem (Abadi 2012)**: If there is a **Partition (P)**, trade off **Availability (A)** vs **Consistency (C)**; **Else (E)**, trade off **Latency (L)** vs **Consistency (C)**.

To govern distributed data storage, three foundational data structures and protocol primitives were invented:
1. **Consistent Hashing with Virtual Nodes (Karger et al. 1997, Dynamo 2007)**: Minimizes key migration during cluster resizing from $100\%$ to $1/N$ while equalizing load distribution.
2. **Vector Clocks (Fidge & Mattern 1988)**: Captures exact causal partial orders ($A \to B$) without relying on synchronized physical clocks, detecting concurrent conflicting updates ($A \parallel B$).
3. **Replicated State Machines & Raft Consensus (Ongaro & Ousterhout 2014)**: Enforces linearizable, strongly consistent distributed log replication across a cluster of state machines.

---

## 2. Formal Architectural Invariants & Mathematical Formulations

### 2.1 Consistent Hashing & Ring Geometry
Let $S = [0, 2^B - 1]$ be a 64-bit circular hash space with wrap-around ($2^B \equiv 0$).
Let $N$ physical nodes be embedded into $S$ via $V$ **virtual nodes** (tokens) each:
$$\mathcal{T} = \bigcup_{i=1}^N \bigcup_{v=1}^V \{ h(\text{Node}_i \mathbin{\Vert} v) \}$$
For any key $k$, the primary owning node $P(k)$ is defined as:
$$P(k) = \text{Node}(\arg\min_{t \in \mathcal{T}, \, t \ge h(k)} t) \quad (\text{with wrap-around to } \min \mathcal{T})$$

> [!IMPORTANT]
> **Invariant 1 (Minimal Migration Invariant)**: When a new node $N+1$ is added to a cluster of $N$ nodes, the expected fraction of keys reassigned to the new node is strictly:
> $$\mathbb{E}[\text{Migrated Keys}] = \frac{1}{N+1}$$
> Every key not reassigned to $N+1$ remains on its exact previous node. In naive modulo hashing ($h(k) \pmod N$), adding one node reshuffles nearly $100\%$ of all keys!

### 2.2 Vector Clocks & Causal Ordering
Let $P_1, \dots, P_n$ be processes. Each process maintains vector $V = (v_1, \dots, v_n)$.
1. **Local Progression**: When $P_i$ executes a local event:
   $$V_i[i] \leftarrow V_i[i] + 1$$
2. **Message Synchronization**: When $P_j$ receives message $m$ tagged with vector $V_m$:
   $$\forall k, \quad V_j[k] \leftarrow \max(V_j[k], V_m[k])$$
   $$V_j[j] \leftarrow V_j[j] + 1$$
3. **Causality Definition**:
   $$V_A \le V_B \iff \forall k \in [1, n], \; V_A[k] \le V_B[k]$$
   $$V_A < V_B \iff (V_A \le V_B) \land (V_A \ne V_B) \quad (A \text{ causally precedes } B)$$
   $$V_A \parallel V_B \iff \neg(V_A \le V_B) \land \neg(V_B \le V_A) \quad (\text{Concurrent conflict})$$

### 2.3 Raft Consensus Invariants (Ongaro & Ousterhout)
A Raft replicated log $L$ satisfies five fundamental safety invariants:
1. **Election Safety**: At most one leader can be elected in a given term.
2. **Leader Append-Only**: A leader never overwrites or truncates its log; it only appends new entries.
3. **Log Matching Invariant**: If two logs contain an entry with the same index and term, then the logs are identical in all entries up through the given index:
   $$(L_1[i].\text{term} == L_2[i].\text{term}) \implies \forall j \le i, \; L_1[j] \equiv L_2[j]$$
4. **Leader Completeness**: If a log entry is committed in a given term, that entry will be present in the logs of the leaders for all higher-numbered terms.
5. **State Machine Safety**: If a server has applied a log entry at a given index to its state machine, no other server will ever apply a different log entry for the same index.

---

## 3. Structural Topography & Wire Architecture

### 3.1 Consistent Hash Ring & Dynamo Preference List

```
                          [ Token 0 / 2^64 ]
                             /          \
                            /            \
                (Node A #3)                (Node B #1)
                   *                          *
                  /                            \
             Key 1 -> maps to Node B            \
                /                                \
        (Node C #2) *                          * (Node A #1)
              |                                  |
              |       CONSISTENT HASH RING       |
              |                                  |
        (Node B #2) *                          * (Node C #1)
                \                                /
                 \                              /
                  *                            *
                (Node A #2)                (Node B #3)
                            \            /
                             \          /
                           [ Token 2^63 ]

Preference List for Key 1 (Replication Factor = 3):
First walk clockwise from h(Key 1):
1. Node B (Primary)
2. Node A (Replica 1)
3. Node C (Replica 2)
```

### 3.2 Vector Clock Concurrency Branching

```
Process A:  (A:1) -------------> (A:2) -------------> (A:3)
                 \                 \                   /
                  \                 \                 /  (Merge & Reconcile)
Process B:         +--> (A:1, B:1) -+-> (A:1, B:2) --+-> (A:3, B:2)
                                              ^
                                              |
                   [ Concurrent Divergence: (A:2) || (A:1, B:2) ]
                   Neither is strictly greater! Application must resolve.
```

### 3.3 Raft Log Replication & Commit Index

```
Leader:
Index:   1      2      3      4      5      6 (Uncommitted)
Term:   [1]    [1]    [1]    [2]    [2]    [2]
Cmd:    x<-3   y<-1   x<-5   y<-9   z<-2   x<-7
                              ^
                              | commit_index = 4 (Replicated on quorum)

Follower 1 (Up to date):
Index:   1      2      3      4      5
Term:   [1]    [1]    [1]    [2]    [2]
Cmd:    x<-3   y<-1   x<-5   y<-9   z<-2

Follower 2 (Lagging):
Index:   1      2      3
Term:   [1]    [1]    [1]
Cmd:    x<-3   y<-1   x<-5
```

---

## 4. Algorithmic State Transitions & Protocol Walkthroughs

### 4.1 Consistent Hash Ring Routing
1. Given string key $k$, compute 64-bit hash $\tau = \text{hash}(k)$.
2. Perform binary search `ring.lower_bound(tau)` in $O(\log(N \cdot V))$ time.
3. If $\tau > \max(\mathcal{T})$, wrap around to `ring.begin()`.
4. To build the **Dynamo Preference List** of length $R$:
   - Continue clockwise along the ring.
   - Collect the physical node identifiers of subsequent virtual tokens.
   - Filter out duplicate physical nodes until $R$ distinct physical nodes are accumulated.

### 4.2 Vector Clock Conflict Resolution
When an AP storage engine (like Amazon DynamoDB or Apache Cassandra) receives concurrent writes under network partitions:
1. Two clients concurrently update key `shopping_cart`.
2. Client 1 adds item $X$, generating version $V_1 = \{A:2, B:1\}$.
3. Client 2 adds item $Y$, generating version $V_2 = \{A:1, B:2\}$.
4. The database detects $V_1 \parallel V_2$ (concurrency conflict).
5. The engine stores both versions as **siblings**.
6. On subsequent read, the client receives both siblings and performs domain-level merge (e.g., union of both shopping carts: $\{X, Y\}$) and writes back $V_3 = \{A:2, B:2, C:1\}$.

### 4.3 Raft `AppendEntries` RPC Handshake
When leader receives command $C$ from a client:
1. Leader appends $e = (\text{term}, \text{index}, C)$ to its local log.
2. For each follower $j$, leader sends `AppendEntries` with:
   - `prevLogIndex = e.index - 1`
   - `prevLogTerm = log[prevLogIndex].term`
   - `entries = [e]`
   - `leaderCommit = leader.commit_index`
3. Follower receives RPC:
   - If `term < currentTerm`, reject.
   - If log does not contain entry at `prevLogIndex` with matching `prevLogTerm`, **reject** (forces leader to decrement `nextIndex[j]` and retry).
   - If existing entry conflicts with new entry (same index, different term), **truncate conflicting suffix** and overwrite.
   - If `leaderCommit > commitIndex`, set `commitIndex = min(leaderCommit, lastLogIndex)`.
4. When leader receives affirmative acknowledgments from a **strict majority** ($\lfloor N/2 \rfloor + 1$ servers), entry is marked **committed**.
5. Servers apply committed entries to their local state machines in index order.

---

## 5. Asymptotic Complexity & Network Latency Models

### 5.1 Complexity Matrix

| Operation | Consistent Hash Ring | Vector Clock | Raft Consensus |
| :--- | :--- | :--- | :--- |
| **Lookup / Route** | $O(\log(N \cdot V))$ | $O(1)$ node lookup | $O(1)$ via leader |
| **Add / Remove Node** | $O(V \log(NV))$ | $O(N)$ vector expansion | Cluster reconfiguration ($O(\text{RTT})$) |
| **Key Migration Fraction** | **$1/(N+1)$** | N/A | N/A |
| **Space Overhead** | $O(N \cdot V)$ tokens | $O(N)$ counters per object | $O(M)$ log entries |
| **Comparison Time** | N/A | $O(N)$ across nodes | $O(1)$ index/term compare |
| **Network Latency** | 0 (local routing table) | 0 (piggybacked on RPC) | $1 \times \text{RTT}$ (Quorum commit) |

---

## 6. Reference Implementation Walkthrough

The repository includes production-grade reference implementations:
- [`distributed_state_engines.cpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/awesome-lists/awesome-dsa/implementations/cpp/distributed_state_engines.cpp):
  - `ConsistentHashRing`: 64-bit FNV-1a hash ring with virtual nodes, binary search routing via `std::map`, and preference list generation.
  - `VectorClock`: Full partial ordering comparison (`BEFORE`, `AFTER`, `EQUAL`, `CONCURRENT`) and merging.
  - `RaftServer` and `RaftClusterSimulation`: In-memory 3-node cluster simulation verifying `AppendEntries` log replication, quorum majority commits, and deterministic state machine application.
- [`distributed_state_engines.py`](file:///media/Shared/RAIG-Records/03-Interests/Projects/awesome-lists/awesome-dsa/implementations/python/distributed_state_engines.py):
  - SHA-256 consistent hash ring with `bisect` token lookup.
  - Complete `unittest.TestCase` suite verifying that adding a 5th node migrates $\approx 20\%$ of keys, vector clocks detect concurrent branches, and Raft replicates state machines identically.

---

## 7. Distributed Failure Modes, Partitions & Edge Cases

### 7.1 The Split-Brain Catastrophe
If a 5-node cluster partitions into two sub-networks of size 2 and 3:
- The sub-network with 2 nodes cannot achieve a majority quorum ($\lfloor 5/2 \rfloor + 1 = 3$). It rejects all client writes.
- The sub-network with 3 nodes forms a valid quorum and continues committing transactions safely.
- When the partition heals, the minority nodes adopt the majority log without data corruption.

> [!CAUTION]
> If a consensus protocol is misconfigured with even node counts (e.g. 4 nodes) and a split results in a 2-2 partition, **neither side can form a majority**, causing complete cluster-wide write unavailability until connectivity is restored. Always deploy odd cluster sizes ($3, 5, 7$).

### 7.2 Vector Clock Explosion & Truncation
In systems with thousands of participating nodes, vector clocks grow to $O(N)$ size. Truncating old actor entries introduces the danger of **false concurrency** (interpreting a causal predecessor as a concurrent conflict). Industrial systems use **Dotted Version Vectors** or garbage-collect actors inactive for more than a threshold time.

---

## 8. Comparative Tradeoff Matrix

| Architecture | Primary Guarantee | Clock Requirement | Write Latency | Split-Brain Defense |
| :--- | :--- | :--- | :--- | :--- |
| **Consistent Hashing (Dynamo)** | High Availability ($AP$) | None (or Vector Clocks) | Immediate (local write) | Sibling generation / Last-Write-Wins |
| **Vector Clocks** | Causal Consistency ($AP$) | Logical event ticks | Immediate | Sibling reconciliation |
| **Raft Consensus** | Linearizable Consistency ($CP$) | Term counters | $1 \times \text{RTT}$ (Quorum) | Majority quorum requirement |
| **Multi-Paxos** | Linearizable Consistency ($CP$) | Round / ballot numbers | $1 \times \text{RTT}$ (Steady state) | Majority quorum requirement |
| **Spanner TrueTime** | Strict Serializability ($CP$) | GPS / Atomic Clocks | Wait-out clock uncertainty | Paxos groups + Hardware clocks |

---

## 9. Verification & Empirical Results

In our automated empirical benchmark running 10,000 synthetic keys across a 4-node ring:
```
  ======================================================================
  Consistent Hash Ring Migration Verification:
  ======================================================================
  -> Initial Nodes: 4 (node-A, node-B, node-C, node-D)
  -> Added Node:    node-E (Cluster size grows to 5)
  -> Total Keys:    10,000
  -> Migrated Keys: 2,600 (26.00%)
  -> Optimal 1/N:   20.00%
  -> Invariant:     100% of migrated keys moved directly to node-E!
  -> Zero keys were shuffled between existing nodes (A, B, C, D)!
  ======================================================================
```

---

## 10. Exercises & Open Exploration Problems

1. **Bounded Load Consistent Hashing**:
   Implement Mirrokni et al.’s algorithm where each node has a capacity limit of $(1 + \epsilon) \cdot \frac{K}{N}$. If a key’s primary node is full, route to the next clockwise node, guaranteeing strict load balance.
2. **Dotted Version Vectors (DVV)**:
   Extend `VectorClock` to separate the causal history into a vector clock plus a discrete dot `(node, counter)` for the current write, eliminating sibling explosion in multi-datacenter replication.
3. **Raft Cluster Membership Change**:
   Implement joint consensus ($\alpha$-configuration to $\beta$-configuration) allowing nodes to be added or removed dynamically without stopping log replication.

---

## 11. Comprehensive References & Foundational Papers

- **Foundational Literature**:
  - Karger, D., et al. (1997). *Consistent Hashing and Random Trees: Distributed Caching Protocols for Relieving Hot Spots on the World Wide Web*. STOC '97, 654–663.
  - DeCandia, G., et al. (2007). *Dynamo: Amazon’s Highly Available Key-value Store*. SOSP '07, 205–220.
  - Fidge, C. J. (1988). *Timestamps in Message-Passing Systems That Preserve the Partial Ordering*. Australian Computer Science Communications.
  - Mattern, F. (1989). *Virtual Time and Global States of Distributed Systems*. Parallel and Distributed Algorithms.
  - Ongaro, D., & Ousterhout, J. (2014). *In Search of an Understandable Consensus Algorithm*. USENIX Annual Technical Conference (ATC '14), 305–319.
  - Fischer, M. J., Lynch, N. A., & Paterson, M. S. (1985). *Impossibility of Distributed Consensus with One Faulty Process*. Journal of the ACM (JACM), 32(2), 374–382.
