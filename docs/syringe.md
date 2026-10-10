# Syringe on Windows 98

Ares and Phobos are loaded into the game by Syringe. The current SyringeEx
([PR #26](https://github.com/Phobos-developers/SyringeEx/pull/26)) needs one fix to work on
Windows 98. `CNCNET98.BAT` installs a build with it:
[Syringe for Windows 98](https://github.com/11EJDE11/SyringeEx/releases/tag/win98-rel32-1),
built from the `pr26-rel32-wrap` branch of
[11EJDE11/SyringeEx](https://github.com/11EJDE11/SyringeEx/tree/pr26-rel32-wrap).
The change is also here as `syringeex-pr26-rel32-wrap.patch`.

## The problem

On Windows 9x, `VirtualAllocEx` memory for another process can only come from the shared
arena above 0x80000000 (.NET for Windows 98's DN98FX library provides it there). Syringe's
hook code is then more than 2 GB from the game's code. Zydis checks branch displacements as
64-bit values, so `RebuildInstructions` failed to re-encode relocated relative branches and
copied them verbatim, making them jump to the wrong place. In one run that was 268 hooks for
Ares + Phobos, and the game crashed seconds into a skirmish.

## The fix

A 32-bit rel32 wraps modulo 2^32, so the patch moves the target by 2^32 before encoding. On
Windows NT nothing changes, because there the hook code is allocated near the game.

The patch adds three cases to `tests/RebuildInstructionsTests.cpp`. Before the patch they fail
(102 of 105 checks pass); after it all 105 pass.
