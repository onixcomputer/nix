#!/usr/bin/env bash

source common.sh

wasmDir="$functionalTestsDir/wasm"

# Skip if wasm support is not compiled in
if ! nix eval --extra-experimental-features 'nix-command wasm-builtin' --expr 'true' 2>/dev/null; then
    echo "wasm-builtin not available, skipping"
    exit 0
fi

wasm_eval() {
    nix eval --extra-experimental-features 'nix-command wasm-builtin' --impure "$@"
}

wasm_eval_stderr() {
    nix eval --extra-experimental-features 'nix-command wasm-builtin' --impure "$@" 2>&1
}

# ── Non-WASI: pure function call ──

echo "Testing non-WASI pure function (double)..."
result=$(wasm_eval --expr "builtins.wasm { path = $wasmDir/pure_double.wasm; function = \"double\"; } 21")
[[ "$result" = "42" ]] || { echo "FAIL: expected 42 got $result"; exit 1; }

# ── Non-WASI: warn output ──

echo "Testing non-WASI warn output..."
output=$(wasm_eval_stderr --expr "builtins.wasm { path = $wasmDir/pure_double.wasm; function = \"double\"; } 5")
echo "$output" | grep -q "doubling" || { echo "FAIL: expected 'doubling' warning"; exit 1; }
echo "$output" | grep -q "10" || { echo "FAIL: expected result 10"; exit 1; }

# ── WASI: function call via return_to_nix ──

echo "Testing WASI module (double)..."
result=$(wasm_eval --expr "builtins.wasm { path = $wasmDir/wasi_double.wasm; } 21")
[[ "$result" = "42" ]] || { echo "FAIL: expected 42 got $result"; exit 1; }

# ── WASI: stdout/stderr captured as warnings ──

echo "Testing WASI stdout/stderr capture..."
output=$(wasm_eval_stderr --expr "builtins.wasm { path = $wasmDir/wasi_hello.wasm; } 0")
echo "$output" | grep -q "hello from stdout" || { echo "FAIL: expected stdout capture"; exit 1; }
echo "$output" | grep -q "hello from stderr" || { echo "FAIL: expected stderr capture"; exit 1; }
echo "$output" | grep -q '"done"' || { echo "FAIL: expected result 'done'"; exit 1; }

# ── Error: missing path/wat attribute ──

echo "Testing error on missing path/wat..."
expectStderr 1 wasm_eval --expr "builtins.wasm { function = \"f\"; } 1" \
    | grep -q "missing required 'path' or 'wat'" || { echo "FAIL: expected missing path/wat error"; exit 1; }

# ── Error: unknown attribute (checked before path extraction) ──

echo "Testing error on unknown attribute..."
expectStderr 1 wasm_eval --expr "builtins.wasm { path = $wasmDir/pure_double.wasm; function = \"double\"; bogus = 1; } 1" \
    | grep -q "unknown attribute 'bogus'" || { echo "FAIL: expected unknown attr error"; exit 1; }

# ── Error: function attr on WASI module ──

echo "Testing error on function attr with WASI module..."
expectStderr 1 wasm_eval --expr "builtins.wasm { path = $wasmDir/wasi_double.wasm; function = \"f\"; } 1" \
    | grep -q "not allowed for WASI" || { echo "FAIL: expected WASI function error"; exit 1; }

# ── Error: missing function attr on non-WASI module ──

echo "Testing error on missing function attr with non-WASI module..."
expectStderr 1 wasm_eval --expr "builtins.wasm { path = $wasmDir/pure_double.wasm; } 1" \
    | grep -q "missing required 'function'" || { echo "FAIL: expected missing function error"; exit 1; }

# ── Error: WASI module without return_to_nix ──

echo "Testing error on WASI module without return_to_nix..."
expectStderr 1 wasm_eval --expr "builtins.wasm { path = $wasmDir/wasi_no_return.wasm; } 1" \
    | grep -q "finished without returning a value" || { echo "FAIL: expected no-return error"; exit 1; }

# ── Error propagation: tryEval catches non-WASI panic ──

