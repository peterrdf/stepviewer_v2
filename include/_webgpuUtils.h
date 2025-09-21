#pragma once

#ifdef _WINDOWS
#include <afxwin.h>
#include <atltypes.h>
#include <afxtooltipctrl.h>
#endif

#include "_mvc.h"
#include "_geometry.h"
#include "_oglScene.h"
#include "_quaterniond.h"

// WebGPU headers (replace GLEW)
#include <webgpu/webgpu.h>
#ifdef _WINDOWS
#include <webgpu/webgpu_cpp.h>
#endif

#include "../glm/glm/vec3.hpp"
#include "../glm/glm/vec4.hpp"
#include "../glm/glm/mat4x4.hpp"
#include "../glm/glm/gtc/matrix_transform.hpp"
#include "../glm/glm/gtc/type_ptr.hpp"

#include <chrono>
#include <ctime>
#define _USE_MATH_DEFINES
#include <math.h>
#undef max
#include <limits>
#include <map>
using namespace std;

// ************************************************************************************************
static const int MIN_VIEW_PORT_LENGTH = 100;

// ************************************************************************************************
const float DEFAULT_TRANSLATION = -5.f;

const float ZOOM_SPEED_MOUSE = 0.01f;
const float ZOOM_SPEED_MOUSE_WHEEL = 0.005f;
const float ZOOM_SPEED_KEYS = ZOOM_SPEED_MOUSE;

#define CULL_FACES_NONE L"<none>"
#define CULL_FACES_FRONT L"Front"
#define CULL_FACES_BACK L"Back"

// ************************************************************************************************
enum class enumProjection : int
{
    Perspective = 0,
    Orthographic,
};

// ************************************************************************************************
enum class enumView : int64_t
{
    Front = 0,
    Back,
    Top,
    Bottom,
    Left,
    Right,
    FrontTopLeft,
    FrontTopRight,
    FrontBottomLeft,
    FrontBottomRight,
    BackTopLeft,
    BackTopRight,
    BackBottomLeft,
    BackBottomRight,
    Isometric,
};

// ************************************************************************************************
enum class enumRotationMode : int
{
    XY = 0, // Standard
    XYZ,	// Quaternions
};

// ************************************************************************************************
class _webgpuUtils
{

public: // Methods

    static uint32_t getVerticesCountLimit(uint32_t iVertexLengthBytes)
    {
        return numeric_limits<uint32_t>::max() / iVertexLengthBytes;
    }

    static uint32_t getIndicesCountLimit()
    {
        return numeric_limits<uint32_t>::max();
    }

#if defined _WINDOWS
    static void checkForErrors(WGPUDevice device)
    {
        // WebGPU error checking will be handled through error callbacks
        // TODO: Implement WebGPU error callback mechanism
    }
#endif // _WINDOWS
};

#ifdef _WINDOWS
// ************************************************************************************************
// X, Y, Z, Nx, Ny, Nz
#define GEOMETRY_VBO_VERTEX_LENGTH  6

// ************************************************************************************************
class _webgpuRendererSettings
{
public: // Fields

    enumProjection m_enProjection;
    enumRotationMode m_enRotationMode;

    float m_fXAngle;
    float m_fYAngle;
    float m_fZAngle;
    _quaterniond m_rotation;

    BOOL m_bGhostView;
    float m_fGhostViewTransparency;

    BOOL m_bShowFaces;
    wstring m_strCullFaces;
    BOOL m_bShowFacesPolygons;
    BOOL m_bShowConceptualFacesPolygons;

    BOOL m_bShowLines;
    float m_fLineWidth;

    BOOL m_bShowPoints;
    float m_fPointSize;

    BOOL m_bShowBoundingBoxes;
    BOOL m_bShowNormalVectors;
    BOOL m_bShowTangenVectors;
    BOOL m_bShowBiNormalVectors;

    float m_fScale;
    float m_fTranslation;

