# Rust and Zig code inside a bare firmware, linked with the C startup/chip layer through the C ABI (FFI).
# The compilers are not copied here: they are reached in their own IDs (Box layout, or the variables):
#   Rust: $BOX_EMBEDDED_RUST (32_17_embedded_rust, launcher rust-box, pinned rustc + bare-metal targets)
#   Zig:  $BOX_MICROZIG      (32_24_microzig, launcher microzig-box, pinned Zig)
#   bare_rust_staticlib(<tgt> <file.rs>)   rustc --crate-type staticlib for the chip's target triple, linked into <tgt>
#   bare_zig_object(<tgt> <file.zig>)      zig build-obj for the chip's CPU, linked into <tgt>
# A project that needs one of them ships needs.sh, which exits 3 (skip) when the ID is missing (check.sh honours it).
include_guard(GLOBAL)
get_filename_component(_BOX_EMBEDDED "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)   # .../32_embedded
if(DEFINED ENV{BOX_EMBEDDED_RUST})
  set(RUST_BOX "$ENV{BOX_EMBEDDED_RUST}/rust-box")
else()
  set(RUST_BOX "${_BOX_EMBEDDED}/32_17_embedded_rust/rust-box")
endif()
if(DEFINED ENV{BOX_MICROZIG})
  set(ZIG_BOX "$ENV{BOX_MICROZIG}/microzig-box")
else()
  set(ZIG_BOX "${_BOX_EMBEDDED}/32_24_microzig/microzig-box")
endif()

function(_bare_lang_triples)
  if(CHIP STREQUAL "mps2-an385" OR CHIP STREQUAL "stm32f103")
    set(rust thumbv7m-none-eabi)
    set(zig thumb-freestanding-eabi)
    set(cpu cortex_m3)
  elseif(CHIP STREQUAL "stm32f411" OR CHIP STREQUAL "nrf52840")
    set(rust thumbv7em-none-eabihf)
    set(zig thumb-freestanding-eabihf)
    set(cpu cortex_m4)
  elseif(CHIP STREQUAL "stm32h743")
    set(rust thumbv7em-none-eabihf)
    set(zig thumb-freestanding-eabihf)
    set(cpu cortex_m7)
  elseif(CHIP STREQUAL "virt-rv32")
    set(rust riscv32imac-unknown-none-elf)
    set(zig riscv32-freestanding-none)
    set(cpu generic_rv32+m+a+c)
  else()
    message(FATAL_ERROR "no Rust/Zig mapping for CHIP ${CHIP}")
  endif()
  set(RUST_TRIPLE ${rust} PARENT_SCOPE)
  set(ZIG_TRIPLE ${zig} PARENT_SCOPE)
  set(ZIG_CPU ${cpu} PARENT_SCOPE)
endfunction()

function(bare_rust_staticlib tgt src)
  if(NOT EXISTS ${RUST_BOX})
    message(FATAL_ERROR "SKIP: Rust needs 32_17_embedded_rust (rust-box not found at ${RUST_BOX}; set BOX_EMBEDDED_RUST)")
  endif()
  _bare_lang_triples()
  get_filename_component(abs ${src} ABSOLUTE)
  set(lib ${CMAKE_CURRENT_BINARY_DIR}/lib${tgt}_rust.a)
  add_custom_command(OUTPUT ${lib}
    COMMAND ${CMAKE_COMMAND} -E env RUSTUP_TOOLCHAIN=1.99.1 ${RUST_BOX} run rustc --edition 2021 --crate-type staticlib
      --target ${RUST_TRIPLE} -C opt-level=s -C panic=abort -C debuginfo=2 ${abs} -o ${lib}
    DEPENDS ${abs} COMMENT "rustc (32_17_embedded_rust) ${RUST_TRIPLE}: ${src}" VERBATIM)
  add_custom_target(${tgt}_rust DEPENDS ${lib})
  add_dependencies(${tgt} ${tgt}_rust)
  target_link_libraries(${tgt} PRIVATE ${lib})
endfunction()

function(bare_zig_object tgt src)
  if(NOT EXISTS ${ZIG_BOX})
    message(FATAL_ERROR "SKIP: Zig needs 32_24_microzig (microzig-box not found at ${ZIG_BOX}; set BOX_MICROZIG)")
  endif()
  _bare_lang_triples()
  get_filename_component(abs ${src} ABSOLUTE)
  get_filename_component(stem ${src} NAME_WE)
  set(obj ${CMAKE_CURRENT_BINARY_DIR}/${tgt}_${stem}_zig.o)
  add_custom_command(OUTPUT ${obj}
    COMMAND ${ZIG_BOX} run zig build-obj -target ${ZIG_TRIPLE} -mcpu=${ZIG_CPU} -O ReleaseSmall -fno-stack-check -fno-unwind-tables
      --cache-dir ${CMAKE_CURRENT_BINARY_DIR}/zig-cache --global-cache-dir ${CMAKE_CURRENT_BINARY_DIR}/zig-cache
      -femit-bin=${obj} ${abs}
    DEPENDS ${abs} COMMENT "zig (32_24_microzig) ${ZIG_TRIPLE}: ${src}" VERBATIM)
  add_custom_target(${tgt}_${stem}_zig DEPENDS ${obj})
  add_dependencies(${tgt} ${tgt}_${stem}_zig)
  target_link_libraries(${tgt} PRIVATE ${obj})
endfunction()
