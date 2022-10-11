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

    if (w.slider("resolution", res, 10, 256))
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

    Gui::RadioButton tex0;
    tex0.label = "order 0";
    tex0.buttonID = 0;
    Gui::RadioButton tex1;
    tex1.label = "order 1";
    tex1.buttonID = 1;

    Gui::RadioButtonGroup texorder;
    texorder.push_back(tex0);
    texorder.push_back(tex1);

    w.separator();

    if (w.radioButtons(texorder, textureOrder))
        retexture = true;

    if (w.slider("boundingBox", boundingbox, 2, 20))
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

void SDFRenderer::initBox() {
    std::vector<Vertex>vertices;

    //front									 
    vertices.push_back({ float3(-0.5, -0.5, +0.5), float3(1,1,1) });
    vertices.push_back({float3(+0.5, -0.5, +0.5), float3(1,1,1) });
    vertices.push_back({ float3(-0.5, +0.5, +0.5), float3(1,1,1) });
    vertices.push_back({ float3(+0.5, +0.5, +0.5), float3(1,1,1) });
    //back
    vertices.push_back({ float3(+0.5, -0.5, -0.5), float3(1,1,1) });
    vertices.push_back({ float3(-0.5, -0.5, -0.5), float3(1,1,1) });
    vertices.push_back({ float3(+0.5, +0.5, -0.5), float3(1,1,1) });
    vertices.push_back({ float3(-0.5, +0.5, -0.5), float3(1,1,1) });
    //right									 
    vertices.push_back({ float3(+0.5, -0.5, +0.5), float3(1,1,1) });
    vertices.push_back({ float3(+0.5, -0.5, -0.5), float3(1,1,1) });
    vertices.push_back({ float3(+0.5, +0.5, +0.5), float3(1,1,1) });
    vertices.push_back({ float3(+0.5, +0.5, -0.5), float3(1,1,1) });
    //left									 
    vertices.push_back({ float3(-0.5, -0.5, -0.5), float3(1,1,1) });
    vertices.push_back({ float3(-0.5, -0.5, +0.5), float3(1,1,1) });
    vertices.push_back({ float3(-0.5, +0.5, -0.5), float3(1,1,1) });
    vertices.push_back({ float3(-0.5, +0.5, +0.5), float3(1,1,1) });
    //top									 
    vertices.push_back({ float3(-0.5, +0.5, +0.5), float3(1,1,1) });
    vertices.push_back({ float3(+0.5, +0.5, +0.5), float3(1,1,1) });
    vertices.push_back({ float3(-0.5, +0.5, -0.5), float3(1,1,1) });
    vertices.push_back({ float3(+0.5, +0.5, -0.5), float3(1,1,1) });
    //bottom								 
    vertices.push_back({ float3(-0.5, -0.5, -0.5), float3(1,1,1) });
    vertices.push_back({ float3(+0.5, -0.5, -0.5), float3(1,1,1) });
    vertices.push_back({ float3(-0.5, -0.5, +0.5), float3(1,1,1) });
    vertices.push_back({ float3(+0.5, -0.5, +0.5), float3(1,1,1) });

    Vertex indices[36];
    int index = 0;
    //4 csúcspontonként 6 index eltárolása
    for (int i = 0; i < 6 * 4; i += 4)
    {
        indices[index + 0] = vertices[i + 0];
        indices[index + 1] = vertices[i + 1];
        indices[index + 2] = vertices[i + 2];
        indices[index + 3] = vertices[i + 1];
        indices[index + 4] = vertices[i + 3];
        indices[index + 5] = vertices[i + 2];
        index += 6;
    }

    const uint32_t vbSize = (uint32_t)(sizeof(Vertex) * arraysize(indices));

    cubeVbo = Buffer::create(vbSize, Buffer::BindFlags::Vertex, Buffer::CpuAccess::Write, (void*)indices);
    FALCOR_ASSERT(cubeVbo);

    // Create VAO
    VertexLayout::SharedPtr pLayout = VertexLayout::create();
    VertexBufferLayout::SharedPtr pBufLayout = VertexBufferLayout::create();
    pBufLayout->addElement("POSITION", 0, ResourceFormat::RGB32Float, 1, 0);
    pBufLayout->addElement("COLOR", 12, ResourceFormat::RGB32Float, 1, 1);
    pLayout->addBufferLayout(0, pBufLayout);

    Vao::BufferVec buffers{ cubeVbo };
    cubeVao = Vao::create(Vao::Topology::TriangleList, pLayout, buffers);
    FALCOR_ASSERT(cubeVao);
}

