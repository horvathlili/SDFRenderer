#pragma once
#include "Falcor.h"
#include "ComputeProgramWrapper.h"
#include "Eigen/Dense"

using namespace Falcor;

class SDFRenderer : public IRenderer
{
public:
    void onLoad(RenderContext* pRenderContext) override;
    void onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo) override;
    void onShutdown() override;
    void onResizeSwapChain(uint32_t width, uint32_t height) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;
    void onGuiRender(Gui* pGui) override;

private:

    GraphicsProgram::SharedPtr mProgram;
    GraphicsVars::SharedPtr mpVars;
    GraphicsState::SharedPtr mpState;
    Buffer::SharedPtr pVbo;
    Vao::SharedPtr pVao;

    struct Vertex
    {
        float3 pos;
        float3 col;
    };

    void initData();
    void initCamera();

    Camera::SharedPtr camera;
    CameraController::SharedPtr ccontrol;

    float2 resolution = float2(1280, 720);

    uint32_t sdf = 1;

    ComputeProgramWrapper::SharedPtr mComputeProgram;
    ComputeProgramWrapper::SharedPtr mComputeProgramFirstOrder;
    Texture::SharedPtr sdfTexture;
    Texture::SharedPtr generateTexture(RenderContext* pRenderContext);
    uint32_t textureOrder = 0;

    int res = 256;
    bool retexture = false;

    uint32_t texturesize = 1;

    Sampler::SharedPtr mpSampler;

    DebugConsole console;

    float x0[108];
    void getPseudoInverse();


};
