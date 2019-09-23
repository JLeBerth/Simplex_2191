#include "AppClass.h"
void Application::InitVariables(void)
{

	//init the map
	int invaderMap[8][11] =
	{
		{ 0,0,1,0,0,0,0,0,1,0,0 },
		{ 0,0,0,1,0,0,0,1,0,0,0 },
		{ 0,0,1,1,1,1,1,1,1,0,0 },
		{ 0,1,1,0,1,1,1,0,1,1,0 },
		{ 0,0,1,1,1,1,1,1,1,0,0 },
		{ 1,0,1,1,1,1,1,1,1,0,1 },
		{ 1,0,1,0,0,0,0,0,1,0,1 },
		{ 0,0,0,1,1,0,1,1,0,0,0 }
	};

	//generate cubes based on map
	for (int row = 0; row < 11; row++)
	{
		for (int column = 0; column < 11; column++)
		{
			if (invaderMap[row][column] == 1)
			{
				meshes.push_back(MyMesh());
				meshes[currentCube].GenerateCube(0.5f, C_BLACK);
				meshLocations.push_back(glm::vec3(column * 0.5f, (row * -0.5f)+5.0f, 3.0f));
				currentCube++;
				
			}
		}
	}

}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	matrix4 m4View = m_pCameraMngr->GetViewMatrix();
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();
	clock++;
	
	//figure out whether to move left or right
	if (clock > 100)
	{
		moveleft = !moveleft;
		clock = 0;
	}
	

	for (int i = 0; i < meshes.size(); i++)
	{
		//get locatoin of the current cube and render
		matrix4 m4Scale = glm::scale(IDENTITY_M4, vector3(1.0f, 1.0f, 1.0f));

		//move
		if (moveleft)
		{
			meshLocations[i] -= glm::vec3(moveBy, 0, 0);
		}
		else
		{
			meshLocations[i] += glm::vec3(moveBy, 0, 0);
		}

		matrix4 m4Translate = glm::translate(IDENTITY_M4, meshLocations[i]);
		//matrix4 m4Model = m4Translate * m4Scale;
		matrix4 m4Model = m4Scale * m4Translate;

		meshes[i].Render(m4Projection, m4View, m4Model);
	}
	
	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	SafeDelete(m_pMesh);

	//release GUI
	ShutdownGUI();
}