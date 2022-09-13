#include "SDFRenderer.h"

#if FALCOR_D3D12_AVAILABLE
FALCOR_EXPORT_D3D12_AGILITY_SDK
#endif


void SDFRenderer::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "debug", { 300, 70 }, { 10, 80 });
    Gui::RadioButton sphere;
    sphere.label = "Sphere";
    sphere.buttonID = 1;
    Gui::RadioButton torus;
    torus.label = "Torus";
    torus.buttonID = 2;
    Gui::RadioButton box;
    box.label = "Box";
    box.buttonID = 3;
    Gui::RadioButtonGroup bg;
    bg.push_back(sphere);
    bg.push_back(torus);
    bg.push_back(box);
    w.radioButtons(bg, sdf);
}


void SDFRenderer::initData() {

    const Vertex vertices[] =
    {
        {float3(-1, -1, 0), float3(1, 0, 0)},
        {float3(1, -1, 0), float3(0, 1,0)},
        {float3(-1, 1, 0), float3(0, 0,1)},
        {float3(1, 1, 0), float3(0,0, 0)},
        };

    const uint32_t vbSize = (uint32_t)(sizeof(Vertex) * arraysize(vertices));
    
    pVbo = Buffer::create(vbSize, Buffer::BindFlags::Vertex, Buffer::CpuAccess::Write, (void*)vertices);
    FALCOR_ASSERT(pVbo);

    // Create VAO
    VertexLayout::SharedPtr pLayout = VertexLayout::create();
    VertexBufferLayout::SharedPtr pBufLayout = VertexBufferLayout::create();
    pBufLayout->addElement("POSITION", 0, ResourceFormat::RGB32Float, 1, 0);
    pBufLayout->addElement("COLOR", 12, ResourceFormat::RGB32Float, 1, 1);
    pLayout->addBufferLayout(0, pBufLayout);

    Vao::BufferVec buffers{ pVbo };
    pVao = Vao::create(Vao::Topology::TriangleStrip, pLayout, buffers);
    FALCOR_ASSERT(pVao);
}

void SDFRenderer::initCamera() {

    camera = Camera::create();
    camera->setPosition(float3(0, 0, 5));
    camera->setTarget(float3(0, 0, 0));
    camera->setUpVector(float3(0, 1, 0));
    camera->setAspectRatio(1280.0f / 720.0f);
    camera->setNearPlane(0.01f);
    camera->setFarPlane(1000.0f);

    ccontrol = FirstPersonCameraController::create(camera);
    ccontrol->setCameraSpeed(1.0f);
}

void SDFRenderer::onLoad(RenderContext* pRenderContext)
{
    Program::Desc d;

    d.addShaderLibrary("Samples/SDFRenderer/Shaders/SDFRenderer.vs.slang").entryPoint(ShaderType::Vertex, "main");
    d.addShaderLibrary("Samples/SDFRenderer/Shaders/SDFRenderer.ps.slang").entryPoint(ShaderType::Pixel, "main");

    mProgram = GraphicsProgram::create(d);
    FALCOR_ASSERT(mProgram);

    mpState = GraphicsState::create();
    mpState->setProgram(mProgram);
    FALCOR_ASSERT(mpState);

    auto pDsState = DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(false));
    mpState->setDepthStencilState(pDsState);

    initData();
    mpState->setVao(pVao);

    initCamera();

    mpVars = GraphicsVars::create(mpState->getProgram().get());

   
}

void SDFRenderer::onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo)
{
    mpState->setFbo(pTargetFbo);
    float4 clearColor = float4(1, 1, 1, 1);
    pRenderContext->clearFbo(mpState->getFbo().get(), clearColor, 1.0f, 0);

    ccontrol->update();
    mpVars["psCb"]["eye"] = camera->getPosition();
    mpVars["psCb"]["center"] = camera->getTarget();
    mpVars["psCb"]["up"] = camera->getUpVector();
    mpVars["psCb"]["ar"] = camera->getAspectRatio();
    mpVars["psCb"]["sdf"] = sdf;

    pRenderContext->draw(mpState.get(), mpVars.get(), 4, 0);
}

void SDFRenderer::onShutdown()
{
}

bool SDFRenderer::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return ccontrol->onKeyEvent(keyEvent);
}

bool SDFRenderer::onMouseEvent(const MouseEvent& mouseEvent)
{
    return ccontrol->onMouseEvent(mouseEvent);
}

void SDFRenderer::onHotReload(HotReloadFlags reloaded)
{
}

void SDFRenderer::onResizeSwapChain(uint32_t width, uint32_t height)
{
    camera->setAspectRatio((float)width / (float)height);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    SDFRenderer::UniquePtr pRenderer = std::make_unique<SDFRenderer>();
    SampleConfig config;
    config.windowDesc.title = "Falcor Project Template";
    config.windowDesc.width = 1280;
    config.windowDesc.height = 720;
    config.deviceDesc.enableVsync = true;
    config.windowDesc.resizableWindow = true;
    Sample::run(config, pRenderer);
    return 0;
}
