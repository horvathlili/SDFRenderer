#include "SDFRenderer.h"
#include "Utils/PseudoInverse.h"
#include "torusfitting.h"

#if FALCOR_D3D12_AVAILABLE
FALCOR_EXPORT_D3D12_AGILITY_SDK
#endif

void SDFRenderer::setUpGui() {
    Gui::RadioButton sphere;
    sphere.label = "Sphere";
    sphere.buttonID = 1;
    Gui::RadioButton torus;
    torus.label = "Torus";
    torus.buttonID = 2;
    Gui::RadioButton box;
    box.label = "Box";
    box.buttonID = 3;
    Gui::RadioButton teapot;
    teapot.label = "Teapot";
    teapot.buttonID = 4;
    bg.push_back(sphere);
    bg.push_back(torus);
    bg.push_back(box);
    bg.push_back(teapot);

    Gui::RadioButton tex16;
    tex16.label = "16 bit";
    tex16.buttonID = 1;
    Gui::RadioButton tex32;
    tex32.label = "32 bit";
    tex32.buttonID = 2;
    texsize.push_back(tex16);
    texsize.push_back(tex32);

    Gui::RadioButton tex0;
    tex0.label = "order 0";
    tex0.buttonID = 0;
    Gui::RadioButton tex1;
    tex1.label = "order 1";
    tex1.buttonID = 1;
    Gui::RadioButton texA2;
    texA2.label = "Algebrai 2";
    texA2.buttonID = 2;
    texorder.push_back(tex0);
    texorder.push_back(tex1);
    texorder.push_back(texA2);

    Gui::RadioButton dfs;
    dfs.label = "distance fields";
    dfs.buttonID = 0;
    Gui::RadioButton tf;
    tf.label = "torus fitting";
    tf.buttonID = 1;
    function.push_back(dfs);
    function.push_back(tf);
}

