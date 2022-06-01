//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "Ants.h"


//toreorganise
#include <fstream>
#include <iostream>

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace ImGui;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
#ifdef DXTK_AUDIO
    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }
#endif
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{

	m_input.Initialise(window);

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	//setup imgui.  its up here cos we need the window handle too
	//pulled from imgui directx11 example
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window);		//tie to our window
	ImGui_ImplDX11_Init(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());	//tie to directx

	m_fullscreenRect.left = 0;
	m_fullscreenRect.top = 0;
	m_fullscreenRect.right = 800;
	m_fullscreenRect.bottom = 600;

	m_CameraViewRect.left = 500;
	m_CameraViewRect.top = 0;
	m_CameraViewRect.right = 800;
	m_CameraViewRect.bottom = 240;

    //setup light
    m_Light.setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
    m_Light.setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
    m_Light.setPosition(0.0f, 10.0f, 0.0f);
    m_Light.setDirection(0.0f, -1.0f, 0.0f);

    //setup camera
    //m_Camera01.setPosition(Vector3(0.0f, 2.0f, -4.0f));
    m_Camera01.setPosition(Vector3(00.0f, 2.0f, -4.0f));
    m_Camera01.setRotation(Vector3(0.0f, 0.0f, 0.0f));	//orientation is -90 becuase zero will be looking up at the sky straight up. 

    doBloom = false; // Initialise var
    doMono = false;

    // setup room

    // centre block
    Cubes.push_back(Vector4(0.0f, 1.0f, 0.0f, 1.0f));

    // block hand
    Cubes.push_back(Vector4(0.0f, 1.0f, 0.0f, 4.0f));


    int limit = 1;
    int min;
    int max;
    for (int width = 0; width < 9; width++) {
        for (int breadth = 0; breadth < 9; breadth++) {

            // floor
            Cubes.push_back(Vector4(float(width - 4), 0.0f, float(breadth - 4), 4.0f));

            // using width as height var for wood walls
            if (width > 2) {


                Cubes.push_back(Vector4(-5, float(width - 4) + 2, float(breadth - 4), 2.0f));
                Cubes.push_back(Vector4(5, float(width - 4) + 2, float(breadth - 4), 2.0f));

                Cubes.push_back(Vector4(float(breadth - 4), float(width - 4) + 2, -5, 2.0f));
                Cubes.push_back(Vector4(float(breadth - 4), float(width - 4) + 2, 5, 2.0f));



}

            // roof
            if (limit <= 3) {
                min = 0 + limit;
                max = 8 - limit;
                if ((max >= width && width >= min) || (max >= breadth && breadth >= min)) {

                    Cubes.push_back(Vector4(float(width - 4), 6 + limit, float(breadth - 4), 3.0f));
                    Cubes.push_back(Vector4(float(4 - width), 6 + limit, float(4 - breadth), 3.0f));
                }
                if ((max + 1 >= width && width >= min - 1) || (max + 1 >= breadth && breadth >= min - 1)) {


                    Cubes.push_back(Vector4(float(breadth - 4), 6 + limit, float(width - 4), 3.0f));
                    Cubes.push_back(Vector4(float(4 - breadth), 6 + limit, float(4 - width), 3.0f));

                }


            }
        }
        // pillars
        if (limit <= 6) {
            Cubes.push_back(Vector4(-4, limit, 4, 4.0f));
            Cubes.push_back(Vector4(-4, limit, -4, 4.0f));
            Cubes.push_back(Vector4(4, limit, 4, 4.0f));
            Cubes.push_back(Vector4(4, limit, -4, 4.0f));
        }

        // bench for glass jars
        if (width != 0 && width != 8) {
            Cubes.push_back(Vector4(-4, 1, width - 4, 3.0f));
            Cubes.push_back(Vector4(-4, 2, width - 4, 5.0f));

            int amount = 5;

            // flies in fly jars
            for (int i = 0; i < amount; i++)
                ants.push_back(Ants(Cubes.back(), 12, Cubes.size() - 1));

        }

        limit++;
    }


    ogCubes = Cubes.size();
    ogAnts = ants.size();

	