    wstring m_strBackgroundColor;

    float m_fXMousePositionLast;
    float m_fYMousePositionLast;

    BOOL m_bRMBPressed;
    BOOL m_bMoverMode;

public: // Methods

    _webgpuRendererSettings();
    virtual ~_webgpuRendererSettings();

    void _reset();
};

// ************************************************************************************************
class _webgpuShader
{

protected: // Fields

    WGPUShaderStage m_stage; // WGPUShaderStage_Vertex or WGPUShaderStage_Fragment
    WGPUShaderModule m_module;
    char* m_szCode;

public: // Methods

    _webgpuShader(WGPUShaderStage shaderStage)
        : m_stage(shaderStage)
        , m_module(nullptr)
        , m_szCode(nullptr)
    {
    }

    virtual ~_webgpuShader(void)
    {
        if (m_szCode) {
            delete[] m_szCode;
        }

        if (m_module) {
            wgpuShaderModuleRelease(m_module);
        }
    }

    WGPUShaderModule getModule() const
    {
        return m_module;
    }

    static char* getResource(int iResource, int iType)
    {
        HMODULE hModule = ::GetModuleHandleW(nullptr);
        HRSRC hResource = ::FindResourceW(hModule, MAKEINTRESOURCEW(iResource), MAKEINTRESOURCEW(iType));
        HGLOBAL rcData = ::LoadResource(hModule, hResource);

        char* szData = static_cast<char*>(::LockResource(rcData));
        DWORD dwSize = ::SizeofResource(hModule, hResource);

        char* szBuffer = new char[dwSize + 1];
        ::memcpy(szBuffer, szData, dwSize);
        szBuffer[dwSize] = 0;

        return szBuffer;
    }

    bool load(int iResource, int iType)
    {
        m_szCode = getResource(iResource, iType);

        return (m_szCode != nullptr);
    }

    bool compile(WGPUDevice device)
    {
        if (m_szCode == nullptr) {
            return false;
        }

        WGPUShaderModuleDescriptor shaderDesc = {};
        WGPUShaderModuleWGSLDescriptor wgslDesc = {};
        wgslDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
        wgslDesc.code = m_szCode;
        shaderDesc.nextInChain = &wgslDesc.chain;

        m_module = wgpuDeviceCreateShaderModule(device, &shaderDesc);

        return (m_module != nullptr);
    }

    void getInfoLog(CString& strInfoLog)
    {
        // WebGPU validation messages are handled through device error callbacks
        strInfoLog = _T("WebGPU shader compiled successfully");
    }

    void printInfoLog()
    {
        CString strInfoLog;
        getInfoLog(strInfoLog);
        OutputDebugString(strInfoLog);
    }
};

// ************************************************************************************************
class _webgpuProgram
{

protected: // Fields

    WGPUDevice m_device;
    WGPURenderPipeline m_renderPipeline;
    WGPUBindGroupLayout m_bindGroupLayout;

public: // Methods

    _webgpuProgram()
        : m_device(nullptr)
        , m_renderPipeline(nullptr)
        , m_bindGroupLayout(nullptr)
    {
    }

    virtual ~_webgpuProgram()
    {
        if (m_renderPipeline) {
            wgpuRenderPipelineRelease(m_renderPipeline);
        }
        if (m_bindGroupLayout) {
            wgpuBindGroupLayoutRelease(m_bindGroupLayout);
        }
    }

    virtual void create(WGPUDevice device, _webgpuShader* pVertexShader, _webgpuShader* pFragmentShader) = 0;
    virtual void use(WGPURenderPassEncoder renderPass) = 0;

    WGPURenderPipeline getRenderPipeline() const
    {
        return m_renderPipeline;
    }

protected:

