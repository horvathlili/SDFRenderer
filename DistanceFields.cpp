#include "DistanceFields.h"
#include "Utils/PseudoInverse.h"


#if FALCOR_D3D12_AVAILABLE
FALCOR_EXPORT_D3D12_AGILITY_SDK
#endif

void DistanceFields::setUpGui() {
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
    tf.label = "tf";
    tf.buttonID = 1;
    Gui::RadioButton cf;
    cf.label = "tf with clustering";
    cf.buttonID = 2;
    Gui::RadioButton g2;
    g2.label = "geometric 2";
    g2.buttonID = 3;
    function.push_back(dfs);
    function.push_back(g2);
    function.push_back(tf);
    function.push_back(cf);


    Gui::RadioButton s1;
    s1.label = "1";
    s1.buttonID = 1;
    Gui::RadioButton s2;
    s2.label = "2";
    s2.buttonID = 2;
    Gui::RadioButton s3;
    s3.label = "3";
    s3.buttonID = 3;
    spheres.push_back(s1);
    spheres.push_back(s2);
    spheres.push_back(s3);
}

void DistanceFields::onGuiRender(Gui* pGui)
{

    Gui::Window w(pGui, "debug", { 350, 200 }, { 10, 90 });

    if (w.radioButtons(function, func)) {
        newpoints = true;
    }
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
    if (func == 1) {
        if (w.slider("number of points", numberofpoints, 2, 10))
            newpoints = true;
        if (w.slider("position", point, 0.f, 2.0f)) {
            newpoints = true;
        }
    }
    if (func == 2) {
        w.text("Right cluster: " + std::to_string(clust.right_cluster));
        w.text("Wrong cluster: " + std::to_string(clust.wrong_cluster));
        if (w.radioButtons(spheres, sphere)) {
            newpoints = true;
        }
        if (w.slider("position", point, 0.f, 2.0f)) {
            newpoints = true;
        }
        if (w.button("one cluster"))
            oneCluster = !oneCluster;
    }
    if (func == 3) {
        if (w.slider("resolution", res, 10, 256))
            retexture = true;
        w.separator();
        if (w.radioButtons(texsize, texturesize))
            retexture = true;
        if (w.slider("boundingBox", boundingbox, 2, 20))
            retexture = true;
        if (w.slider(" ", db, 1, 10))
            retexture = true;
    }

}


