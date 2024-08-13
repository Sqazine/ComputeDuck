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
        Core
        ExecutionEngine
        Object
        OrcJIT
        native
        X86CodeGen
        X86AsmParser
        X86Disassembler
        X86Desc
        X86Info
        X86TargetMCA
        ARMCodeGen
        ARMAsmParser
        ARMDisassembler
        ARMDesc
        ARMInfo
        AArch64CodeGen
        AArch64AsmParser
        AArch64Disassembler
        AArch64Desc
        AArch64Info
    )
    
    llvm_map_components_to_libnames(llvm_libs ${LLVM_LINK_COMPONENTS})
    
    set(${llvmlibs} ${llvm_libs} PARENT_SCOPE)


    if(MSVC)
        set_property(TARGET ${llvm_libs} PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET AArch64 PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET all-targets PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET AMDGPU PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET ARM PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET AVR PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET BPF PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET distribution PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET Engine PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET Hexagon PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET install-distribution PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET install-distribution-stripped PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET Lanai PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET Mips PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET MSP430 PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET Native PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET NativeCodeGen PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET NVPTX PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET PowerPC PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET RISCV PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET Sparc PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET SystemZ PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET VE PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET WebAssembly PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET X86 PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET XCore PROPERTY FOLDER 3rd/LLVM)
        set_property(TARGET LLVMVisualizers PROPERTY FOLDER 3rd/LLVM/Utils)
        set_property(TARGET llvm_vcsrevision_h PROPERTY FOLDER 3rd/LLVM/Misc)
        set_property(TARGET llvm-headers PROPERTY FOLDER 3rd/LLVM/Misc)
        set_property(TARGET llvm-libraries PROPERTY FOLDER 3rd/LLVM/Misc)
        set_property(TARGET ocaml_all PROPERTY FOLDER 3rd/LLVM/Misc)
        set_property(TARGET ocaml_make_directory PROPERTY FOLDER 3rd/LLVM/Misc)
        set_property(TARGET srpm PROPERTY FOLDER 3rd/LLVM/Misc)
        set_property(TARGET LLVMHello PROPERTY FOLDER 3rd/LLVM/Loadable modules)
        set_property(TARGET AArch64CommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET acc_gen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET AMDGPUCommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET ARMCommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET AVRCommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET BPFCommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET DllOptionsTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET HexagonCommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET InstCombineTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET intrinsics_gen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET LanaiCommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET LibOptionsTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET llvm-tblgen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET MipsCommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET MSP430CommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET NVPTXCommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET omp_gen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET PowerPCCommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET RISCVCommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET SparcCommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET SystemZCommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET VECommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET WebAssemblyCommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET X86CommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)
        set_property(TARGET XCoreCommonTableGen PROPERTY FOLDER 3rd/LLVM/Tablegenning)

        set_property(TARGET BrainF PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET BuildingAJIT-Ch1 PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET BuildingAJIT-Ch2 PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET BuildingAJIT-Ch3 PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET BuildingAJIT-Ch4 PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET ExampleIRTransforms PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET Fibonacci PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET HowToUseJIT PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET HowToUseLLJIT PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET Kaleidoscope PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET Kaleidoscope-Ch2 PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET Kaleidoscope-Ch3 PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET Kaleidoscope-Ch4 PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET Kaleidoscope-Ch5 PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET Kaleidoscope-Ch6 PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET Kaleidoscope-Ch7 PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET Kaleidoscope-Ch8 PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET Kaleidoscope-Ch9 PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET LLJITDumpObjects PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET LLJITWithCustomObjectLinkingLayer PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET LLJITWithExecutorProcessControl PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET LLJITWithGDBRegistrationListener PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET LLJITWithInitializers PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET LLJITWithLazyReexports PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET LLJITWithObjectCache PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET LLJITWithObjectLinkingLayerPlugin PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET LLJITWithOptimizingIRTransform PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET LLJITWithThinLTOSummaries PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET ModuleMaker PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET OrcV2CBindingsAddObjectFile PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET OrcV2CBindingsBasicUsage PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET OrcV2CBindingsDumpObjects PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET OrcV2CBindingsIRTransforms PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET OrcV2CBindingsLazy PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET OrcV2CBindingsReflectProcessSymbols PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET OrcV2CBindingsRemovableCode PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET OrcV2CBindingsVeryLazy PROPERTY FOLDER 3rd/LLVM/Examples)
        set_property(TARGET SpeculativeJIT PROPERTY FOLDER 3rd/LLVM/Examples)

        set_property(TARGET LLVMAArch64AsmParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAArch64CodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAArch64Desc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAArch64Disassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAArch64Info PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAArch64Utils PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAggressiveInstCombine PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAMDGPUAsmParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAMDGPUCodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAMDGPUDesc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAMDGPUDisassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAMDGPUInfo PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAMDGPUUtils PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAMDGPUTargetMCA PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMARMAsmParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMARMCodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMARMDesc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMARMDisassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMARMInfo PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMARMUtils PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAsmParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAsmPrinter PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAVRAsmParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAVRCodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAVRDesc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAVRDisassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMAVRInfo PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMBinaryFormat PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMBitReader PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMBitstreamReader PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMBitWriter PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMBPFAsmParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMBPFCodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMBPFDesc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMBPFDisassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMBPFInfo PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMCFGuard PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMCodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMCoroutines PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMCoverage PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMDebugInfoCodeView PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMDebuginfod PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMDebugInfoDWARF PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMDebugInfoGSYM PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMDebugInfoMSF PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMDebugInfoPDB PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMDemangle PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMDlltoolDriver PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMDWARFLinker PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMDWP PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMExtensions PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMFileCheck PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMFrontendOpenMP PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMFuzzMutate PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMGlobalISel PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMFrontendOpenACC PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMHexagonAsmParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMHexagonCodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMHexagonDesc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMHexagonDisassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMHexagonInfo PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMInstrumentation PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMInterfaceStub PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMInterpreter PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMipo PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMIRReader PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMJITLink PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMLanaiAsmParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMLanaiCodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMLanaiDesc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMLanaiDisassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMLanaiInfo PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMLibDriver PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMLineEditor PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMLinker PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMLTO PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMMC PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMMCA PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMMCDisassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMMCJIT PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMMCParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMMipsCodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMMipsDesc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMMipsDisassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMMipsInfo PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMMIRParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMMipsAsmParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMMSP430AsmParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMMSP430CodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMMSP430Desc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMMSP430Disassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMMSP430Info PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMNVPTXCodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMNVPTXDesc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMNVPTXInfo PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMObjCARCOpts PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMObjectYAML PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMOption PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMOrcShared PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMOrcTargetProcess PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMPasses PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMPowerPCAsmParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMPowerPCCodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMPowerPCDesc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMPowerPCDisassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMPowerPCInfo PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMProfileData PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMRemarks PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMRISCVAsmParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMRISCVCodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMRISCVDesc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMRISCVDisassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMRISCVInfo PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMSelectionDAG PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMSparcAsmParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMSparcCodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMSparcDesc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMSparcDisassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMSparcInfo PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMSymbolize PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMSystemZAsmParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMSystemZCodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMSystemZDesc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMSystemZDisassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMSystemZInfo PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMTableGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMTableGenGlobalISel PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMTarget PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMTextAPI PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMTransformUtils PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMVEAsmParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMVECodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMVectorize PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMVEDesc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMVEDisassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMVEInfo PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMWebAssemblyAsmParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMWebAssemblyCodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMWebAssemblyDesc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMWebAssemblyDisassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMWebAssemblyInfo PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMWebAssemblyUtils PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMWindowsManifest PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMX86AsmParser PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMX86CodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMX86Desc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMX86Disassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMX86Info PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMX86TargetMCA PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMXCoreCodeGen PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMXCoreDesc PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMXCoreDisassembler PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMXCoreInfo PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET LLVMXRay PROPERTY FOLDER 3rd/LLVM/Libraries)
        set_property(TARGET obj.llvm-tblgen PROPERTY FOLDER 3rd/LLVM/Object Libraries)
    endif()

endfunction(SetLLVM)