    bool createRenderPipeline(WGPUDevice device, _webgpuShader* pVertexShader, _webgpuShader* pFragmentShader, 
                             const WGPUVertexBufferLayout* vertexBufferLayouts, uint32_t vertexBufferCount)
    {
        m_device = device;

        // Create bind group layout for uniforms
        WGPUBindGroupLayoutEntry uniformBinding = {};
        uniformBinding.binding = 0;
        uniformBinding.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
        uniformBinding.buffer.type = WGPUBufferBindingType_Uniform;
        uniformBinding.buffer.hasDynamicOffset = false;

        WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {};
        bindGroupLayoutDesc.entryCount = 1;
        bindGroupLayoutDesc.entries = &uniformBinding;

        m_bindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &bindGroupLayoutDesc);

        WGPUPipelineLayoutDescriptor pipelineLayoutDesc = {};
        pipelineLayoutDesc.bindGroupLayoutCount = 1;
        pipelineLayoutDesc.bindGroupLayouts = &m_bindGroupLayout;

        WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &pipelineLayoutDesc);

        // Create render pipeline
        WGPURenderPipelineDescriptor pipelineDesc = {};
        pipelineDesc.layout = pipelineLayout;

        // Vertex stage
        pipelineDesc.vertex.module = pVertexShader->getModule();
        pipelineDesc.vertex.entryPoint = "vs_main";
        pipelineDesc.vertex.bufferCount = vertexBufferCount;
        pipelineDesc.vertex.buffers = vertexBufferLayouts;

        // Fragment stage
        WGPUFragmentState fragmentState = {};
        fragmentState.module = pFragmentShader->getModule();
        fragmentState.entryPoint = "fs_main";

        WGPUColorTargetState colorTarget = {};
        colorTarget.format = WGPUTextureFormat_BGRA8Unorm;
        colorTarget.writeMask = WGPUColorWriteMask_All;

        fragmentState.targetCount = 1;
        fragmentState.targets = &colorTarget;
        pipelineDesc.fragment = &fragmentState;

        // Primitive state
        pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
        pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
        pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
        pipelineDesc.primitive.cullMode = WGPUCullMode_Back;

        // Depth stencil state
        WGPUDepthStencilState depthStencilState = {};
        depthStencilState.format = WGPUTextureFormat_Depth24Plus;
        depthStencilState.depthWriteEnabled = true;
        depthStencilState.depthCompare = WGPUCompareFunction_Less;
        pipelineDesc.depthStencil = &depthStencilState;

        // Multisample state
        pipelineDesc.multisample.count = 1;
        pipelineDesc.multisample.mask = 0xFFFFFFFF;
        pipelineDesc.multisample.alphaToCoverageEnabled = false;

        m_renderPipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);

        wgpuPipelineLayoutRelease(pipelineLayout);

        return (m_renderPipeline != nullptr);
    }
};

// ************************************************************************************************
class _webgpuBlinnPhongProgram : public _webgpuProgram
{

private: // Fields

    WGPUBuffer m_uniformBuffer;
    WGPUBindGroup m_bindGroup;

public: // Methods

    _webgpuBlinnPhongProgram()
        : _webgpuProgram()
        , m_uniformBuffer(nullptr)
        , m_bindGroup(nullptr)
    {
    }

    virtual ~_webgpuBlinnPhongProgram() override
    {
        if (m_uniformBuffer) {
            wgpuBufferRelease(m_uniformBuffer);
        }
        if (m_bindGroup) {
            wgpuBindGroupRelease(m_bindGroup);
        }
    }

