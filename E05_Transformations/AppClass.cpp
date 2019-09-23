#include "AppClass.h"
void Application::InitVariables(void)
{

	//init the mesh
	
	//generate cubes based on map
	for (int row = 0; row < 8; row++)
	{
		for (int column = 0; column < 11; column++)
		{
			if (*invaderMap[row, column] == 1)
			{
				meshes.push_back(MyMesh());
				meshes[currentCube].GenerateCube(0.5f, C_BLACK);
				meshLocations.push_back(glm::translate(IDENTITY_M4, vector3(row * 1.0f, column * 1.0f, 3.0f)));
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
	

	for (int i = 0; i < meshes.size(); i++)
	{
		//get locatoin of the current cube and render
		matrix4 m4Scale = glm::scale(IDENTITY_M4, vector3(1.0f, 1.0f, 1.0f));

		//matrix4 m4Model = m4Translate * m4Scale;
		matrix4 m4Model = m4Scale * meshLocations[i];

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