# HTTP adapter — Milestone 1

No unsafe homemade socket server in M0. Antigravity should vendor pinned,
license-compliant copies of `cpp-httplib` and `nlohmann/json` in `third_party/`
and create `src/api/HttpServer.h/.cpp`, `Routes.h/.cpp`, DTO validation,
and `src/server/main.cpp`. **Only** the transport layer may use those libraries.

First acceptance test:
- bind to `127.0.0.1` (not `0.0.0.0`),
- serve static `web/index.html` at `/`,
- `GET /api/health` responds with `{"ok":true,"name":"InternPath"}`,
- parse and validate one JSON request/response round trip,
- build on teammates' computers.

Do not claim authentication. Demo profile selection is not security.