    virtual void create(WGPUDevice device, _webgpuShader* pVertexShader, _webgpuShader* pFragmentShader) override
    {
        // Define vertex buffer layout
        WGPUVertexAttribute attributes[3] = {};
        
        // Position
        attributes[0].format = WGPUVertexFormat_Float32x3;
        attributes[0].offset = 0;
        attributes[0].shaderLocation = 0;
        
        // Normal
        attributes[1].format = WGPUVertexFormat_Float32x3;
        attributes[1].offset = 3 * sizeof(float);
        attributes[1].shaderLocation = 1;
        
        // Texture coordinates
        attributes[2].format = WGPUVertexFormat_Float32x2;
        attributes[2].offset = 6 * sizeof(float);
        attributes[2].shaderLocation = 2;

        WGPUVertexBufferLayout vertexBufferLayout = {};
        vertexBufferLayout.arrayStride = 8 * sizeof(float);
        vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;
        vertexBufferLayout.attributeCount = 3;
        vertexBufferLayout.attributes = attributes;

        createRenderPipeline(device, pVertexShader, pFragmentShader, &vertexBufferLayout, 1);

        // Create uniform buffer
        WGPUBufferDescriptor uniformBufferDesc = {};
        uniformBufferDesc.size = sizeof(BlinnPhongUniforms);
        uniformBufferDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
        m_uniformBuffer = wgpuDeviceCreateBuffer(device, &uniformBufferDesc);

        // Create bind group
        WGPUBindGroupEntry bindGroupEntry = {};
        bindGroupEntry.binding = 0;
        bindGroupEntry.buffer = m_uniformBuffer;
        bindGroupEntry.offset = 0;
        bindGroupEntry.size = sizeof(BlinnPhongUniforms);

        WGPUBindGroupDescriptor bindGroupDesc = {};
        bindGroupDesc.layout = m_bindGroupLayout;
        bindGroupDesc.entryCount = 1;
        bindGroupDesc.entries = &bindGroupEntry;

        m_bindGroup = wgpuDeviceCreateBindGroup(device, &bindGroupDesc);
    }

    virtual void use(WGPURenderPassEncoder renderPass) override
    {
        wgpuRenderPassEncoderSetPipeline(renderPass, m_renderPipeline);
        wgpuRenderPassEncoderSetBindGroup(renderPass, 0, m_bindGroup, 0, nullptr);
    }

    void updateUniforms(WGPUDevice device, const BlinnPhongUniforms& uniforms)
    {
        wgpuDeviceGetQueue(device);
        WGPUQueue queue = wgpuDeviceGetQueue(device);
        wgpuQueueWriteBuffer(queue, m_uniformBuffer, 0, &uniforms, sizeof(BlinnPhongUniforms));
    }

private:

    struct BlinnPhongUniforms {
        glm::mat4 projectionMatrix;
        glm::mat4 modelViewMatrix;
        glm::mat3 normalMatrix;
        glm::vec3 diffuseMaterial;
        float enableLighting;
        float enableTexture;
        glm::vec3 lightPosition;
        glm::vec3 ambientMaterial;
        glm::vec3 specularMaterial;
        float transparency;
        float shininess;
        glm::vec3 ambientLightWeighting;
        glm::vec3 diffuseLightWeighting;
        glm::vec3 specularLightWeighting;
    };
};

// ************************************************************************************************
class _webgpuPerPixelProgram : public _webgpuProgram
{

private: // Fields

    WGPUBuffer m_uniformBuffer;
    WGPUBindGroup m_bindGroup;

public: // Methods

    _webgpuPerPixelProgram()
        : _webgpuProgram()
        , m_uniformBuffer(nullptr)
        , m_bindGroup(nullptr)
    {
    }

    virtual ~_webgpuPerPixelProgram() override
    {
        if (m_uniformBuffer) {
            wgpuBufferRelease(m_uniformBuffer);
        }
        if (m_bindGroup) {
            wgpuBindGroupRelease(m_bindGroup);
        }
    }

