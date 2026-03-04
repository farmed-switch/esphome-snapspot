/*
 * Stub that forces the linker to pull in SnapSpotComponent from the prebuilt .a.
 *
 * The vtable (_ZTVN7esphome8snapspot17SnapSpotComponentE) lives in
 * snapspot_component.cpp.o inside libsnapspot.a. Without an explicit reference
 * the linker treats the .a as optional and omits the vtable → undefined ref.
 *
 * By referencing the vtable symbol here (in a compiled-and-linked stub .o)
 * the linker is forced to resolve it from libsnapspot.a, pulling in the whole
 * translation unit — no --whole-archive needed.
 */

/* The vtable reference must be excluded from the bootloader build.
 * Bootloader has no C++ runtime, so the symbol does not exist there. */
#ifndef BOOTLOADER_BUILD

/* The vtable for esphome::snapspot::SnapSpotComponent (C++ mangled name). */
extern char _ZTVN7esphome8snapspot17SnapSpotComponentE[];

/* Mark as used so the compiler/linker cannot optimise this away. */
__attribute__((used))
void* snapspot_vtable_anchor_ = (void*)_ZTVN7esphome8snapspot17SnapSpotComponentE;

#endif /* BOOTLOADER_BUILD */