void SDFRenderer::initCamera() {

    camera = Camera::create();
    camera->setPosition(float3(0.75,0.75, -0.75));
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
    initBox();

    //mpState->setVao(pVao);
 
    initCamera();

    mpVars = GraphicsVars::create(mpState->getProgram().get());

    mComputeProgram = ComputeProgramWrapper::create();
    mComputeProgram->createProgram("Samples/SDFRenderer/Shaders/SDFTexture.cs.slang");
    mComputeProgramFirstOrder = ComputeProgramWrapper::create();
    mComputeProgramFirstOrder->createProgram("Samples/SDFRenderer/Shaders/FirstOrder.cs.slang");


    {
        Sampler::Desc desc;
        desc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear);
        mpSampler = Sampler::create(desc);
    }

    sdfTexture = generateTexture(pRenderContext);
   
    mpVars["mSampler"] = mpSampler;
}

bool SDFRenderer::isOutOfBox(float3 pos)
{
    float b = (float)boundingbox;
    bool r = (pos.x > -b/2.0 && pos.x < b/2.0 &&
        pos.y > -b/2.0 && pos.y < b/2.0 &&
        pos.z > -b/2.0 && pos.z < b/2.0);

    return !r;

}

void SDFRenderer::onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo)
{
    if (retexture) {
        sdfTexture = generateTexture(pRenderContext);
        retexture = false;
    }
    
    mpVars["texture"] = sdfTexture;

    mpState->setFbo(pTargetFbo);
    float4 clearColor = float4(0, 0, 0, 1);
    pRenderContext->clearFbo(mpState->getFbo().get(), clearColor, 1.0f, 0);

    ccontrol->update();
    //kívül vagyunk-e a boundingboxon
    isbox = isOutOfBox(camera->getPosition());

    mpVars["psCb"]["eye"] = camera->getPosition();
    mpVars["psCb"]["center"] = camera->getTarget();
    mpVars["psCb"]["up"] = camera->getUpVector();
    mpVars["psCb"]["ar"] = camera->getAspectRatio();
    mpVars["psCb"]["sdf"] = sdf;
    mpVars["psCb"]["texorder"] = textureOrder;
    mpVars["psCb"]["box"] = isbox;
    mpVars["psCb"]["res"] = res;
    mpVars["psCb"]["boundingBox"] = (float)boundingbox;

    float4x4 m = glm::scale(float4x4(1.0), float3((float)boundingbox));
    mpVars["vsCb"]["model"] = m;
    mpVars["vsCb"]["viewproj"] = camera->getViewProjMatrix();
    mpVars["vsCb"]["box"] = isbox;

   if (isbox) {
        mpState->setVao(cubeVao);
        pRenderContext->draw(mpState.get(), mpVars.get(), 36, 0);
   }
   else {
       mpState->setVao(pVao);
       pRenderContext->draw(mpState.get(), mpVars.get(), 4, 0);
   }

    
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

    
    auto& comp = (textureOrder == 0) ? *mComputeProgram : *mComputeProgramFirstOrder;

    comp["tex3D_uav"].setUav(pTex->getUAV(0));
    comp["csCb"]["sdf"] = sdf;
    comp["csCb"]["res"] = res;
    comp["csCb"]["boundingBox"] = (float)boundingbox;

    if (textureOrder == 1) {
        getPseudoInverse();
        comp.allocateStructuredBuffer("x0", 108, x0->data(), sizeof(float) * 108);
    }

    comp.runProgram(pRenderContext,res,res,res);

    return pTex;
}

void SDFRenderer::getPseudoInverse() {

    Eigen::MatrixXf m(27,4);

    x0->clear();

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            for (int k = -1; k <= 1; k++) {

                float3 x = float3(i, j, k) * 2.0f / (float)res * 0.6f;

                m((i + 1) * 9 + (j + 1) * 3 + (k + 1),0) = x.x;
                m((i + 1) * 9 + (j + 1) * 3 + (k + 1),1) = x.y;
                m((i + 1) * 9 + (j + 1) * 3 + (k + 1),2) = x.z;
                m((i + 1) * 9 + (j + 1) * 3 + (k + 1),3) = 1.0;
                
            }
        }
    }

  

    Eigen::MatrixXf result = (m.transpose()*m).inverse() * m.transpose();

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 27; j++) {
            x0->push_back(result(i, j));
        }
    }
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