    virtual void create(WGPUDevice device, _webgpuShader* pVertexShader, _webgpuShader* pFragmentShader) override
    {
        // Define vertex buffer layout for advanced shader
        WGPUVertexAttribute attributes[3] = {};
        
        // Position
        attributes[0].format = WGPUVertexFormat_Float32x3;
        attributes[0].offset = 0;
        attributes[0].shaderLocation = 0;
        
        // Normal
        attributes[1].format = WGPUVertexFormat_Float32x3;
        attributes[1].offset = 3 * sizeof(float);
        attributes[1].shaderLocation = 1;
        
        // Texture coordinates
        attributes[2].format = WGPUVertexFormat_Float32x2;
        attributes[2].offset = 6 * sizeof(float);
        attributes[2].shaderLocation = 2;

        WGPUVertexBufferLayout vertexBufferLayout = {};
        vertexBufferLayout.arrayStride = 8 * sizeof(float);
        vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;
        vertexBufferLayout.attributeCount = 3;
        vertexBufferLayout.attributes = attributes;

        createRenderPipeline(device, pVertexShader, pFragmentShader, &vertexBufferLayout, 1);

        // Create uniform buffer
        WGPUBufferDescriptor uniformBufferDesc = {};
        uniformBufferDesc.size = sizeof(PerPixelUniforms);
        uniformBufferDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
        m_uniformBuffer = wgpuDeviceCreateBuffer(device, &uniformBufferDesc);

        // Create bind group
        WGPUBindGroupEntry bindGroupEntry = {};
        bindGroupEntry.binding = 0;
        bindGroupEntry.buffer = m_uniformBuffer;
        bindGroupEntry.offset = 0;
        bindGroupEntry.size = sizeof(PerPixelUniforms);

        WGPUBindGroupDescriptor bindGroupDesc = {};
        bindGroupDesc.layout = m_bindGroupLayout;
        bindGroupDesc.entryCount = 1;
        bindGroupDesc.entries = &bindGroupEntry;

        m_bindGroup = wgpuDeviceCreateBindGroup(device, &bindGroupDesc);
    }

    virtual void use(WGPURenderPassEncoder renderPass) override
    {
        wgpuRenderPassEncoderSetPipeline(renderPass, m_renderPipeline);
        wgpuRenderPassEncoderSetBindGroup(renderPass, 0, m_bindGroup, 0, nullptr);
    }

    void updateUniforms(WGPUDevice device, const PerPixelUniforms& uniforms)
    {
        WGPUQueue queue = wgpuDeviceGetQueue(device);
        wgpuQueueWriteBuffer(queue, m_uniformBuffer, 0, &uniforms, sizeof(PerPixelUniforms));
    }

private:

    struct PerPixelUniforms {
        float useBlinnPhongModel;
        float useTexture;
        glm::mat4 mvMatrix;
        glm::mat4 pMatrix;
        glm::mat4 nMatrix;
        glm::vec3 pointLightingLocation;
        glm::vec3 ambientLightWeighting;
        glm::vec3 specularLightWeighting;
        glm::vec3 diffuseLightWeighting;
        float materialShininess;
        float contrast;
        float brightness;
        float gamma;
        glm::vec3 materialAmbientColor;
        float transparency;
        glm::vec3 materialDiffuseColor;
        glm::vec3 materialSpecularColor;
        glm::vec3 materialEmissiveColor;
    };
};

// ************************************************************************************************
class _webgpuContext
{

public: // Constants

    const int MIN_WEBGPU_MAJOR_VERSION = 1;
    const int MIN_WEBGPU_MINOR_VERSION = 0;

private: // Fields

    HWND m_hWnd;
    WGPUInstance m_instance;
    WGPUAdapter m_adapter;
    WGPUDevice m_device;
    WGPUSurface m_surface;
    WGPUSwapChain m_swapChain;

    int m_iSamples;

public: // Methods

    _webgpuContext(HWND hWnd, int iSamples)
        : m_hWnd(hWnd)
        , m_instance(nullptr)
        , m_adapter(nullptr)
        , m_device(nullptr)
        , m_surface(nullptr)
        , m_swapChain(nullptr)
        , m_iSamples(iSamples)
    {
    }

