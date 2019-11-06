#include "MyOctant.h"
using namespace Simplex;

//Constructor, will create an octant containing all MagnaEntities Instances in the Mesh
//manager	currently contains

//method to initialize values
void MyOctant::Init()
{
	//get a reference to the entity manager and mesh manager
	m_pEntityMngr = MyEntityManager::GetInstance();
	m_pMeshMngr = MeshManager::GetInstance();

	//declare children and other setup 
	m_uChildren = 0;
	m_uLevel = 0;
	m_uID = m_uOctantCount;

	m_v3Center = vector3(0.0f, 0.0f, 0.0f);
	m_v3Max = vector3(0.0f, 0.0f, 0.0f);
	m_v3Min = vector3(0.0f, 0.0f, 0.0f);

	//set all pointers to null
	m_pRoot = nullptr;
	m_pParent = nullptr;

	for (int i = 0; i < 8; i++)
	{
		m_pChild[i] = nullptr;
	}

}

//deleter method
void MyOctant::Release()
{
	//set all values to 0
	m_uChildren = 0;
	m_fSize = 0;
	m_EntityList.clear();
	m_lChild.clear();
	//if the root kill branches
	if (m_uLevel = 0)
	{
		KillBranches;
	}
}

//myoctant constructor for root
MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCout)
{
	//initialize octree
	Init();

	//read in new values
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCout;
	m_uID = m_uOctantCount;

	//set this to root
	m_pRoot = this;

	//read in the points of every min and max as vectors on a shape to create a large shape, then ge thte min and max of that shape
	std::vector<vector3> allPoints;

	uint entities = m_pEntityMngr->GetEntityCount();

	for (int i = 0; i < entities; i++)
	{
		//get the rigid body from the entity list
		MyEntity* entityPointer = m_pEntityMngr->GetEntity(i);
		MyRigidBody* rigidBodyPointer = entityPointer->GetRigidBody();

		//get the min and max from the rigidbody

		allPoints.push_back(rigidBodyPointer->GetMaxGlobal());
		allPoints.push_back(rigidBodyPointer->GetMinGlobal());
	}
	MyRigidBody* rigidbodyPointer = new MyRigidBody(allPoints);

	//set size to x length, then check it against the y and z and change to those if they are larger
	vector3 sizelength = rigidbodyPointer->GetHalfWidth();
	float size = sizelength.x;

	if (sizelength.y > size)
	{
		size = sizelength.y;
	}
	if (sizelength.z > size)
	{
		size = sizelength.z;
	}

	//get center of rigidbody
	m_v3Center = rigidbodyPointer->GetCenterLocal();
	
	//values no longer needed clean up
	SafeDelete(rigidbodyPointer);
	allPoints.clear();

	//use gotten values
	m_fSize = size * 2;
	m_v3Max = m_v3Center + vector3(size);
	m_v3Min = m_v3Center - vector3(size);

	//increment octant count
	m_uOctantCount += 1;

	ConstructTree(m_uMaxLevel);
}

//myoctant constructor for branches
MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	//initialize values
	Init();
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	m_v3Min = m_v3Center - vector3((m_fSize / 2.0f));
	m_v3Max = m_v3Center + vector3((m_fSize / 2.0f));

	m_uOctantCount;
}

//myoctant copyconstructor
MyOctant::MyOctant(MyOctant const& other)
{

	m_uID = other.m_uID; 
	m_uLevel = other.m_uLevel; 

	for (int i = 0; i < 8; i++)
	{
		m_pChild[i] = other.m_pChild[i];
	}
	m_lChild = other.m_lChild;
	m_uChildren = other.m_uChildren;

	m_fSize = other.m_fSize; 

	m_pMeshMngr = nullptr;
	m_pEntityMngr = nullptr; 

	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	m_pParent = other.m_pParent;
	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_pRoot = other.m_pRoot;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();
}

//myoctant set & operator, creates an & call for the octant
MyOctant& MyOctant::operator=(MyOctant const& other)
{
	//swap if needed then return 
	if (this != &other)
	{
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}

//set release to the deleter
MyOctant::~MyOctant()
{
	Release();
}

//swap statement for myoctant
void Simplex::MyOctant::Swap(MyOctant& other)
{
}

//returns the octants size
float Simplex::MyOctant::GetSize(void)
{
	return m_fSize;
}

//returns the octants center
vector3 Simplex::MyOctant::GetCenterGlobal(void)
{
	return m_v3Center;
}

//returns the octants min
vector3 Simplex::MyOctant::GetMinGlobal(void)
{
	return m_v3Min;
}

//returns the octants max
vector3 Simplex::MyOctant::GetMaxGlobal(void)
{
	return m_v3Max;
}

//collision logic for the octant, determines if any points are colliding, aabb taken from myrigidbody.cpp
bool Simplex::MyOctant::IsColliding(uint a_uRBIndex)
{
	//check to see if the object is in the octree area
	uint nObjectCount = m_pEntityMngr->GetEntityCount();
	if (a_uRBIndex >= nObjectCount)
	{
		return false;
	}

	//get objects min and max to check for collision
	MyEntity* entityPointer = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* rigidbodyPointer = entityPointer->GetRigidBody();
	vector3 otherMin = rigidbodyPointer->GetMinGlobal();
	vector3 otherMax = rigidbodyPointer->GetMaxGlobal();

	bool colliding = true;

	//check x
	if (m_v3Max.x < otherMin.x)
	{
		colliding = false;
	}
	if (m_v3Min.x > otherMax.x)
	{
		colliding = false;
	}

	//check y
	if (m_v3Max.y < otherMin.y)
	{
		colliding = false;
	}
	if (m_v3Min.y > otherMax.y)
	{
		colliding = false;
	}

	//check z
	if (m_v3Max.z < otherMin.z)
	{
		colliding = false;
	}
	if (m_v3Min.z > otherMax.z)
	{
		colliding = false;
	}

	return colliding;
}

void Simplex::MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_nIndex, a_v3Color);
	}
	if (m_uID == a_nIndex)
	{
		m_pMeshMngr->AddWireCubeToRenderList(
			glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)),
			a_v3Color, RENDER_WIRE
		);
	}
	return;
}

