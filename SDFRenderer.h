#pragma once
#include "Falcor.h"
#include "ComputeProgramWrapper.h"
#include "Eigen/Dense"
#include <vector>
//#include "torusfitting.h"
#include "clustering.h"

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
    void setUpGui();

private:

    DepthStencilState::SharedPtr mpDepthTestDS = nullptr;

    GraphicsProgram::SharedPtr mProgram;
    GraphicsVars::SharedPtr mpVars;
    GraphicsState::SharedPtr mpState;
    Buffer::SharedPtr pVbo;
    Vao::SharedPtr pVao;

    Program::DefineList defList;

    Buffer::SharedPtr cubeVbo;
    Vao::SharedPtr cubeVao;
    bool isbox = false;

    struct Vertex
    {
        float3 pos;
        float3 col;
    };

    void initData();
    void initBox();
    void initCamera();

    Camera::SharedPtr camera;
    CameraController::SharedPtr ccontrol;

    float2 resolution = float2(1280, 720);


    uint32_t sdf = 1;

    ComputeProgramWrapper::SharedPtr mComputeProgram;
    ComputeProgramWrapper::SharedPtr mComputeProgramFirstOrder;
    ComputeProgramWrapper::SharedPtr mComputeProgramA2;
    std::vector<Texture::SharedPtr> sdfTextures;
    std::vector<Texture::SharedPtr> generateTexture(RenderContext* pRenderContext);
    uint32_t textureOrder = 0;

    int res = 100;
    bool retexture = false;
    int boundingbox = 2;
    bool isOutOfBox(float3 pos);

    uint32_t texturesize = 1;

    Sampler::SharedPtr mpSampler;

   std::vector<float> x0;


    //gui
    Gui::RadioButtonGroup bg;
    Gui::RadioButtonGroup texsize;
    Gui::RadioButtonGroup texorder;
    Gui::RadioButtonGroup function;
    Gui::RadioButtonGroup spheres;
    uint32_t func = 0;
    uint32_t sphere = 1;


    //tóruszillesztés
    void initFittingProgram();
    GraphicsProgram::SharedPtr fittingProgram;
    GraphicsVars::SharedPtr fittingVars;
    GraphicsState::SharedPtr fittingState;
    std::vector<float4> toruspoints;
    int numberofpoints = 3;
    bool newpoints = false;
    TorusFitting tfit;
    TorusFitting tfit1;
    Clustering clust;
    float pointx = 1;
    float pointy = 0;
    float pointz = 0;
    float boxsize = 2;

    DebugConsole console;

};