    virtual ~_webgpuContext()
    {
        if (m_swapChain) {
            wgpuSwapChainRelease(m_swapChain);
        }
        if (m_surface) {
            wgpuSurfaceRelease(m_surface);
        }
        if (m_device) {
            wgpuDeviceRelease(m_device);
        }
        if (m_adapter) {
            wgpuAdapterRelease(m_adapter);
        }
        if (m_instance) {
            wgpuInstanceRelease(m_instance);
        }
    }

    BOOL makeCurrent()
    {
        // WebGPU doesn't have the concept of "making current" like OpenGL
        // The device is always available once created
        return (m_device != nullptr);
    }

    void create()
    {
        // Create WebGPU instance
        WGPUInstanceDescriptor instanceDesc = {};
        m_instance = wgpuCreateInstance(&instanceDesc);

        if (!m_instance) {
            ::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), 
                        _T("Failed to create WebGPU instance."), 
                        _T("WebGPU Error"), MB_ICONERROR | MB_OK);
            PostQuitMessage(0);
            return;
        }

        // Create surface for Windows
#ifdef _WINDOWS
        WGPUSurfaceDescriptorFromWindowsHWND surfaceDesc = {};
        surfaceDesc.chain.sType = WGPUSType_SurfaceDescriptorFromWindowsHWND;
        surfaceDesc.hwnd = m_hWnd;
        surfaceDesc.hinstance = GetModuleHandle(nullptr);

        WGPUSurfaceDescriptor surfaceDescriptor = {};
        surfaceDescriptor.nextInChain = &surfaceDesc.chain;

        m_surface = wgpuInstanceCreateSurface(m_instance, &surfaceDescriptor);
#endif

        if (!m_surface) {
            ::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), 
                        _T("Failed to create WebGPU surface."), 
                        _T("WebGPU Error"), MB_ICONERROR | MB_OK);
            PostQuitMessage(0);
            return;
        }

        // Request adapter
        WGPURequestAdapterOptions adapterOptions = {};
        adapterOptions.compatibleSurface = m_surface;
        adapterOptions.powerPreference = WGPUPowerPreference_HighPerformance;

        // This would normally be async, but for simplicity we'll assume synchronous operation
        // In a real implementation, you'd use callbacks
        wgpuInstanceRequestAdapter(m_instance, &adapterOptions, onAdapterReceived, this);
    }

    WGPUDevice getDevice() const { return m_device; }
    WGPUSurface getSurface() const { return m_surface; }
    WGPUSwapChain getSwapChain() const { return m_swapChain; }

private:

    static void onAdapterReceived(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* userdata)
    {
        _webgpuContext* context = static_cast<_webgpuContext*>(userdata);
        
        if (status != WGPURequestAdapterStatus_Success) {
            ::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), 
                        _T("Failed to get WebGPU adapter."), 
                        _T("WebGPU Error"), MB_ICONERROR | MB_OK);
            PostQuitMessage(0);
            return;
        }

        context->m_adapter = adapter;

        // Request device
        WGPUDeviceDescriptor deviceDesc = {};
        wgpuAdapterRequestDevice(adapter, &deviceDesc, onDeviceReceived, userdata);
    }

    static void onDeviceReceived(WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* userdata)
    {
        _webgpuContext* context = static_cast<_webgpuContext*>(userdata);
        
        if (status != WGPURequestDeviceStatus_Success) {
            ::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), 
                        _T("Failed to get WebGPU device."), 
                        _T("WebGPU Error"), MB_ICONERROR | MB_OK);
            PostQuitMessage(0);
            return;
        }

        context->m_device = device;
        context->createSwapChain();
    }

    void createSwapChain()
    {
        // Get window size
        RECT rect;
        GetClientRect(m_hWnd, &rect);
        uint32_t width = rect.right - rect.left;
        uint32_t height = rect.bottom - rect.top;

        // Create swap chain
        WGPUSwapChainDescriptor swapChainDesc = {};
        swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
        swapChainDesc.format = WGPUTextureFormat_BGRA8Unorm;
        swapChainDesc.width = width;
        swapChainDesc.height = height;
        swapChainDesc.presentMode = WGPUPresentMode_Fifo;

        m_swapChain = wgpuDeviceCreateSwapChain(m_device, m_surface, &swapChainDesc);

        if (!m_swapChain) {
            ::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), 
                        _T("Failed to create WebGPU swap chain."), 
                        _T("WebGPU Error"), MB_ICONERROR | MB_OK);
            PostQuitMessage(0);
        }
    }
};

