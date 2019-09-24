#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code

	//grab values and points we need for cone
	float radiansSeperation = (360.0f / a_nSubdivisions) * (PI / 180.0f); //number of radians between each point
	glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f); //the center point of the circle
	glm::vec3 prevPoint = glm::vec3(a_fRadius, 0.0f, 0.0f); //the connecting point between the last and the next triangle, for the first triangle starts 1 radius out
	glm::vec3 currentPoint = glm::vec3(0.0f, 0.0f, 0.0f); //the point x degrees along the circles circumference to form the next triangle
	glm::vec3 conePoint = glm::vec3(0.0f, a_fHeight, 0.0f); //the tip of the cone

	//add points to cone
	//creates a circle at the bottom, then for each triangle around the base makes the sides, goes in both directions so they ban be viewed from both sides
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		currentPoint = glm::vec3((a_fRadius * float(cos(radiansSeperation * (i + 1)))), 0.0f, (a_fRadius * float(sin(radiansSeperation * (i + 1)))));
		AddTri(center, prevPoint, currentPoint);
		AddTri(prevPoint, currentPoint, conePoint);
		AddTri(currentPoint, prevPoint, conePoint);
		prevPoint = currentPoint; //sets previous point to current point for next triangle
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code

	//grab values and points we need for cylinder
	float radiansSeperation = (360.0f / a_nSubdivisions) * (PI / 180.0f); //number of radians between each point
	glm::vec3 center = glm::vec3(0.0f, -a_fHeight/2.0f, 0.0f); //the center point of the circle
	glm::vec3 centerTop = glm::vec3(0.0f, a_fHeight/2.0f, 0.0f); //the center point of the circle top circle
	glm::vec3 prevPoint = glm::vec3(a_fRadius, -a_fHeight/2.0f, 0.0f); //the connecting point between the last and the next triangle, for the first triangle starts 1 radius out
	glm::vec3 currentPoint = glm::vec3(0.0f, 0.0f, 0.0f); //the point x degrees along the circles circumference to form the next triangle
	glm::vec3 prevPointTop = glm::vec3(a_fRadius, a_fHeight/2.0f, 0.0f); //the connecting point between the last and the next triangle, for the first triangle starts 1 radius out top circle
	glm::vec3 currentPointTop = glm::vec3(0.0f, 0.0f, 0.0f); //the point x degrees along the circles circumference to form the next triangle top circle

	//add points to cylinder
	//create two circles top facing and bottom facing, then draw quads connecting them
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		//find new points
		currentPoint = glm::vec3((a_fRadius * float(cos(radiansSeperation * (i + 1)))), -a_fHeight/2.0f, (a_fRadius * float(sin(radiansSeperation * (i + 1)))));
		currentPointTop = glm::vec3((a_fRadius * float(cos(radiansSeperation * (i + 1)))), a_fHeight/2.0f, (a_fRadius * float(sin(radiansSeperation * (i + 1)))));

		//create circle portions
		AddTri(center, prevPoint, currentPoint);
		AddTri(centerTop, currentPointTop, prevPointTop);

		//create outer faces
		AddQuad(prevPoint, currentPoint, prevPointTop, currentPointTop);
		AddQuad(prevPointTop, currentPointTop, prevPoint, currentPoint);

		prevPoint = currentPoint; //sets previous point to current point for next triangle
		prevPointTop = currentPointTop; //sets previous point to current point for next triangle top circle
	}

	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	//grab values and points we need for tube
	float radiansSeperation = (360.0f / a_nSubdivisions) * (PI / 180.0f); //number of radians between each point
	glm::vec3 prevPoint = glm::vec3(a_fInnerRadius, -a_fHeight / 2.0f, 0.0f); //the connecting point between the last and the next triangle, for the first triangle starts 1 radius out
	glm::vec3 currentPoint = glm::vec3(0.0f, 0.0f, 0.0f); //the point x degrees along the circles circumference to form the next triangle
	glm::vec3 prevPointTop = glm::vec3(a_fInnerRadius, a_fHeight / 2.0f, 0.0f); //the connecting point between the last and the next triangle, for the first triangle starts 1 radius out top circle
	glm::vec3 currentPointTop = glm::vec3(0.0f, 0.0f, 0.0f); //the point x degrees along the circles circumference to form the next triangle top circle

	glm::vec3 prevPointOuter = glm::vec3(a_fOuterRadius, -a_fHeight / 2.0f, 0.0f); //the connecting point between the last and the next triangle, for the first triangle starts 1 radius out
	glm::vec3 currentPointOuter = glm::vec3(0.0f, 0.0f, 0.0f); //the point x degrees along the circles circumference to form the next triangle
	glm::vec3 prevPointTopOuter = glm::vec3(a_fOuterRadius, a_fHeight / 2.0f, 0.0f); //the connecting point between the last and the next triangle, for the first triangle starts 1 radius out top circle
	glm::vec3 currentPointTopOuter = glm::vec3(0.0f, 0.0f, 0.0f); //the point x degrees along the circles circumference to form the next triangle top circle

	//add points to tube
	//create two circles top facing and bottom facing, then draw quads connecting them twice
	//afterwards create quads connecting the seperate cylinders
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		//create inner cylinder faces
		currentPoint = glm::vec3((a_fInnerRadius * float(cos(radiansSeperation * (i + 1)))), -a_fHeight / 2.0f, (a_fInnerRadius * float(sin(radiansSeperation * (i + 1)))));
		currentPointTop = glm::vec3((a_fInnerRadius * float(cos(radiansSeperation * (i + 1)))), a_fHeight / 2.0f, (a_fInnerRadius * float(sin(radiansSeperation * (i + 1)))));
		AddQuad(prevPoint, currentPoint, prevPointTop, currentPointTop);
		AddQuad(prevPointTop, currentPointTop, prevPoint, currentPoint);

		//create outer cylinder faces
		currentPointOuter = glm::vec3((a_fOuterRadius * float(cos(radiansSeperation * (i + 1)))), -a_fHeight / 2.0f, (a_fOuterRadius * float(sin(radiansSeperation * (i + 1)))));
		currentPointTopOuter = glm::vec3((a_fOuterRadius * float(cos(radiansSeperation * (i + 1)))), a_fHeight / 2.0f, (a_fOuterRadius * float(sin(radiansSeperation * (i + 1)))));
		AddQuad(prevPointOuter, currentPointOuter, prevPointTopOuter, currentPointTopOuter);
		AddQuad(prevPointTopOuter, currentPointTopOuter, prevPointOuter, currentPointOuter);

		//connect cylinders to make tube
		AddQuad(prevPointOuter,currentPointOuter,prevPoint,currentPoint);
		AddQuad(prevPointTop,currentPointTop, prevPointTopOuter, currentPointTopOuter);
		prevPointOuter = currentPointOuter; //sets previous point to current point for next triangle outer circle
		prevPointTopOuter = currentPointTopOuter; //sets previous point to current point for next triangle top outer circle
		prevPoint = currentPoint; //sets previous point to current point for next triangle
		prevPointTop = currentPointTop; //sets previous point to current point for next triangle top circle
	}

	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// Replace this with your code
	GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();

	// Replace this with your code
	//grab values and points we need for sphere
	float radiansSeperation = (360.0f / a_nSubdivisions) * (PI / 180.0f); //number of radians between each point
	float radiansSeperationHalf = (180 / a_nSubdivisions) * (PI / 180.0f); //radians between points along one half of the circle such that we can divide vertically
	float baseRadian = (-180.0f) * (PI / 180.0f); //radian measurement of bottom point to move up from

	glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f); //the center point of the circle
	glm::vec3 bottom = glm::vec3(0.0f, -a_fRadius, 0.0f); //the bottom of the sphere
	glm::vec3 top = glm::vec3(0.0f, a_fRadius, 0.0f); //the top of the sphere
	glm::vec3 prevPoint = glm::vec3(a_fRadius, 0.0f, 0.0f); //the connecting point between the last and the next triangle, for the first triangle starts 1 radius out
	glm::vec3 currentPoint = glm::vec3(0.0f, 0.0f, 0.0f); //the point x degrees along the circles circumference to form the next triangle


	glm::vec3 currentCenter = center; //the center of the current circle used for partitioning the sphere
	float currentRadius = a_fRadius; //the radius of the current circle used for partitioning the sphere

	//add points to sphere
	//creates circles connecting to previous circles as well as top and bottom
	for (int i = 0; i < a_nSubdivisions; i++)
	{

		currentCenter = glm::vec3(0.0f, a_fRadius * float(sin((radiansSeperationHalf * (i+1))+baseRadian)), 0.0f); //find out new center
		
		glm::vec3 surfacePoint = (glm::vec3(a_fRadius * float(cos((radiansSeperationHalf * (i+1)) + baseRadian)), 
			a_fRadius * float(sin((radiansSeperationHalf * (i + 1)) + baseRadian)), 0.0f)); //finds surface area point  at same vertical height as our new center

		currentRadius = glm::length(surfacePoint - currentCenter); //find radius of current circle

		prevPoint = glm::vec3(currentRadius, currentCenter.y, 0.0f);

		for (int j = 0; j < a_nSubdivisions; j++)
		{
			currentPoint = glm::vec3((currentRadius * float(cos(radiansSeperation * (j + 1)))), currentCenter.y, (currentRadius * float(sin(radiansSeperation * (j + 1)))));

			if (i == 0)
			{
				AddTri(bottom, currentPoint, prevPoint);
			}
			else if (i == a_nSubdivisions - 1)
			{
				AddTri(top, prevPoint, currentPoint);
			}
			else
			{

			}
			prevPoint = currentPoint;
		}

	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}