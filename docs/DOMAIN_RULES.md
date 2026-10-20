# InternPath — frozen minimum business rules (V1 fresh build)

## Product boundary
InternPath is a practical internship platform. DSA implementations are internal.
A university mini-project presentation may separately explain DSA choices.
No DSA lab, algorithm tutorial, lecturer view or benchmark screen inside the site.

## Proficiency and matching
- Levels: student 0..3; requirement 1..3; weight 1..5.
- Per requirement: `w * min(student, required) * (6 / required)`.
- Numerator N is sum of contributions, denominator D is `6*sum(weights)`.
- Eligibility requires **all mandatory** thresholds to be met.
- Explain missing mandatory requirements independently of fit percentage.
- Compare N/D exactly; do not rank using the rounded display percentage.
- Recommendations: eligible first; fit descending; deadline ascending; ID ascending.

## Skill prerequisites
- Edge A->B says A must reach edge's minimum proficiency before advancing B.
- Reject self-edge, duplicate, unknown skill, and cycles.
- DFS checks a proposed cycle, Kahn gives ordered learning stages.
- For learning plans, propagate demanded minimum prerequisites in **reverse**
  topological order, then output tasks in topological order.
- Never fabricate learning hour estimates if not included in curated data.

## Applications
- States: Submitted, Allocated, Waitlisted, Withdrawn, Cancelled.
- Open postings accept only eligible students before their deadline.
- Submit-time fit is frozen; profile edits do not change submitted scores.
- Only one active application per student and internship pair.
- Admin explicitly closes a round. Rank by N DESC, submit sequence ASC, ID ASC.
- First P are allocated; remaining candidates waitlisted in ranked order.
- Waitlisted withdrawals remove a node from the middle using an ID-to-handle hash.
- Allocated withdrawals promote the waiting-list head if available.
- No offer-confirmation or cross-internship exclusivity in first version.
- Undo applies to **profile edits only**, not applications or allocations.

## Persistence
- Append-only primary arrays with stable IDs; derived hash/AVL/list indexes rebuilt.
- Keep data under versioned snapshots, validate on load, handle broken CSV.
- No public deployment or insecure self-built authentication in V1.