void SDFRenderer::onGuiRender(Gui* pGui)
{

    Gui::Window w(pGui, "debug", { 350, 200 }, { 10, 90 });
    
    w.radioButtons(function, func);
    w.separator();

    if (func == 0) {
        if (w.radioButtons(bg, sdf))
            retexture = true;

        if (w.slider("resolution", res, 10, 256))
            retexture = true;

        w.separator();

        if (w.radioButtons(texsize, texturesize))
            retexture = true;

        w.separator();

        if (w.radioButtons(texorder, textureOrder))
            retexture = true;

        if (w.slider("boundingBox", boundingbox, 2, 20))
            retexture = true;
    }
    else {
        if (w.slider("number of points", numberofpoints, 30, 300))
            newpoints = true;
    }
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

void SDFRenderer::initFittingProgram() {
    Program::Desc d1;

    d1.addShaderLibrary("Samples/SDFRenderer/Shaders/TorusFitting.vs.slang").entryPoint(ShaderType::Vertex, "main");
    d1.addShaderLibrary("Samples/SDFRenderer/Shaders/TorusFitting.ps.slang").entryPoint(ShaderType::Pixel, "main");

    fittingProgram = GraphicsProgram::create(d1);
    FALCOR_ASSERT(fittingProgram);

    fittingState = GraphicsState::create();
    fittingState->setProgram(fittingProgram);
    FALCOR_ASSERT(fittingState);

    fittingVars = GraphicsVars::create(fittingState->getProgram().get());
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

    initData();
    initBox();

    //mpState->setVao(pVao);
 
    initCamera();

    mpVars = GraphicsVars::create(mpState->getProgram().get());

    initFittingProgram();

    mComputeProgram = ComputeProgramWrapper::create();
    mComputeProgram->createProgram("Samples/SDFRenderer/Shaders/SDFTexture.cs.slang");
    mComputeProgramFirstOrder = ComputeProgramWrapper::create();
    mComputeProgramFirstOrder->createProgram("Samples/SDFRenderer/Shaders/FirstOrder.cs.slang");
    mComputeProgramA2 = ComputeProgramWrapper::create();
    mComputeProgramA2->createProgram("Samples/SDFRenderer/Shaders/A2.cs.slang");


    {
        Sampler::Desc desc;
        desc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear);
        mpSampler = Sampler::create(desc);
    }

    sdfTextures = generateTexture(pRenderContext);
   
    mpVars["mSampler"] = mpSampler;

    setUpGui();

    toruspoints = getTorusPoints(numberofpoints, 0.3f, 1.0f);
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
    mpState->setFbo(pTargetFbo);
    fittingState->setFbo(pTargetFbo);
    float4 clearColor = float4(0, 0, 0, 1);
    pRenderContext->clearFbo(mpState->getFbo().get(), clearColor, 1.0f, 0);
    pRenderContext->clearFbo(fittingState->getFbo().get(), clearColor, 1.0f, 0);

    ccontrol->update();

    if (func == 0) {
        if (retexture) {
            sdfTextures = generateTexture(pRenderContext);
            retexture = false;
        }

        float4x4 m = glm::scale(float4x4(1.0), float3((float)boundingbox));

        //sdf-ek
        mpVars["texture1"] = sdfTextures[0];

        if (textureOrder == 2) {
            mpVars["texture2"] = sdfTextures[1];
            mpVars["texture3"] = sdfTextures[2];
        }

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
        mpVars["psCb"]["viewproj"] = camera->getViewProjMatrix();


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
    else {
        //tóruszillesztés
        if (newpoints) {
            toruspoints = getNewPoints(numberofpoints);
            newpoints = false;
        }

        Buffer::SharedPtr pBuffer = Buffer::createStructured(fittingState->getProgram().get(), "points", numberofpoints);
        pBuffer->setBlob(toruspoints.data(),0, numberofpoints*sizeof(float3));
        
        float4x4 m = glm::scale(float4x4(1.0), float3((float)boundingbox));
        fittingVars["vsCb"]["model"] = m;
        fittingVars["vsCb"]["viewproj"] = camera->getViewProjMatrix();
        fittingVars["vsCb"]["box"] = isbox;
        fittingVars["psCb"]["eye"] = camera->getPosition();
        fittingVars["psCb"]["center"] = camera->getTarget();
        fittingVars["psCb"]["up"] = camera->getUpVector();
        fittingVars["psCb"]["ar"] = camera->getAspectRatio();
        fittingVars["psCb"]["n"] = numberofpoints;

        fittingVars->setBuffer("points", pBuffer);
        
        fittingState->setVao(pVao);
        pRenderContext->draw(fittingState.get(), fittingVars.get(), 4, 0);
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



std::vector<Texture::SharedPtr> SDFRenderer::generateTexture(RenderContext* pRenderContext) {

    Texture::SharedPtr pTex1 = nullptr;
    Texture::SharedPtr pTex2 = nullptr;
    Texture::SharedPtr pTex3 = nullptr;

    if (texturesize == 1) {
        pTex1 = Texture::create3D(res, res, res, ResourceFormat::RGBA16Float, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess);
        pTex2 = Texture::create3D(res, res, res, ResourceFormat::RGBA16Float, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess);
        pTex3 = Texture::create3D(res, res, res, ResourceFormat::RGBA16Float, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess);

    }
    if (texturesize == 2) {
        pTex1 = Texture::create3D(res, res, res, ResourceFormat::RGBA32Float, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess);
        pTex2 = Texture::create3D(res, res, res, ResourceFormat::RGBA32Float, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess);
        pTex3 = Texture::create3D(res, res, res, ResourceFormat::RGBA32Float, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess);
    }
    
    auto& comp = textureOrder == 0 ? *mComputeProgram : (textureOrder == 1 ? *mComputeProgramFirstOrder : *mComputeProgramA2);
   
    comp["tex3D_uav1"].setUav(pTex1->getUAV(0));
    comp["csCb"]["sdf"] = sdf;
    comp["csCb"]["res"] = res;
    comp["csCb"]["boundingBox"] = (float)boundingbox;

    if (textureOrder == 1) {
        x0 = getPseudoInverse1(boundingbox,res);
        comp.allocateStructuredBuffer("x0", 108, x0.data(), sizeof(float) * 108);
    }

    if (textureOrder == 2) {
        
        comp["tex3D_uav2"].setUav(pTex2->getUAV(0));
        comp["tex3D_uav3"].setUav(pTex3->getUAV(0));
        x0 = getPseudoInverse2(boundingbox, res);
        comp.allocateStructuredBuffer("x0", 270, x0.data(), sizeof(float) * 270);
    }


    comp.runProgram(pRenderContext,res,res,res);

    std::vector<Texture::SharedPtr> textures;
    textures.push_back(pTex1);
    textures.push_back(pTex2);
    textures.push_back(pTex3);


    return textures;
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