#ifdef DXTK_AUDIO
    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);

    m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

    m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"MusicMono_adpcm.wav");
    m_effect1 = m_soundEffect->CreateInstance();
    m_effect2 = m_waveBank->CreateInstance(10);

    m_effect1->Play(true);
    m_effect2->Play();
#endif
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
	//take in input
	m_input.Update();								//update the hardware
	m_gameInputCommands = m_input.getGameInput();	//retrieve the input for our game
	
	//Update all game objects
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

	//Render all game content. 
    Render();

#ifdef DXTK_AUDIO
    // Only update audio engine once per frame
    if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
#endif

	
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{	
	//this is hacky,  i dont like this here.  
	auto device = m_deviceResources->GetD3DDevice();

    float delta = float(timer.GetElapsedSeconds()); // Delta-time
    int speedScale = 40; // Const to multiply all movements by

    //note that currently.  Delta-time is not considered in the game object movement. 
    Vector3 preMove = m_Camera01.getPosition();

    colBox = Vector4(0, 0, 0, 0);
    if (m_gameInputCommands.left)
    {
        Vector3 rotation = m_Camera01.getRotation();
        rotation.y += (m_Camera01.getRotationSpeed() * delta * speedScale);
        m_Camera01.setRotation(rotation);
    }
    if (m_gameInputCommands.right)
    {
        Vector3 rotation = m_Camera01.getRotation();
        rotation.y -= (m_Camera01.getRotationSpeed() * delta * speedScale);
        m_Camera01.setRotation(rotation);
    }
    if (m_gameInputCommands.forward)
    {
        Vector3 position = m_Camera01.getPosition(); //get the position
        position += (m_Camera01.getForward() * m_Camera01.getMoveSpeed() * delta * speedScale * 0.25); //add the forward vector
        m_Camera01.setPosition(position);
    }
    if (m_gameInputCommands.back)
    {
        Vector3 position = m_Camera01.getPosition(); //get the position
        position -= (m_Camera01.getForward() * m_Camera01.getMoveSpeed() * delta * speedScale * 0.25); //add the forward vector
        m_Camera01.setPosition(position);
    }
    if (m_gameInputCommands.strLeft)
    {
        Vector3 position = m_Camera01.getPosition(); //get the position
        position -= (m_Camera01.getRight() * m_Camera01.getMoveSpeed() * delta * speedScale * 0.25); //add the forward vector
        m_Camera01.setPosition(position);
    }
    if (m_gameInputCommands.strRight)
    {
        Vector3 position = m_Camera01.getPosition(); //get the position
        position += (m_Camera01.getRight() * m_Camera01.getMoveSpeed() * delta * speedScale * 0.25); //add the forward vector
        m_Camera01.setPosition(position);
    }

    


    if (m_gameInputCommands.strUp)
    {
        //bounce = true;
        Vector3 position = m_Camera01.getPosition();
        position.y += m_Camera01.getMoveSpeed() * delta * speedScale * 0.5; 
        m_Camera01.setPosition(position);
    }

    if (m_gameInputCommands.strDown)
    {
        //bounce = true;
        Vector3 position = m_Camera01.getPosition();
        position.y -= m_Camera01.getMoveSpeed() * delta * speedScale * 0.5;
        m_Camera01.setPosition(position);
    }

    /*
    Vector3 position = m_Camera01.getPosition();
    float yVal = 0.0f;
    if (bounce) {

        bTimer += delta;
        if (bTimer < 0.5f) {
            position.y += m_Camera01.getMoveSpeed() * delta * speedScale * 0.5;
            m_Camera01.setPosition(position);
        }
        else {
            bounce = false;
            bTimer = 0.0f;
        }
    }

    else {
        if (colBox == Vector4(0, 0, 0, 0) || colBox.y+2 < m_Camera01.getPosition().y ) {
            //yVal = m_Camera01.getMoveSpeed() * delta * speedScale * 0.5;
            //position.y -= yVal;
            //m_Camera01.setPosition(position);
        }
    }
   

    if (m_gameInputCommands.strDown)
    {
        crouching = true;
        preCrouchY = m_Camera01.getPosition().y;
    }
    else
        crouching = false;

    if (crouching) {
        Vector3 position = m_Camera01.getPosition(); //get the position
        //position.y = preCrouchY - 0.5f;
        m_Camera01.setPosition(position);
    }
    */

    m_Camera01.setPosition(Collision(Cubes, m_Camera01.getPosition(), preMove));
    
    if (m_gameInputCommands.pause)
    {
        pause = !pause;
    }

    if (m_gameInputCommands.reset)
    {

        m_Camera01.setPosition(Vector3(0,2,-4));
        m_Camera01.setRotation(Vector3(0.0f, 0.0f, 0.0f));
        doMono = false;
        doBloom = false;

        std::vector<Ants> temp = ants;
        ants.clear();
        for (int i = 0; i < ogAnts; i++) {
            if (temp[i].getCubeType() == 5) {
                ants.push_back(temp[i]);
            }
        }
        temp.clear();
        eatenCubes.clear();

        std::vector<DirectX::SimpleMath::Vector4> temp2 = Cubes;
        Cubes.clear();
        for (int i = 0; i < ogCubes; i++) {
            Cubes.push_back(temp2[i]);
        }
    }

    if (m_gameInputCommands.toggleGlass) {
        showGlass = !showGlass;
    }

    if (m_gameInputCommands.toggleSand) {
        showSand = !showSand;
    }

    if (m_gameInputCommands.addAnt) {
        makeAnt = true;
    }
    
    if (m_gameInputCommands.toggleEaten) {
        toggleEaten = !toggleEaten;
    }

    if (m_gameInputCommands.toggleHand) {
        blockHandID++;
        if (blockHandID > 5)
            blockHandID = 2;
    }

    if (m_gameInputCommands.placeBlock) {
        float x = round(Cubes[1].x);
        float y = round(Cubes[1].y);
        float z = round(Cubes[1].z);

        bool allow = true;
        for (int i = 0; i < Cubes.size(); i++) {
            if (Cubes[i].x == x && Cubes[i].y == y && Cubes[i].z == z && i != 1)
                allow = false;
        }

        if (allow)
            Cubes.push_back(Vector4(x, y, z, blockHandID));

        if (blockHandID == 5) {
            int amount = 5;

            // flies in fly jars
            for (int i = 0; i < amount; i++)
                ants.push_back(Ants(Cubes.back(), 12, Cubes.size() - 1));

        }
    }

    if (m_gameInputCommands.deleteBlock) {
        float x = round(Cubes[1].x);
        float y = round(Cubes[1].y);
        float z = round(Cubes[1].z);

        bool allow = false;
        int marker;
        Vector4 temp;
        for (int i = 0; i < Cubes.size(); i++) {
            if (Cubes[i].x == x && Cubes[i].y == y && Cubes[i].z == z && i != 1 && i >= ogCubes)
            {
                allow = true;
                marker = i;
                temp = Cubes[i];
            }
        }

        if (allow)
        {
            std::vector<DirectX::SimpleMath::Vector4> temp2 = Cubes;
            Cubes.clear();
            for (int i = 0; i < temp2.size(); i++) {
                if (temp2[i] != temp)
                    Cubes.push_back(temp2[i]);
            }
        }
    }

    if (m_gameInputCommands.monochrome) {
        doMono = !doMono;
    }
    
    if (m_gameInputCommands.bloom) {
        doBloom = !doBloom;
    }




    //update blockhand position;
    auto temp = m_Camera01.getPosition();
    auto fw = m_Camera01.getForward();
    auto x = round(temp.x + (fw.x * 3));
    auto y = round(temp.y - 1 + fw.y);
    auto z = round(temp.z + (fw.z * 3));
    Cubes[1] = Vector4(x, y, z, Cubes[1].w);



    // update ants
    if (pause == false) {
        for (int i = 0; i < ants.size(); i++) {
            if (ants[i].isDead() == false) {

                // shorten lifespan
                ants[i].UpdateLifeSpan(1.0f * delta);

                // calc new position
                Vector3 oldPos = ants[i].GetPosition();
                Vector3 digVector = ants[i].GetDigVector();
                float digSpeed = ants[i].GetDigSpeed();
                Vector3 newPos;

                newPos = oldPos + (digVector * digSpeed * delta);
                ants[i].SetPosition(newPos);

                // if ant is stuck
                if (oldPos == ants[i].GetPosition())
                    ants[i].stuck(delta);
                else if (ants[i].isStuck() == true)
                    ants[i].resetStuck();
            }

        }

    }


	m_Camera01.Update();	//camera update.
	
	m_view = m_Camera01.getCameraMatrix();
	m_world = Matrix::Identity;

	/*create our UI*/
	SetupGUI();

#ifdef DXTK_AUDIO
    m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
    if (m_audioTimerAcc < 0)
    {
        if (m_retryDefault)
        {
            m_retryDefault = false;
            if (m_audEngine->Reset())
            {
                // Restart looping audio
                m_effect1->Play(true);
            }
        }
        else
        {
            m_audioTimerAcc = 4.f;

            m_waveBank->Play(m_audioEvent++);

            if (m_audioEvent >= 11)
                m_audioEvent = 0;
        }
    }
#endif

  
	if (m_input.Quit())
	{
		ExitGame();
	}
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{	
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

    // Draw Text to the screen
    m_sprites->Begin();
		m_font->DrawString(m_sprites.get(), L"Procedural Methods", XMFLOAT2(10, 10), Colors::Yellow);
    m_sprites->End();

    if (doBloom && !doMono) {
        m_RenderTexture1->setRenderTarget(context);
        // Clear the render to texture.
        m_RenderTexture1->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 1.0f);

    }
    else if (!doBloom && doMono) {
        m_RenderTexture1->setRenderTarget(context);
        // Clear the render to texture.
        m_RenderTexture1->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 1.0f);

    }

	//Set Rendering states. 
	//context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetBlendState(m_states->AlphaBlend(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	context->RSSetState(m_states->CullClockwise());

    int cubeDimension = 12;
    //float off = -4.5f;
    float off = -5.5f;

    // add ant to block 1
    if (makeAnt) {
        // adds ants to cube 0
        Ants temp = Ants(Cubes[0], cubeDimension, 0);
        ants.push_back(temp);
        makeAnt = false;
    }

    for (auto i : ants) {

        // renders non dead ants
        if (i.isDead() == false) {
            m_world = SimpleMath::Matrix::Identity;
            SimpleMath::Matrix newPosition = SimpleMath::Matrix::CreateTranslation(i.GetPosition().x + off, i.GetPosition().y - off, i.GetPosition().z + off);
            SimpleMath::Matrix cubePosition = SimpleMath::Matrix::CreateTranslation(i.GetCubePosition().x, i.GetCubePosition().y - 1, i.GetCubePosition().z);
            SimpleMath::Matrix newScale = SimpleMath::Matrix::CreateScale(0.5f);

            // shrinks ant object size to be smaller than one block
            //newPosition += cubePosition;
            m_world *= newScale;

            // sets ants position to block pos
            m_world *= newPosition;

            // shrinks ants to fit inside cube
            newScale = SimpleMath::Matrix::CreateScale(0.0833f);
            m_world *= newScale;

            m_world *= cubePosition;


            m_BasicShaderPair.EnableShader(context);
            m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, ant.Get());
            block.Render(context);


            // calc block eating
            int cubeID = i.getCubeID();
            Vector3 antPos = i.GetPosition();
            Vector3 cubePos = i.GetCubePosition();

            // very basic collision, using rounding, determine which block the ant is mostly in
            antPos.x = round(antPos.x);
            antPos.y = round(antPos.y) - 1;
            antPos.z = round(antPos.z);

            Vector4 checkBlock = Vector4(antPos.z, antPos.y, antPos.x, cubeID);
            bool alreadyEaten = false;
            for (int i = 0; i < eatenCubes.size(); i++) {
                if (checkBlock == eatenCubes[i])
                    alreadyEaten = true;
            }
            if (alreadyEaten == false)
                eatenCubes.push_back(checkBlock);
        }
    }

    //float off = 0;
    float testID = 0.0f;
    // for cubes

    for (int i = 0; i < Cubes.size(); i++) {

        // for glass block

        m_world = SimpleMath::Matrix::Identity;
        //SimpleMath::Matrix newPosition = SimpleMath::Matrix::CreateTranslation(Cubes[i].x +off, Cubes[i].y - off, Cubes[i].z + off);
        SimpleMath::Matrix newPosition = SimpleMath::Matrix::CreateTranslation(Cubes[i].x, Cubes[i].y, Cubes[i].z);
        SimpleMath::Matrix newScale = SimpleMath::Matrix::CreateScale(0.5f);


        // block hand
        if (i == 1) {

        }

        else if (Cubes[i].w == 1.0f) {
            m_world = m_world * newPosition;

            if (showGlass) {
                m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, glass.Get());
                block.Render(context);
            }

            else {
                m_world = m_world * newPosition;

                // for sand blocks;
                testID = i;
                for (int height = 1; height < cubeDimension - 1; height++) {
                    for (int breadth = 1; breadth < cubeDimension - 1; breadth++) {
                        for (int width = 1; width < cubeDimension - 1; width++) {

                            m_world = SimpleMath::Matrix::Identity;
                            SimpleMath::Matrix newPosition = SimpleMath::Matrix::CreateTranslation(Cubes[i].x + width + off, Cubes[i].y + height - off, Cubes[i].z + breadth + off);
                            SimpleMath::Matrix newScale = SimpleMath::Matrix::CreateScale(0.0833f);
                            //SimpleMath::Matrix newScale = SimpleMath::Matrix::CreateScale(0.06f);
                            m_world = m_world * newPosition;
                            m_world = m_world * newScale;
                            m_BasicShaderPair.EnableShader(context);

                            m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, sand.Get());

                            // check if sand block has been eaten
                            bool eaten = false;
                            for (int e = 0; e < eatenCubes.size(); e++) {

                                // if eaten block is part of this cube
                                if (eatenCubes[e].w == testID) {
                                    Vector3 testVector = Vector3(eatenCubes[e].x, eatenCubes[e].y, eatenCubes[e].z);
                                    if (testVector == Vector3(float(breadth), float(height), float(width)))
                                        //if (testVector == Vector3(1,8,8))
                                    {
                                        eaten = true;
                                        m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, glass.Get());

                                    }

                                }
                            }




                            // if showing any sand
                            if (showSand) {
                                // toggleEaten: user toggles if eaten/non eaten blocks get rendered
                                // false, display uneaten, true display eaten.

                                if (toggleEaten) {
                                    if (eaten)
                                        block.Render(context);
                                }
                                else {
                                    if (!eaten)
                                        block.Render(context);
                                }

                            }
                        }
                    }
                }

            }

        }
        else if (Cubes[i].w == 2.0f) {
            m_world = m_world * newPosition;

            m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, wood.Get());
            block.Render(context);
        }
        else if (Cubes[i].w == 3.0f) {
            m_world = m_world * newPosition;

            m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, stone.Get());
            block.Render(context);
        }
        else if (Cubes[i].w == 4.0f) {
            m_world = m_world * newPosition;

            m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, log.Get());
            block.Render(context);
        }
        else if (Cubes[i].w == 6.0f) {
            m_world = m_world * newPosition;

            m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, cb.Get());
            block.Render(context);
        }
    }


    // glass
    for (int i = 0; i < Cubes.size(); i++) {
        m_world = SimpleMath::Matrix::Identity;
        //SimpleMath::Matrix newPosition = SimpleMath::Matrix::CreateTranslation(Cubes[i].x +off, Cubes[i].y - off, Cubes[i].z + off);
        SimpleMath::Matrix newPosition = SimpleMath::Matrix::CreateTranslation(Cubes[i].x, Cubes[i].y, Cubes[i].z);
        //SimpleMath::Matrix newScale = SimpleMath::Matrix::CreateScale(0.0833f);
        m_world = m_world * newPosition;
        //m_world = m_world * newScale;
        m_BasicShaderPair.EnableShader(context);
        if (Cubes[i].w == 5.0f) {
            m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, glass.Get());
            block.Render(context);
        }

    }

    m_world = SimpleMath::Matrix::Identity;
    //SimpleMath::Matrix newPosition = SimpleMath::Matrix::CreateTranslation(Cubes[i].x +off, Cubes[i].y - off, Cubes[i].z + off);
    SimpleMath::Matrix newPosition = SimpleMath::Matrix::CreateTranslation(Cubes[1].x, Cubes[1].y, Cubes[1].z);
    SimpleMath::Matrix newScale = SimpleMath::Matrix::CreateScale(0.5f);

    SimpleMath::Matrix rotate = SimpleMath::Matrix::CreateRotationY(m_Camera01.getRotation().y * (3.14159 / 180));
    //m_world *= rotate;

    m_world = m_world * newScale;
    m_world = m_world * newPosition;

    m_BasicShaderPair.EnableShader(context);


    switch (blockHandID) {
    case 2:
        m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, wood.Get());
        break;

    case 3:
        m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, stone.Get());
        break;

    case 4:
        m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, log.Get());
        break;

    case 5:
        m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, glass.Get());
        break;

    }

    block.Render(context);
    
    if (doBloom && !doMono) {
        context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
        m_postProcess->SetSourceTexture(m_RenderTexture1->getShaderResourceView());
        // Apply post process effect
        m_postProcess->SetBloomBlurParameters(true, 4.0f, 2.0f);
        m_postProcess->SetEffect(BasicPostProcess::BloomBlur);

        m_postProcess->Process(context);

    }
    else if (!doBloom && doMono) {
        context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
        m_postProcess1->SetSourceTexture(m_RenderTexture1->getShaderResourceView());
        // Apply post process effect
        m_postProcess1->SetEffect(BasicPostProcess::Monochrome);

        m_postProcess1->Process(context);

    }
    else {
        doMono = false;
        doBloom = false;
    }

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());



    // Show the new frame.
    m_deviceResources->Present();
    
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

