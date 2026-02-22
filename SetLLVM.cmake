function(SetSourceLLVM llvmDir llvmlibs)
    set(LLVM_BUILD_TOOLS OFF)
    set(LLVM_BUILD_DOCS OFF)
    set(LLVM_BUILD_EXAMPLES OFF)
    set(LLVM_INCLUDE_TOOLS OFF)
    set(LLVM_BUILD_UTILS OFF)
    set(LLVM_INCLUDE_UTILS OFF)
    set(LLVM_BUILD_TESTS OFF)
    set(LLVM_INCLUDE_TESTS OFF)
    set(LLVM_INCLUDE_GO_TESTS OFF)
    set(LLVM_BUILD_BENCHMARKS OFF)
    set(LLVM_INCLUDE_BENCHMARKS OFF)
    set(LLVM_ENABLE_BACKTRACES OFF)
    set(LLVM_ENABLE_CRASH_OVERRIDES OFF)
    set(LLVM_ENABLE_UNWIND_TABLES OFF)
    set(LLVM_ENABLE_TERMINFO OFF)
    set(LLVM_ENABLE_LIBEDIT OFF)
    set(LLVM_ENABLE_LIBPFM OFF)
    set(LLVM_ENABLE_PEDANTIC OFF)
    set(LLVM_INCLUDE_DOCS OFF)
    set(LLVM_ENABLE_OCAMLDOC OFF)
    set(LLVM_ENABLE_BINDINGS OFF)
    set(LLVM_VERSION_PRINTER_SHOW_HOST_TARGET_INFO OFF)
    set(LLVM_ENABLE_PER_TARGET_RUNTIME_DIR OFF)

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        set(LLVM_TARGETS_TO_BUILD "X86")
        set(TARGET_COMPONENTS X86CodeGen X86Info X86Desc X86AsmParser X86Disassembler)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64")
        set(LLVM_TARGETS_TO_BUILD "AArch64")
        set(TARGET_COMPONENTS AArch64CodeGen AArch64Info AArch64Desc AArch64AsmParser AArch64Disassembler)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|ARM")
        set(LLVM_TARGETS_TO_BUILD "ARM")
        set(TARGET_COMPONENTS ARMCodeGen ARMInfo ARMDesc ARMAsmParser ARMDisassembler)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "riscv64|riscv")
        set(LLVM_TARGETS_TO_BUILD "RISCV")
        set(TARGET_COMPONENTS RISCVCodeGen RISCVInfo RISCVDesc RISCVAsmParser RISCVDisassembler)
    else()
        message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    add_subdirectory(${llvmDir} ${LLVM_GENERATE_DIR} EXCLUDE_FROM_ALL)
    
    set(LLVM_LINK_COMPONENTS
        Core
        ExecutionEngine
        Object
        OrcJIT
        native
        ${TARGET_COMPONENTS}
    )
    
    llvm_map_components_to_libnames(llvm_libs ${LLVM_LINK_COMPONENTS})
    set(${llvmlibs} ${llvm_libs} PARENT_SCOPE)
endfunction(SetSourceLLVM)