void Simplex::MyOctant::Display(vector3 a_v3Color)
{
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(
		glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)),
		a_v3Color, RENDER_WIRE
	);
	return;
}

void Simplex::MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->DisplayLeafs(a_v3Color);
	}
	if (IsLeaf())
	{
		m_pMeshMngr->AddWireCubeToRenderList(
			glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)),
			a_v3Color, RENDER_WIRE
		);
	}
	return;
}

//traverses the tree and clears entity lists for each node
void Simplex::MyOctant::ClearEntityList(void)
{
	m_EntityList.clear();
	if (!IsLeaf())
	{
		for (int i = 0; i < 8; i++)
		{
			if (m_pChild[i] != NULL)
			{
				m_pChild[i]->ClearEntityList();
			}
		}
	}
}

//allocates 8 smaller octants in the child pointers
void Simplex::MyOctant::Subdivide(void)
{
	for (int i = 0; i < 8; i++)
	{
		//find new size as half of the previous
		float newSize = m_fSize / 2.0f;

		//find new center by moving center the new size in one of eight directions
		vector3 newCenter;
		vector3 direction;
		switch (i)
		{
		case 0:
			direction = vector3(1.0, 1.0, 1.0);
			break;
		case 1:
			direction = vector3(1.0, -1.0, 1.0);
			break;
		case 2:
			direction = vector3(1.0, 1.0, -1.0);
			break;
		case 3:
			direction = vector3(1.0, -1.0, -1.0);
			break;
		case 4:
			direction = vector3(-1.0, 1.0, 1.0);
			break;
		case 5:
			direction = vector3(-1.0, -1.0, 1.0);
			break;
		case 6:
			direction = vector3(-1.0, 1.0, -1.0);
			break;
		case 7:
			direction = vector3(-1.0, -1.0, -1.0);
			break;
		}
		direction = glm::normalize(direction);
		newCenter = m_v3Center + (direction * newSize);
		m_pChild[i] = new MyOctant(newCenter, newSize);
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel = this->m_uLevel + 1;
	}
	m_uChildren = 8;
}

//returns the pointer to the specific child
MyOctant* Simplex::MyOctant::GetChild(uint a_nChild)
{
	if (a_nChild > 7 || a_nChild < 0)
	{
		return nullptr;
	}
	return m_pChild[a_nChild];
}

//returns the pointer to the parent if not null
MyOctant* Simplex::MyOctant::GetParent(void)
{
	return m_pParent;
}

//returns whether or not the octant is a leaf
bool Simplex::MyOctant::IsLeaf(void)
{
	if (m_uChildren == 0)
	{
		return true;
	}

	return false;
}

//returns true if the octant contains more then a certain number of entities
bool Simplex::MyOctant::ContainsMoreThan(uint a_nEntities)
{
	uint entities = 0;
	int objects = m_pEntityMngr->GetEntityCount();

	//if the object is in the area then increase the contains number, then return true if greater then the sought number
	for (int i = 0; i < objects; i++)
	{
		if (IsColliding(i))
		{
			entities += 1;
		}
		if (entities > a_nEntities)
		{
			return true;
		}
	}
	return false;
}

//delete children, and their children, works from the bottom up
void Simplex::MyOctant::KillBranches(void)
{
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->KillBranches();
		delete m_pChild[i];
		m_pChild[i] = nullptr;
	}
	m_uChildren = 0;
}

void Simplex::MyOctant::ConstructTree(uint a_nMaxLevel)
{
}

void Simplex::MyOctant::AssignIDtoEntity(void)
{
}

//returns the number total number of octants in the world
uint Simplex::MyOctant::GetOctantCount(void)
{
	uint count = 0;
	if (!IsLeaf())
	{
		for (int i = 0; i < 8; i++)
		{
			if (m_pChild[i] != NULL)
			{
				count += m_pChild[i]->GetOctantCount();
			}
		}
	}
	count++;
	return uint();
}