#pragma endregion
DirectX::SimpleMath::Vector3 Game::Collision(std::vector<DirectX::SimpleMath::Vector4> cubes, DirectX::SimpleMath::Vector3 player, DirectX::SimpleMath::Vector3 old) {

    // player has two cubes, feet and head
    Vector3 head = player;
    Vector3 feet = player;
    feet.y -= 1.0f;

    float centre = 0.5f;
    
    float intX = 0;
    float intY = 0;
    float intZ = 0;
    
    float intFX = 0;
    float intFY = 0;
    float intFZ = 0;

    float dX = 0;
    float dY = 0;
    float dZ = 0;
    
    float feetX = 0;
    float feetY = 0;
    float feetZ = 0;
    
    for (int i = 0; i < cubes.size(); i++) {
        /*
        int target = 0; // 1=x,2=y,3=z
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        bool isX = false;
        bool isY = false;
        bool isZ = false;

        //check each axis
        if ((cubes[i].x >= head.x && cubes[i].x - 1 <= head.x) || (cubes[i].x >= feet.x && cubes[i].x - 1 <= feet.x)) {
            x = cubes[i].x - head.x;
            isX = true;
        }
        if ((cubes[i].y >= head.y && cubes[i].y - 1 <= head.y) || (cubes[i].y >= feet.y && cubes[i].y - 1 <= feet.y)) {
            y = cubes[i].y - head.y;
            isY = true;
        }
        if ((cubes[i].z >= head.z && cubes[i].z - 1 <= head.z) || (cubes[i].z >= feet.z && cubes[i].z - 1 <= feet.z)) {
            z = cubes[i].z - head.z;
            isZ = true;
        }

     

        //blocks only intersect if all 3 axis intersect
        if (isX && isY && isZ) {
            inside.push_back(cubes[i]);

            return true;
        }*/
        dX = (head.x) - cubes[i].x;
        dY = (head.y) - cubes[i].y;
        dZ = (head.z) - cubes[i].z;

        feetX = (feet.x) - cubes[i].x;
        feetY = (feet.y) - cubes[i].y;
        feetZ = (feet.z) - cubes[i].z;

        intX = abs(dX) - (2 * centre);
        intY = abs(dY) - (2 * centre);
        intZ = abs(dZ) - (2 * centre);

        intFX = abs(feetX) - (2 * centre);
        intFY = abs(feetY) - (2 * centre);
        intFZ = abs(feetZ) - (2 * centre);

        // head
        if (intX < 0 && intZ < 0 && intY < 0) {

                // find largest
                if (intX > intY && intX > intZ) { head.x = old.x; }
                else if (intY > intX && intY > intZ) { head.y = old.y; }
                else if (intZ > intX && intZ > intY) { head.z = old.z; }

                colBox = cubes[i];

            
        }
        // feet
        if (intFX < 0 && intFZ < 0 && intFY < 0) {
                // find largest
                if (intFX > intFY && intFX > intFZ) { head.x = old.x; }
                else if (intFY > intFX && intFY > intFZ) { head.y = old.y; }
                else if (intFZ > intFX && intZ > intFY) { head.z = old.z; }

                colBox = cubes[i];
            }
        }
    

    return head;



}




