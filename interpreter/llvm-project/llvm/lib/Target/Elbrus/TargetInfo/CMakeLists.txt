add_llvm_component_library(LLVMElbrusInfo
  ElbrusTargetInfo.cpp

  LINK_COMPONENTS
  Support

  ADD_TO_COMPONENT
  Elbrus
  )
