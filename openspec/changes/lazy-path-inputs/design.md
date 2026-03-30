# Design: Lazy Path Inputs

## Context

Path inputs eagerly copy the entire directory to the store in
`getAccessor()`, even though evaluation typically only reads `flake.nix`.

## Decisions

### 1. Return PosixSourceAccessor for non-store paths

**Choice:** When the path is not already a valid store path, return a
`PosixSourceAccessor` rooted at the path instead of copying to the store.
**Rationale:** The evaluator can read `flake.nix` directly from the
filesystem. The store copy is deferred to `mountInput()` → `fetchToStore()`.

### 2. Compute lastModified by stat-walking

**Choice:** Walk the directory tree with `lstat()` to find the max mtime.
**Rationale:** Cheaper than `dumpPathAndGetMtime` (which also reads file
contents for NAR serialization). Only stats files, doesn't read contents.

### 3. NAR hash fingerprint for cache correctness

**Choice:** Compute the NAR hash via `accessor->hashPath()` for the
fingerprint.
**Rationale:** `fetchToStore()` needs a content-based fingerprint for its
cache. Path+mtime fingerprints broke lock file verification because the
lock file stores the narHash, not mtime.

### 4. Re-mount store accessor in mountInput for lazy inputs

**Choice:** After `fetchToStore()` copies the tree, mount a store accessor
(not the original PosixAccessor) into the storeFS. Tagged with
`lazyPathInput` flag to avoid affecting other input types.
**Rationale:** The PosixAccessor doesn't handle missing files (like
`flake.lock`) the same way the storeFS expects. Mounting the store
accessor preserves correct behavior for lock file resolution.

### 5. Preserve original accessor fingerprint

**Choice:** Copy the fingerprint from the original accessor to the
re-mounted store accessor.
**Rationale:** Without the fingerprint, `fetchToStore()` treats subsequent
accesses as uncacheable, causing test failures.