#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
#ifdef DXTK_AUDIO
    m_audEngine->Suspend();
#endif
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

#ifdef DXTK_AUDIO
    m_audEngine->Resume();
#endif
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

#ifdef DXTK_AUDIO
void Game::NewAudioDevice()
{
    if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}
#endif

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);
    m_fxFactory = std::make_unique<EffectFactory>(device);
    m_sprites = std::make_unique<SpriteBatch>(context);
    m_font = std::make_unique<SpriteFont>(device, L"SegoeUI_18.spritefont");
	m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);
    m_postProcess = std::make_unique<BasicPostProcess>(device);
    m_postProcess1 = std::make_unique<BasicPostProcess>(device);

    // floor
    m_BasicModel.InitializeBox(device, 1.0f, 1.0f, 1.0f);	//box includes dimensions
    block.InitializeBox(device, 1.0f, 1.0f, 1.0f);


	//load and set up our Vertex and Pixel Shaders
	m_BasicShaderPair.InitStandard(device, L"light_vs.cso", L"light_ps.cso");

    //load Textures
    CreateDDSTextureFromFile(device, L"sand.dds", nullptr, sand.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"glassTexture.dds", nullptr, glass.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"black.dds", nullptr, ant.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"block.dds", nullptr, wood.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"stone.dds", nullptr, stone.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"log.dds", nullptr, log.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"cb.dds", nullptr, cb.ReleaseAndGetAddressOf());


	//Initialise Render to texture
	m_FirstRenderPass = new RenderTexture(device, 800, 600, 1, 2);	//for our rendering, We dont use the last two properties. but.  they cant be zero and they cant be the same. 
    m_RenderTexture = new RenderTexture(device, 1920, 1080, 1, 2); //blur
    m_RenderTexture1 = new RenderTexture(device, 1920, 1080, 1, 2); //bloom

}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    float aspectRatio = float(size.right) / float(size.bottom);
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // This sample makes use of a right-handed coordinate system using row-major matrices.
    m_projection = Matrix::CreatePerspectiveFieldOfView(
        fovAngleY,
        aspectRatio,
        0.01f,
        100.0f
    );
}

void Game::SetupGUI()
{

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Camera Pos");
    ImGui::Text("x: %f", m_Camera01.getPosition().x);
    ImGui::Text("y: %f", m_Camera01.getPosition().y);
    ImGui::Text("z: %f", m_Camera01.getPosition().z);


    ImGui::Text("");
    ImGui::Text("Box Pos");
    ImGui::Text("x: %f", colBox.x);
    ImGui::Text("y: %f", colBox.y);
    ImGui::Text("z: %f", colBox.z);

		//ImGui::SliderFloat("Wave Amplitude",	1, 0.0f, 10.0f);
		//ImGui::SliderFloat("Wavelength",		, 0.0f, 1.0f);
	ImGui::End();
}


void Game::OnDeviceLost()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_font.reset();
	m_batch.reset();
	m_testmodel.reset();
    m_batchInputLayout.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}
#pragma endregion
