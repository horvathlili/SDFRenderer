#include "SDFRenderer.h"

#if FALCOR_D3D12_AVAILABLE
FALCOR_EXPORT_D3D12_AGILITY_SDK
#endif


void SDFRenderer::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "debug", { 300, 150 }, { 10, 90 });
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

    if (w.radioButtons(bg, sdf))
        retexture = true;

    if (w.slider("resolution", res, 10, 750))
        retexture = true;

    if (w.slider("Bounding box", boundingBox, 0, 200))
        retexture = true;

    Gui::RadioButton tex16;
    tex16.label = "16 bit";
    tex16.buttonID = 1;
    Gui::RadioButton tex32;
    tex32.label = "32 bit";
    tex32.buttonID = 2;

    Gui::RadioButtonGroup texsize;
    texsize.push_back(tex16);
    texsize.push_back(tex32);

    w.separator();

    if (w.radioButtons(texsize, texturesize))
        retexture = true;
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
    camera->setPosition(float3(0, -5, 10));
    camera->setTarget(float3(0, 0, 0));
    camera->setUpVector(float3(0, 1, 0));
    camera->setAspectRatio(1280.0f / 720.0f);
    camera->setNearPlane(0.01f);
    camera->setFarPlane(1000.0f);
    camera->beginFrame();

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

    mComputeProgram = ComputeProgramWrapper::create();
    mComputeProgram->createProgram("Samples/SDFRenderer/Shaders/SDFTexture.cs.slang");

    {
        Sampler::Desc desc;
        desc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear);
        mpSampler = Sampler::create(desc);
    }

    sdfTexture = generateTexture(pRenderContext);

    mpVars["mSampler"] = mpSampler;

}

void SDFRenderer::onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo)
{
    //textúra generálás
    if (retexture) {
        sdfTexture = generateTexture(pRenderContext);
        retexture = false;
    }
    
    mpVars["texture"] = sdfTexture;

    mpState->setFbo(pTargetFbo);
    float4 clearColor = float4(1, 1, 1, 1);
    pRenderContext->clearFbo(mpState->getFbo().get(), clearColor, 1.0f, 0);

    ccontrol->update();
    mpVars["psCb"]["eye"] = camera->getPosition();
    mpVars["psCb"]["center"] = camera->getTarget();
    mpVars["psCb"]["up"] = camera->getUpVector();
    mpVars["psCb"]["ar"] = camera->getAspectRatio();
    mpVars["psCb"]["sdf"] = sdf;
    mpVars["psCb"]["boundingBox"] = boundingBox;

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

Texture::SharedPtr SDFRenderer::generateTexture(RenderContext* pRenderContext) {

    Texture::SharedPtr pTex = nullptr;

    if (texturesize == 1)
        pTex = Texture::create3D(res,res,res,ResourceFormat::RGBA16Float, 1 ,nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess);

    if (texturesize == 2)
        pTex = Texture::create3D(res, res, res, ResourceFormat::RGBA32Float, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess);

    auto comp = *mComputeProgram;
    comp["tex3D_uav"].setUav(pTex->getUAV(0));
    comp["csCb"]["sdf"].set(sdf);
    comp["csCb"]["boundingBox"].set(boundingBox);
    comp["csCb"]["res"].set(res);

    comp.runProgram(pRenderContext,res,res,res);

    return pTex;
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