void DistanceFields::initData() {

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

void DistanceFields::initBox() {
    std::vector<Vertex>vertices;

    //front									 
    vertices.push_back({ float3(-0.5, -0.5, +0.5), float3(1,1,1) });
    vertices.push_back({ float3(+0.5, -0.5, +0.5), float3(1,1,1) });
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

void DistanceFields::initCamera() {

    camera = Camera::create();
    camera->setPosition(float3(0.75, 0.75, -0.75));
    camera->setTarget(float3(0, 0, 0));
    camera->setUpVector(float3(0, 1, 0));
    camera->setAspectRatio(1280.0f / 720.0f);
    camera->setNearPlane(0.01f);
    camera->setFarPlane(1000.0f);
    camera->beginFrame();

    ccontrol = FirstPersonCameraController::create(camera);
    ccontrol->setCameraSpeed(1.0f);
}

void DistanceFields::initFittingProgram() {
    Program::Desc d1;

    d1.addShaderLibrary("Samples/DistanceFields/Shaders/TorusFitting.vs.slang").entryPoint(ShaderType::Vertex, "main");
    d1.addShaderLibrary("Samples/DistanceFields/Shaders/TorusFitting.ps.slang").entryPoint(ShaderType::Pixel, "main");

    fittingProgram = GraphicsProgram::create(d1);
    FALCOR_ASSERT(fittingProgram);

    fittingState = GraphicsState::create();
    fittingState->setProgram(fittingProgram);
    FALCOR_ASSERT(fittingState);

    fittingVars = GraphicsVars::create(fittingState->getProgram().get());
}

void DistanceFields::onLoad(RenderContext* pRenderContext)
{
    Program::Desc d;

    d.addShaderLibrary("Samples/DistanceFields/Shaders/DistanceFields.vs.slang").entryPoint(ShaderType::Vertex, "main");
    d.addShaderLibrary("Samples/DistanceFields/Shaders/DistanceFields.ps.slang").entryPoint(ShaderType::Pixel, "main");

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

    /*DepthStencilState::Desc dsDesc;
    dsDesc.setDepthFunc(ComparisonFunc::Less).setDepthEnabled(true);
    mpDepthTestDS = DepthStencilState::create(dsDesc);*/

    mComputeProgram = ComputeProgramWrapper::create();
    mComputeProgram->createProgram("Samples/DistanceFields/Shaders/SDFTexture.cs.slang");
    mComputeProgramFirstOrder = ComputeProgramWrapper::create();
    mComputeProgramFirstOrder->createProgram("Samples/DistanceFields/Shaders/FirstOrder.cs.slang");
    mComputeProgramA2 = ComputeProgramWrapper::create();
    mComputeProgramA2->createProgram("Samples/DistanceFields/Shaders/A2.cs.slang");


    {
        Sampler::Desc desc;
        desc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear);
        mpSampler = Sampler::create(desc);
    }

    sdfTextures = generateTexture(pRenderContext);

    mpVars["mSampler"] = mpSampler;

    setUpGui();

    tfit.getPoints(float3(point), boxsize, numberofpoints, 0.1, 0.5, float3(0, 0, 0), float3(0, 0, 1));
    tfit.fitTorus();

    clust.getPoints(1, float3(0), 2, 5);
}

bool DistanceFields::isOutOfBox(float3 pos)
{
    float b = (float)boundingbox;
    bool r = (pos.x > -b / 2.0 && pos.x < b / 2.0 &&
        pos.y > -b / 2.0 && pos.y < b / 2.0 &&
        pos.z > -b / 2.0 && pos.z < b / 2.0);

    return !r;

}

void DistanceFields::onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo)
{
    mpState->setFbo(pTargetFbo);
    // mpState->setDepthStencilState(mpDepthTestDS);
    fittingState->setFbo(pTargetFbo);
    //fittingState->setDepthStencilState(mpDepthTestDS);
    float4 clearColor = float4(0, 0, 0, 1);
    pRenderContext->clearFbo(mpState->getFbo().get(), clearColor, 1.0f, 0);
    pRenderContext->clearFbo(fittingState->getFbo().get(), clearColor, 1.0f, 0);

    ccontrol->update();

    if (func == 0) {
        if (retexture) {
            sdfTextures = generateTexture(pRenderContext);
            retexture = false;
            mProgram->addDefine("TEXORDER", std::to_string(textureOrder));
            mProgram->addDefine("SDF", std::to_string(sdf));
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
    if (func == 1) {
        //tóruszillesztés
        if (newpoints) {
            tfit.getPoints(float3(point), boxsize, numberofpoints, 0.1, 0.5, float3(0, 0, 0), float3(0, 0, 1));
            tfit.fitTorus();
            fittingProgram->addDefine("MODE", std::to_string(0));
            newpoints = false;
        }

        /*Buffer::SharedPtr pBuffer = Buffer::createStructured(fittingState->getProgram().get(), "points", numberofpoints * numberofpoints * numberofpoints);
        pBuffer->setBlob(toruspoints.data(), 0, numberofpoints*numberofpoints*numberofpoints * sizeof(float4));*/

        float4x4 m = glm::scale(float4x4(1.0), float3((float)boundingbox));
        fittingVars["vsCb"]["model"] = m;
        fittingVars["vsCb"]["viewproj"] = camera->getViewProjMatrix();
        fittingVars["vsCb"]["box"] = isbox;
        fittingVars["psCb"]["eye"] = camera->getPosition();
        fittingVars["psCb"]["center"] = camera->getTarget();
        fittingVars["psCb"]["up"] = camera->getUpVector();
        fittingVars["psCb"]["ar"] = camera->getAspectRatio();
        fittingVars["psCb"]["r"] = float(tfit.minimised_params(0));
        fittingVars["psCb"]["R"] = float(tfit.minimised_params(1));
        fittingVars["psCb"]["c"] = float3(tfit.minimised_params(2), tfit.minimised_params(3), tfit.minimised_params(4));
        fittingVars["psCb"]["d"] = float3(tfit.minimised_params(5), tfit.minimised_params(6), tfit.minimised_params(7));
        fittingVars["psCb"]["viewproj"] = camera->getViewProjMatrix();

        //fittingVars->setBuffer("points", pBuffer);

        fittingState->setVao(pVao);
        pRenderContext->draw(fittingState.get(), fittingVars.get(), 4, 0);
    }

    if (func == 2) {
        if (newpoints) {
            clust.getPoints(sphere, point, boxsize, 4);
            clust.clusterPoints();
            tfit.torus_points = clust.c1;
            tfit1.torus_points = clust.c2;
            tfit.fitTorus();
            tfit1.fitTorus();
            fittingProgram->addDefine("MODE", std::to_string(1));
            newpoints = false;
        }

        float4x4 m = glm::scale(float4x4(1.0), float3((float)boundingbox));
        fittingVars["vsCb"]["model"] = m;
        fittingVars["vsCb"]["viewproj"] = camera->getViewProjMatrix();
        fittingVars["vsCb"]["box"] = isbox;
        fittingVars["psCb"]["eye"] = camera->getPosition();
        fittingVars["psCb"]["center"] = camera->getTarget();
        fittingVars["psCb"]["up"] = camera->getUpVector();
        fittingVars["psCb"]["ar"] = camera->getAspectRatio();
        fittingVars["psCb"]["r"] = float(tfit.minimised_params(0));
        fittingVars["psCb"]["R"] = float(tfit.minimised_params(1));
        fittingVars["psCb"]["c"] = float3(tfit.minimised_params(2), tfit.minimised_params(3), tfit.minimised_params(4));
        fittingVars["psCb"]["d"] = float3(tfit.minimised_params(5), tfit.minimised_params(6), tfit.minimised_params(7));
        fittingVars["psCb"]["r1"] = float(tfit1.minimised_params(0));
        fittingVars["psCb"]["R1"] = float(tfit1.minimised_params(1));
        fittingVars["psCb"]["c1"] = float3(tfit1.minimised_params(2), tfit1.minimised_params(3), tfit1.minimised_params(4));
        fittingVars["psCb"]["d1"] = float3(tfit1.minimised_params(5), tfit1.minimised_params(6), tfit1.minimised_params(7));
        fittingVars["psCb"]["viewproj"] = camera->getViewProjMatrix();

        if (oneCluster) {
            fittingProgram->addDefine("MODE", std::to_string(2));
            fittingVars["psCb"]["cluster"] = clust.initial_footpoint;
        }
        else {
            fittingProgram->addDefine("MODE", std::to_string(1));
        }

        Buffer::SharedPtr pBuffer = Buffer::createStructured(fittingState->getProgram().get(), "points", 4 * 4 * 4 + 1);
        pBuffer->setBlob(clust.points_pos.data(), 0, (4 * 4 * 4 + 1) * sizeof(float4));
        fittingVars->setBuffer("points", pBuffer);

        fittingState->setVao(pVao);
        pRenderContext->draw(fittingState.get(), fittingVars.get(), 4, 0);
    }
}

void DistanceFields::onShutdown()
{
}

bool DistanceFields::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return ccontrol->onKeyEvent(keyEvent);
}

bool DistanceFields::onMouseEvent(const MouseEvent& mouseEvent)
{
    return ccontrol->onMouseEvent(mouseEvent);
}

void DistanceFields::onHotReload(HotReloadFlags reloaded)
{
}

void DistanceFields::onResizeSwapChain(uint32_t width, uint32_t height)
{
    camera->setAspectRatio((float)width / (float)height);
}



std::vector<Texture::SharedPtr> DistanceFields::generateTexture(RenderContext* pRenderContext) {

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

    comp.getProgram()->addDefine("SDF", std::to_string(sdf));

    comp["tex3D_uav1"].setUav(pTex1->getUAV(0));
    comp["csCb"]["sdf"] = sdf;
    comp["csCb"]["res"] = res;
    comp["csCb"]["boundingBox"] = (float)boundingbox;

    if (textureOrder == 1) {
        x0 = getPseudoInverse1(boundingbox, res);
        comp.allocateStructuredBuffer("x0", 108, x0.data(), sizeof(float) * 108);
    }

    if (textureOrder == 2) {

        comp["tex3D_uav2"].setUav(pTex2->getUAV(0));
        comp["tex3D_uav3"].setUav(pTex3->getUAV(0));
        x0 = getPseudoInverse2(boundingbox, res);
        comp.allocateStructuredBuffer("x0", 270, x0.data(), sizeof(float) * 270);
    }


    comp.runProgram(pRenderContext, res, res, res);

    std::vector<Texture::SharedPtr> textures;
    textures.push_back(pTex1);
    textures.push_back(pTex2);
    textures.push_back(pTex3);


    return textures;
}


std::vector<Texture::SharedPtr> DistanceFields::generateTorusTexture(RenderContext* pRenderContext) {

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


    for (int i = 0; i < db; i++) {
        for (int j = 0; j < db; j++) {
            for (int l = 0; l < db; l++) {

                float3 pos = float3(-boundingbox / 2.0);
                pos.x += boundingbox / (float)res * (i + 0.5);
                pos.y += boundingbox / (float)res * (j + 0.5);
                pos.z += boundingbox / (float)res * (l + 0.5);
                tfit.getPoints(pos, 0.6, 4, 0.4, 1, float3(0), float3(0, 1, 0));

                tfit.fitTorus();

                //pTex1->getUAV()[i][j][l] = float4(tfit.minimised_params(0), tfit.minimised_params(1), tfit.minimised_params(2), tfit.minimised_params(3));
            }
        }
    }


    std::vector<Texture::SharedPtr> textures;
    textures.push_back(pTex1);
    textures.push_back(pTex2);
    textures.push_back(pTex3);


    return textures;
}


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    DistanceFields::UniquePtr pRenderer = std::make_unique<DistanceFields>();
    SampleConfig config;
    config.windowDesc.title = "Falcor Project Template";
    config.windowDesc.width = 1280;
    config.windowDesc.height = 720;
    config.deviceDesc.enableVsync = true;
    config.windowDesc.resizableWindow = true;
    Sample::run(config, pRenderer);
    return 0;
}
