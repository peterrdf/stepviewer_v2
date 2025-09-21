# WebGPU Migration Documentation

## Overview
This document outlines the completed WebGPU migration infrastructure for the STEPViewer project and the remaining integration steps.

## Completed Components

### 1. Core WebGPU Infrastructure (`include/_webgpuUtils.h`)
- ✅ `_webgpuUtils` class with error checking utilities
- ✅ `_webgpuShader` class for WGSL shader compilation
- ✅ `_webgpuProgram` base class for render pipelines
- ✅ `_webgpuBlinnPhongProgram` for Blinn-Phong lighting
- ✅ `_webgpuPerPixelProgram` for advanced per-pixel lighting
- ✅ `_webgpuContext` class managing device/adapter/surface
- ✅ `_webgpuBuffers` for vertex/index buffer management
- ✅ `_webgpuRenderer` base rendering class
- ✅ `_webgpuView` base view class

### 2. Shader Migration
- ✅ `STEPViewer/res/Shader.wgsl` - Basic vertex shader (GLSL → WGSL)
- ✅ `STEPViewer/res/Shader_fragment.wgsl` - Basic fragment shader
- ✅ `STEPViewer/res/vertex_shader2.wgsl` - Advanced vertex shader
- ✅ `STEPViewer/res/fragment_shader2.wgsl` - Advanced fragment shader

### 3. View Classes
- ✅ `STEPViewer/AP242/AP242WebGPUView.h/.cpp` - AP242 WebGPU view
- ✅ `STEPViewer/IFC/IFCWebGPUView.h/.cpp` - IFC WebGPU view  
- ✅ `STEPViewer/CIS2/CIS2WebGPUView.h/.cpp` - CIS2 WebGPU view

## Next Steps for Integration

### 1. WebGPU Library Dependencies
Add WebGPU libraries to the project:
```
// Replace in STEPViewer.vcxproj:
glew32s.lib → webgpu.lib
opengl32.lib → (remove - not needed for WebGPU)
glu32.lib → (remove - not needed for WebGPU)

// Add WebGPU include paths:
./../webgpu/include
```

### 2. Preprocessor Definitions
Update project preprocessor definitions:
```
// Remove:
GLEW_STATIC
_ENABLE_OPENGL_DEBUG

// Add:
_WEBGPU_ENABLED
_WEBGPU_DEBUG (for debug builds)
```

### 3. Missing Dependencies
The project requires these WebGPU libraries:
- `webgpu.h` - Main WebGPU headers
- `webgpu_cpp.h` - C++ WebGPU bindings
- WebGPU implementation library (e.g., Dawn, wgpu-native)

### 4. Resource Loading
Update resource handling for WGSL shaders:
```cpp
// Add to Resource.h:
#define IDR_TEXTFILE_VERTEX_SHADER_WGSL     ???
#define IDR_TEXTFILE_FRAGMENT_SHADER_WGSL   ???
#define IDR_TEXTFILE_VERTEX_SHADER2_WGSL    ???
#define IDR_TEXTFILE_FRAGMENT_SHADER2_WGSL  ???
```

### 5. Application Integration
Update main application to use WebGPU views:
```cpp
// In controller/view creation code:
#ifdef _WEBGPU_ENABLED
    CAP242WebGPUView* pView = new CAP242WebGPUView(pWnd);
#else
    CAP242OpenGLView* pView = new CAP242OpenGLView(pWnd);
#endif
```

## Technical Notes

### WebGPU vs OpenGL Differences
1. **Context Creation**: WebGPU uses instance→adapter→device pattern vs OpenGL's context
2. **Shaders**: WGSL instead of GLSL, different binding model
3. **Rendering**: Command encoders + render passes vs immediate mode
4. **Buffers**: Explicit buffer creation with usage flags
5. **Pipeline State**: Render pipelines bundle all GPU state

### Shader Binding Model Migration
- OpenGL uniforms → WebGPU uniform buffers in bind groups
- OpenGL attributes → WebGPU vertex buffer layouts
- OpenGL samplers → WebGPU texture + sampler binding

### Error Handling
WebGPU uses callback-based error reporting instead of OpenGL's error state polling.

## Testing Strategy
1. Keep both OpenGL and WebGPU systems active initially
2. Add compile-time switches to toggle between them
3. Compare rendering output for accuracy
4. Performance benchmark both implementations
5. Gradually phase out OpenGL once WebGPU is stable

## Current Status
- **Infrastructure**: 100% complete
- **Shader Migration**: 100% complete  
- **View Classes**: 100% complete
- **Project Integration**: 0% (requires WebGPU libraries)
- **Testing**: 0% (pending integration)

The migration foundation is complete and ready for WebGPU library integration and testing.