# Limit Concurrent curl Handles

**Upstream:** DeterminateSystems/nix-src PR #315
**Upstream version:** Determinate Nix v3.15.2 (Jan 2026)
**Tier:** 2 — performance fix

## Why

When querying binary caches for large closures (>100k paths), Nix enqueues all
downloads at once. This triggers quadratic behavior in curl's multi handle.
Progress bar rendering also becomes a bottleneck with that many in-flight items.

DetSys fixed this by limiting the number of active curl handles to a reasonable
window, with the caveat that `http-connections = 0` (meaning "unlimited") must
remain functional.

## What Changes

- **File transfer**: Limit the number of simultaneously active curl handles.
  Excess requests are queued internally.
- **http-connections = 0**: Treated as "as many as curl allows" rather than
  "enqueue everything at once".
- **Progress bar**: Fixed to handle large numbers of pending downloads without
  excessive CPU usage.

## Capabilities

### Modified Capabilities
- `transfer-performance`: Concurrent downloads bounded to avoid quadratic curl behavior

## Impact

- **Files**: `src/libstore/filetransfer.cc` (curl multi handle management)
- **APIs**: None; `http-connections` semantics refined for edge case
- **Dependencies**: None
- **Testing**: Stress test with a large closure against a local binary cache

## Merge Conflict Risk

Medium. The file transfer code is actively maintained and may have diverged.
The change is conceptually simple (add a queue in front of curl_multi_add_handle)
but the implementation touches the download scheduling loop.