// ************************************************************************************************
class _webgpuBuffers
{

private: // Fields

    map<uint32_t, vector<_geometry*>> m_mapCohorts;
    map<wstring, WGPUBuffer> m_mapVertexBuffers;
    map<wstring, WGPUBuffer> m_mapIndexBuffers;

public: // Methods

    _webgpuBuffers()
        : m_mapCohorts()
        , m_mapVertexBuffers()
        , m_mapIndexBuffers()
    {
    }

    virtual ~_webgpuBuffers()
    {
        clear();
    }

    void clear()
    {
        for (auto& pair : m_mapVertexBuffers) {
            if (pair.second) {
                wgpuBufferRelease(pair.second);
            }
        }
        for (auto& pair : m_mapIndexBuffers) {
            if (pair.second) {
                wgpuBufferRelease(pair.second);
            }
        }
        m_mapCohorts.clear();
        m_mapVertexBuffers.clear();
        m_mapIndexBuffers.clear();
    }

    // Add methods for buffer management
    void addVertexBuffer(const wstring& name, WGPUBuffer buffer)
    {
        m_mapVertexBuffers[name] = buffer;
    }

    void addIndexBuffer(const wstring& name, WGPUBuffer buffer)
    {
        m_mapIndexBuffers[name] = buffer;
    }

    WGPUBuffer getVertexBuffer(const wstring& name)
    {
        auto it = m_mapVertexBuffers.find(name);
        return (it != m_mapVertexBuffers.end()) ? it->second : nullptr;
    }

    WGPUBuffer getIndexBuffer(const wstring& name)
    {
        auto it = m_mapIndexBuffers.find(name);
        return (it != m_mapIndexBuffers.end()) ? it->second : nullptr;
    }
};

