# Tasks: Concurrent Substitution Dedup

## Phase 1: Port

- [ ] Fetch DetSys PR #398 diff
- [ ] Study how substitution goals are currently created and tracked
- [ ] Add a map of in-flight substitutions keyed by store path
- [ ] When a substitution is requested for a path already in flight, share the existing goal
- [ ] Handle cleanup when the original substitution completes or fails

## Phase 2: Verify

- [ ] Run `meson test`
- [ ] Test with `nix build` of multiple derivations that share a common dependency
- [ ] Verify only one download per unique path in daemon logs
- [ ] Commit on a new branch `subst-dedup-2.33.3`
