# If we don't need RTTI or EH, there's no reason to export anything
# from the SRPass plugin.
if( NOT LLVM_REQUIRES_RTTI )
  if( NOT LLVM_REQUIRES_EH )
    set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/SRPass.exports)
  endif()
endif()

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS Core Support)
endif()

add_llvm_library( SRPass MODULE BUILDTREE_ONLY
  SCIPass.cpp
  DynamicCallCounter.cpp
  SRAnalysisPass.cpp
  SCClean.cpp
  utils.cpp
  CostModel.cpp
  PLUGIN_TOOL
  opt
  )

add_dependencies(SRPass LLVMInstrumentation)

# Create symlinks for SanRazor-clang.rb
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bin/SanRazor-clang
  COMMAND cd ${CMAKE_BINARY_DIR}/bin && ln -s ${CMAKE_CURRENT_SOURCE_DIR}/SanRazor-clang.rb SanRazor-clang
  COMMENT Creating symlink for SanRazor-clang
  VERBATIM)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bin/SanRazor-clang++
  COMMAND cd ${CMAKE_BINARY_DIR}/bin && ln -s ${CMAKE_CURRENT_SOURCE_DIR}/SanRazor-clang.rb SanRazor-clang++
  COMMENT Creating symlink for SanRazor-clang++
  VERBATIM)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bin/SanRazor-ranlib
  COMMAND cd ${CMAKE_BINARY_DIR}/bin && ln -s ${CMAKE_CURRENT_SOURCE_DIR}/SanRazor-clang.rb SanRazor-ranlib
  COMMENT Creating symlink for SanRazor-ranlib
  VERBATIM)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bin/SanRazor-ar
  COMMAND cd ${CMAKE_BINARY_DIR}/bin && ln -s ${CMAKE_CURRENT_SOURCE_DIR}/SanRazor-clang.rb SanRazor-ar
  COMMENT Creating symlink for SanRazor-ar
  VERBATIM)
add_custom_target(SanRazor_clang_symlinks
  ALL
  DEPENDS ${CMAKE_BINARY_DIR}/bin/SanRazor-clang
          ${CMAKE_BINARY_DIR}/bin/SanRazor-clang++
          ${CMAKE_BINARY_DIR}/bin/SanRazor-ranlib
          ${CMAKE_BINARY_DIR}/bin/SanRazor-ar)