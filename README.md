# PS2-NativeHooks
Native hooking and runtime code injection for PlayStation 2 games

PS2 NativeHooks is a research project and development base for executing
custom native code inside PlayStation 2 games.

The project demonstrates a workflow for compiling C into native Emotion
Engine (R5900) machine code, linking that code at a known location within
a game's address space, and redirecting existing game execution into custom
functions through runtime patches.

This provides an "internal" style development environment similar to
traditional game-hacking workflows on PC, allowing injected code to:

- Hook existing game functions
- Call native engine functions
- Access game structures directly
- Traverse game objects and entities
- Use native rendering systems
- Implement custom runtime behavior

PCSX2 PNACH patches are currently used as the delivery mechanism for the
compiled payload and hooks.

## Current Proof of Concept

SOCOM is currently being used as the primary research target and
proof-of-concept implementation.

The SOCOM example currently demonstrates:

- C compiled to native R5900 code
- Function hooking
- Calling original functions from hooks
- Calling native game functions
- Direct access to reconstructed game structures
- Entity iteration
- Native 2D rendering
- Native 3D rendering
- World-to-screen projection
- Skeleton rendering
- Runtime HUD / C2D research

The underlying NativeHooks technique is not intended to be SOCOM-specific.