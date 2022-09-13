#pragma once
#include "Falcor.h"

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

    uint32_t sdf = 1;

   // DebugConsole console;


};
