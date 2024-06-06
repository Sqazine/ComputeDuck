function(SetLLVM llvmDir llvmlibs)
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

    add_subdirectory(${llvmDir})
    
    set(LLVM_LINK_COMPONENTS
        Analysis
        Core
        ExecutionEngine
        InstCombine
        Object
        OrcJIT
        RuntimeDyld
        ScalarOpts
        Support
        native
        X86CodeGen
        X86AsmParser
        X86Disassembler
        X86Desc
        X86Info
        X86TargetMCA
        AArch64CodeGen
        AArch64AsmParser
        AArch64Disassembler
        AArch64Desc
        AArch64Info
        AArch64Utils
    )
    
    llvm_map_components_to_libnames(llvm_libs ${LLVM_LINK_COMPONENTS})
    
    set(${llvmlibs} ${llvm_libs} PARENT_SCOPE)
endfunction(SetLLVM)