echo "Testing tryEval catches non-WASI WASM panic..."
result=$(wasm_eval --expr '
  builtins.tryEval (builtins.wasm {
    path = '"$wasmDir"'/pure_panic.wasm;
    function = "will_panic";
  } 1)
')
echo "$result" | grep -q 'success = false' || { echo "FAIL: tryEval should catch WASM panic, got: $result"; exit 1; }

# ── Error propagation: tryEval catches WASI panic ──

echo "Testing tryEval catches WASI WASM panic..."
result=$(wasm_eval --expr '
  builtins.tryEval (builtins.wasm {
    path = '"$wasmDir"'/wasi_panic.wasm;
  } 1)
')
echo "$result" | grep -q 'success = false' || { echo "FAIL: tryEval should catch WASI panic, got: $result"; exit 1; }

# ── Error propagation: panic error message preserved ──

echo "Testing WASM panic error message is preserved..."
output=$(wasm_eval_stderr --expr 'builtins.wasm { path = '"$wasmDir"'/pure_panic.wasm; function = "will_panic"; } 1' || true)
echo "$output" | grep -q "intentional panic for testing" || { echo "FAIL: panic message not preserved in output"; exit 1; }

# ── Error propagation: normal calls still work after panic test ──

echo "Testing normal WASM calls still work..."
result=$(wasm_eval --expr "builtins.wasm { path = $wasmDir/pure_double.wasm; function = \"double\"; } 21")
[[ "$result" = "42" ]] || { echo "FAIL: expected 42 got $result after panic tests"; exit 1; }

# ── Error: feature gate ──

echo "Testing feature gate..."
expectStderr 1 nix eval --extra-experimental-features 'nix-command' \
    --impure --expr "builtins.wasm { path = $wasmDir/pure_double.wasm; function = \"double\"; } 1" \
    | grep -qi "wasm" || { echo "FAIL: expected feature gate error"; exit 1; }

# ── String context: has_context on plain string ──

echo "Testing has_context on plain string..."
result=$(wasm_eval --expr '
  let r = builtins.wasm {
    path = '"$wasmDir"'/string_context.wasm;
    function = "inspect_context";
  } "hello";
  in r.has_ctx
')
[[ "$result" = "0" ]] || { echo "FAIL: expected has_ctx=0, got $result"; exit 1; }

# ── String context: has_context on string with context ──

echo "Testing has_context on string with context..."
result=$(wasm_eval --expr '
  let
    drv = derivation { name = "ctx-test"; system = builtins.currentSystem; builder = "/bin/sh"; };
    s = "${drv}";
    r = builtins.wasm {
      path = '"$wasmDir"'/string_context.wasm;
      function = "inspect_context";
    } s;
  in r.has_ctx
')
[[ "$result" = "1" ]] || { echo "FAIL: expected has_ctx=1, got $result"; exit 1; }

# ── String context: context count ──

echo "Testing context count..."
result=$(wasm_eval --expr '
  let
    drv = derivation { name = "ctx-test"; system = builtins.currentSystem; builder = "/bin/sh"; };
    s = "${drv}";
    r = builtins.wasm {
      path = '"$wasmDir"'/string_context.wasm;
      function = "inspect_context";
    } s;
  in r.ctx_count
')
[[ "$result" = "1" ]] || { echo "FAIL: expected ctx_count=1, got $result"; exit 1; }

# ── String context: round-trip preserves context ──

echo "Testing string context round-trip..."
result=$(wasm_eval --expr '
  let
    drv = derivation { name = "ctx-rt"; system = builtins.currentSystem; builder = "/bin/sh"; };
    s = "${drv}";
    rt = builtins.wasm {
      path = '"$wasmDir"'/string_context.wasm;
      function = "passthrough_string";
    } s;
  in builtins.getContext rt == builtins.getContext s
')
[[ "$result" = "true" ]] || { echo "FAIL: round-trip context mismatch"; exit 1; }

# ── String context: plain string round-trip has no context ──

echo "Testing plain string round-trip has no context..."
result=$(wasm_eval --expr '
  let
    rt = builtins.wasm {
      path = '"$wasmDir"'/string_context.wasm;
      function = "passthrough_string";
    } "plain";
  in builtins.getContext rt == {}
')
[[ "$result" = "true" ]] || { echo "FAIL: plain string should have empty context"; exit 1; }

# ── String context: make_string (no context) still works ──

echo "Testing make_string backward compat..."
result=$(wasm_eval --expr '
  let
    r = builtins.wasm {
      path = '"$wasmDir"'/string_context.wasm;
      function = "inspect_context";
    } "test";
  in r.value
')
[[ "$result" = '"test"' ]] || { echo "FAIL: expected '\"test\"', got $result"; exit 1; }

# ── WAT: inline WebAssembly Text format ──

echo "Testing inline WAT (fibonacci)..."
result=$(wasm_eval --json --expr "
  builtins.wasm {
    wat = builtins.readFile $wasmDir/fib.wat;
    function = \"fib\";
  } 40
")
[[ "$result" = "165580141" ]] || { echo "FAIL: expected fib(40)=165580141, got $result"; exit 1; }

# ── WAT: precompiled .wasm equivalent produces same result ──

echo "Testing precompiled fib.wasm matches WAT result..."
result=$(wasm_eval --json --expr "builtins.wasm { path = $wasmDir/fib.wasm; function = \"fib\"; } 40")
[[ "$result" = "165580141" ]] || { echo "FAIL: expected fib(40)=165580141 from .wasm, got $result"; exit 1; }

# ── WAT: mutual exclusivity with path ──

echo "Testing wat + path mutual exclusivity..."
expectStderr 1 wasm_eval --expr "
  builtins.wasm {
    path = $wasmDir/fib.wasm;
    wat = \"(module)\";
    function = \"fib\";
  } 1
" | grep -q "mutually exclusive" || { echo "FAIL: expected mutual exclusivity error"; exit 1; }

# ── Crash fix: bad WASM path loaded twice doesn't crash ──

echo "Testing crash fix: bad WASM path loaded twice..."
mkdir -p "$TEST_ROOT"
echo "not a wasm file" > "$TEST_ROOT/bad.wasm"
result=$(wasm_eval --expr '
  let
    r1 = builtins.tryEval (builtins.wasm { path = '"$TEST_ROOT"'/bad.wasm; function = "f"; } 1);
    r2 = builtins.tryEval (builtins.wasm { path = '"$TEST_ROOT"'/bad.wasm; function = "f"; } 1);
  in { inherit (r1) success; second = r2.success; }
')
echo "$result" | grep -q 'success = false' || { echo "FAIL: first bad load should fail, got: $result"; exit 1; }
echo "$result" | grep -q 'second = false' || { echo "FAIL: second bad load should fail (not crash), got: $result"; exit 1; }

echo "All wasm tests passed."
