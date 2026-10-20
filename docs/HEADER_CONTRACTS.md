# Module ownership and headers

This is a **foundation**, not completed application code. TODO stubs compile,
but throw deliberate `logic_error` on exercise use. Do not auto-solve.

| File | Assigned to | Must implement | Expected complexity |
| --- | --- | --- | --- |
| ds/DynamicArray.h | A | reserve, append, indexed access, erase, copy/move | Amortized append O(1) |
| ds/SinglyLinkedList.h | A | append, remove, chain traversal | O(1) tail append |
| ds/DoublyLinkedList.h | A | node handles, append, unlink, pop head | O(1) known-node unlink |
| ds/Stack.h | A | push, pop, top | O(1) amortized |
| ds/CircularQueue.h | A | ring buffer and expansion | O(1) amortized |
| algo/Sorting.h | A | stable merge, insertion, optional quicksort | O(n log n) merge |
| ds/Hashing.h | B | deterministic hash for integer and string keys | O(key length) strings |
| ds/HashTable.h | B | chaining, lookup, insert, erase, rehash | O(1) expected |
| ds/DeadlineAVL.h | B | insert, delete, range, rotations, validate | O(log n) updates |
| algo/Searching.h | B | linear, binary lower bound | O(n), O(log n) |
| ds/DirectedGraph.h | C | adjacency, edges, cycle-safe insertion | DFS O(V+E) |
| algo/GraphAlgorithms.h/.cpp | C | DFS reachability and Kahn ordering | O(V+E) |
| domain/Matching.h/.cpp | C | **already working** | O(number of requirements) |
| core/AppState.h | B/C | record ownership and derived-index consistency | Module-dependent |
| core/WaitingList.h | A | coordinate DLL+hash handles | O(1) expected removal |
| core/UndoHistory.h | A | profile-only undo | O(1) |

## Interface design guardrails
- Header-only templates; `.cpp` for non-template algorithms and services.
- Existing models, signatures and data flow are a starting contract. Team may
  revise in **one reviewed interfaces PR** before parallel coding.
- AppState owns records. Indexes store IDs or array positions, not raw pointers
  to arrays that can grow. Avoid copying ownership and stale DLL handles.
- `SinglyLinkedList` currently lacks `removeIf` (needed for hash chaining).
  Member A and B should agree on this extension before implementations diverge.
- A `DynamicArray<T>` may contain non-copyable list types; design proper move
  support in A06 before using it for DirectedGraph's adjacency rows.
- Keep service errors explicit, don't print in domain or DSA code.
