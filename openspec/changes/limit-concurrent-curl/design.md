# Limit Concurrent curl Handles — Design

## Problem

When querying binary caches for large closures (>100k paths), Nix creates a
`TransferItem` per `.narinfo` and immediately registers each as a curl handle.
curl's internal bookkeeping scales poorly with many inactive handles — even
though only `http-connections` (default 25) actually transfer data at once,
`curl_multi_perform()` iterates over all registered handles on each call.
Result: 100% CPU on the download thread, active downloads stall and time out.

## Solution

### 1. Cap active curl handles

Add a `maxActiveHandles` limit computed as `httpConnections * 5`. The worker
loop checks `items.size() + incoming.size() >= maxActiveHandles` before
dequeuing from `state->incoming`. Excess items stay in the priority queue and
are retried after a 100ms wakeup.

### 2. Handle http-connections = 0

`http-connections = 0` means "unlimited" (passed to curl as-is). For our
internal cap, fall back to `std::max(1U, std::thread::hardware_concurrency())`
so the multiplier produces a sane number.

### 3. Lazy Activity creation

Defer the `Activity` (progress bar entry) from `TransferItem` construction to
the point where curl actually starts the download. Uses
`CURLOPT_RESOLVER_START_FUNCTION` (libcurl >= 7.59.0) as the trigger. This
avoids showing hundreds of "downloading" entries for items that are just
queued. Also resets `startTime` so reported durations reflect actual transfer
time.

## Files Changed

- `src/libstore/filetransfer.cc` — all changes in this single file
