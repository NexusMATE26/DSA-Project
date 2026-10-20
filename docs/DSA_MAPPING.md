# Justification — nine mandatory DSA topics (not product website content)

| Required topic | Real operation | Selection rationale | Complexity to demonstrate |
| --- | --- | --- | --- |
| Dynamic array | Own records, levels, sortable results | Fast indexed lookup, growing capacity | Access O(1); append amortized O(1) |
| Singly/doubly lists | Hash chains; ranked waitlist with node handles | Chain insertion and O(1) unlink by handle | O(1) unlink once located |
| Stack | Profile-edit undo | Reverses last edit first | Push/pop O(1) |
| Queue | BFS and Kahn frontier | Processes graph layers in FIFO order | Enqueue/dequeue O(1) |
| Hash table | ID lookups, duplicates, waitlist handles | Exact-key access, no ordering required | O(1) expected, O(n) worst |
| AVL tree | Dynamic open-deadline index, closing-soon ranges | Ordered queries plus O(log n) insert/erase | Range O(log n + k) |
| Directed graph | Skill prerequisites, cycle checking, learning plans | DAG captures dependencies | O(V+E) traversals |
| Sorting | Applicant stable merge sort, ordered-index insertion | Predictable ranking, efficient near-sorted updates | O(n log n) merge; O(n^2) insertion worst |
| Searching | Linear keyword filters; binary lower bound; hash exact; AVL deadline | Choose by data organization | O(n), O(log n), O(1) expected |

**Viva trade-offs:** For small data a sorted array can beat AVL on simplicity;
a heap is useful if ranking changes continually, but our waiting order is fixed
at close. Never call the waitlist a plain FIFO queue in its ranking phase.