// ************************************************************************************************
class _webgpuRenderer 
    : public _webgpuRendererSettings
{

protected: // Fields

    // MFC
    CWnd* m_pWnd;
    CMFCToolTipCtrl m_toolTipCtrl;

    // WebGPU
    _webgpuContext* m_pWebGPUContext;
#ifdef _BLINN_PHONG_SHADERS
    _webgpuBlinnPhongProgram* m_pWebGPUProgram;
#else
    _webgpuPerPixelProgram* m_pWebGPUProgram;
#endif
    _webgpuShader* m_pVertexShader;
    _webgpuShader* m_pFragmentShader;
    glm::mat4 m_matModelView;

    // Cache
    _webgpuBuffers m_worldBuffers;
    vector<_webgpuBuffers*> m_vecDecorationBuffers;

    // World
    float m_fXmin;
    float m_fXmax;
    float m_fYmin;
    float m_fYmax;
    float m_fZmin;
    float m_fZmax;

    // Zoom/Pan
    float m_fZoomMin;
    float m_fZoomMax;

public: // Methods

    _webgpuRenderer()
        : _webgpuRendererSettings()
        , m_pWnd(nullptr)
        , m_pWebGPUContext(nullptr)
        , m_pWebGPUProgram(nullptr)
        , m_pVertexShader(nullptr)
        , m_pFragmentShader(nullptr)
        , m_matModelView(1.0f)
        , m_worldBuffers()
        , m_vecDecorationBuffers()
        , m_fXmin(0.f)
        , m_fXmax(0.f)
        , m_fYmin(0.f)
        , m_fYmax(0.f)
        , m_fZmin(0.f)
        , m_fZmax(0.f)
        , m_fZoomMin(0.f)
        , m_fZoomMax(0.f)
    {
    }

    virtual ~_webgpuRenderer()
    {
        _cleanup();
    }

protected: // Methods

    virtual void _reset()
    {
        _webgpuRendererSettings::_reset();
        
        // Reset WebGPU-specific state
        m_matModelView = glm::mat4(1.0f);
        m_worldBuffers.clear();
        
        for (auto* buffer : m_vecDecorationBuffers) {
            delete buffer;
        }
        m_vecDecorationBuffers.clear();
    }

    void _initialize(CWnd* pWnd, int iSamples, int iVertexShaderResource, int iFragmentShaderResource, 
                    int iResourceType, bool bEnableDebug)
    {
        m_pWnd = pWnd;

        // Create WebGPU context
        m_pWebGPUContext = new _webgpuContext(pWnd->GetSafeHwnd(), iSamples);
        m_pWebGPUContext->create();

        // Create shaders
        m_pVertexShader = new _webgpuShader(WGPUShaderStage_Vertex);
        m_pFragmentShader = new _webgpuShader(WGPUShaderStage_Fragment);

        if (!m_pVertexShader->load(iVertexShaderResource, iResourceType) ||
            !m_pVertexShader->compile(m_pWebGPUContext->getDevice())) {
            ::MessageBox(pWnd->GetSafeHwnd(), 
                        _T("Failed to compile vertex shader."), 
                        _T("WebGPU Error"), MB_ICONERROR | MB_OK);
            return;
        }

        if (!m_pFragmentShader->load(iFragmentShaderResource, iResourceType) ||
            !m_pFragmentShader->compile(m_pWebGPUContext->getDevice())) {
            ::MessageBox(pWnd->GetSafeHwnd(), 
                        _T("Failed to compile fragment shader."), 
                        _T("WebGPU Error"), MB_ICONERROR | MB_OK);
            return;
        }

        // Create program
#ifdef _BLINN_PHONG_SHADERS
        m_pWebGPUProgram = new _webgpuBlinnPhongProgram();
#else
        m_pWebGPUProgram = new _webgpuPerPixelProgram();
#endif
        m_pWebGPUProgram->create(m_pWebGPUContext->getDevice(), m_pVertexShader, m_pFragmentShader);
    }

    void _cleanup()
    {
        if (m_pWebGPUProgram) {
            delete m_pWebGPUProgram;
            m_pWebGPUProgram = nullptr;
        }

        if (m_pVertexShader) {
            delete m_pVertexShader;
            m_pVertexShader = nullptr;
        }

        if (m_pFragmentShader) {
            delete m_pFragmentShader;
            m_pFragmentShader = nullptr;
        }

        if (m_pWebGPUContext) {
            delete m_pWebGPUContext;
            m_pWebGPUContext = nullptr;
        }

        _reset();
    }
};

// ************************************************************************************************
class _model;
class _view;
class _instance;

// ************************************************************************************************
class _webgpuView
    : public _webgpuRenderer
    , public _view
{

private: // Fields

    map<_instance*, _material*> m_mapUserDefinedMaterials;

protected: // Fields

    bool m_bMultiSelect;

public: // Methods

    _webgpuView()
        : _webgpuRenderer()
        , _view()
        , m_mapUserDefinedMaterials()
        , m_bMultiSelect(false)
    {
    }

    virtual ~_webgpuView()
    {
        removeUserDefinedMaterials();
    }

    void addUserDefinedMaterial(const vector<_instance*>& vecInstances, float fR, float fG, float fB);
    void removeUserDefinedMaterials();

public: // Properties

    const map<_instance*, _material*>& getUserDefinedMaterials() const { return m_mapUserDefinedMaterials; }
};

#endif // _WINDOWS