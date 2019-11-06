#include "MyOctant.h"
using namespace Simplex;

//set certain values in static to prevent linker errors
uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 3;
uint MyOctant::m_uIdealEntityCount = 5;

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
	m_fSize = 0.0f;

	m_v3Center = vector3(0.0f, 0.0f, 0.0f);
	m_v3Max = vector3(0.0f, 0.0f, 0.0f);
	m_v3Min = vector3(0.0f, 0.0f, 0.0f);

	//set all pointers to null
	m_pRoot = nullptr;
	m_pParent = nullptr;

	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i] = nullptr;
	}
}

//deleter method
void MyOctant::Release()
{
	//if the root kill branches
	if (m_uLevel = 0)
	{
		KillBranches();
	}
	//set all values to 0
	m_uChildren = 0;
	m_fSize = 0;
	m_EntityList.clear();
	m_lChild.clear();
	
}

//myoctant constructor for root
MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCout)
{
	//initialize octree
	Init();

	//read in new values
	m_uOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCout;
	m_uID = m_uOctantCount;

	//set this to root
	m_pRoot = this;
	m_lChild.clear();

	//read in the points of every min and max as vectors on a shape to create a large shape, then ge thte min and max of that shape
	std::vector<vector3> allPoints;

	uint entities = m_pEntityMngr->GetEntityCount();

	for (uint i = 0; i < entities; i++)
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
	m_fSize = size * 2.0f;
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

	m_v3Min = m_v3Center - (vector3((m_fSize / 2.0f)));
	m_v3Max = m_v3Center + (vector3((m_fSize / 2.0f)));

	m_uOctantCount++;
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
void MyOctant::Swap(MyOctant& other)
{
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();
	std::swap(m_uID,other.m_uID);//
	std::swap(m_uLevel,other.m_uLevel);//

	for (uint i = 0; i < 8; i++)
	{
		std::swap(m_pChild[i],other.m_pChild[i]);//
	}
	std::swap(m_lChild,  other.m_lChild);//
	std::swap(m_uChildren,  other.m_uChildren);// 

	std::swap(m_fSize, other.m_fSize);//

	std::swap(m_v3Center, other.m_v3Center);//
	std::swap(m_v3Min, other.m_v3Min);//
	std::swap(m_v3Max, other.m_v3Max);//

	std::swap(m_pParent, other.m_pParent);//
	std::swap(m_uLevel , other.m_uLevel);//
	std::swap(m_pRoot,  other.m_pRoot);//

	
}

//returns the octants size
float MyOctant::GetSize(void)
{
	return m_fSize;
}

//returns the octants center
vector3 MyOctant::GetCenterGlobal(void)
{
	return m_v3Center;
}

//returns the octants min
vector3 MyOctant::GetMinGlobal(void)
{
	return m_v3Min;
}

//returns the octants max
vector3 MyOctant::GetMaxGlobal(void)
{
	return m_v3Max;
}

//collision logic for the octant, determines if any points are colliding, aabb taken from myrigidbody.cpp
bool MyOctant::IsColliding(uint a_uRBIndex)
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

void MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
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
	};
}

void MyOctant::Display(vector3 a_v3Color)
{
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(
		glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)),
		a_v3Color, RENDER_WIRE
	);
}

void MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	uint leaves = m_lChild.size();
	for (uint i = 0; i < leaves; i++)
	{
		m_pChild[i]->DisplayLeafs(a_v3Color);
	}

	m_pMeshMngr->AddWireCubeToRenderList(
	glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)),
	a_v3Color, RENDER_WIRE);
}

//traverses the tree and clears entity lists for each node
void MyOctant::ClearEntityList(void)
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
//altered to follow along more with the example code
//original setup of the subdivide code drew vectors along eight diagonal lines
//and travelled along them to find new centers with the altered sizes
//broke because i needed to divide the size by 4 instead of just by 2 leading to the new centers being at
//the 8 vertexes of the beginning cube
void MyOctant::Subdivide(void)
{
	//break if at maximum level
	if (m_uLevel >= m_uMaxLevel)
	{
		return;
	}
	//break if already divided
	if (m_uChildren != 0)
	{
		return;
	}
	//declare children
	m_uChildren = 8;

	
	float childHalfSize = m_fSize / 4.0f;
	float childSize = m_fSize / 2.0f;

	vector3 newCenter = vector3(m_v3Center.x - childHalfSize, m_v3Center.y - childHalfSize, m_v3Center.z - childHalfSize);

	//create the eight children moving through the cube
	m_pChild[0] = new MyOctant(newCenter, childSize);

	newCenter.x += childSize;
	m_pChild[1] = new MyOctant(newCenter, childSize);

	newCenter.z += childSize;
	m_pChild[2] = new MyOctant(newCenter, childSize);

	newCenter.x -= childSize;
	m_pChild[3] = new MyOctant(newCenter, childSize);

	newCenter.y += childSize;
	m_pChild[4] = new MyOctant(newCenter, childSize);

	newCenter.z -= childSize;
	m_pChild[5] = new MyOctant(newCenter, childSize);

	newCenter.x += childSize;
	m_pChild[6] = new MyOctant(newCenter, childSize);

	newCenter.z += childSize;
	m_pChild[7] = new MyOctant(newCenter, childSize);

	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_pRoot = m_pRoot;
		m_pChild[i]->m_uLevel = m_uLevel + 1;
		if (m_pChild[i]->ContainsMoreThan(m_uIdealEntityCount))
		{
			m_pChild[i]->Subdivide();
		}
	}
}

//returns the pointer to the specific child
MyOctant* MyOctant::GetChild(uint a_nChild)
{
	if (a_nChild > 7 || a_nChild < 0)
	{
		return nullptr;
	}
	return m_pChild[a_nChild];
}

//returns the pointer to the parent if not null
MyOctant* MyOctant::GetParent(void)
{
	return m_pParent;
}

//returns whether or not the octant is a leaf
bool MyOctant::IsLeaf(void)
{
	if (m_uChildren == 0)
	{
		return true;
	}

	return false;
}

//returns true if the octant contains more then a certain number of entities
bool MyOctant::ContainsMoreThan(uint a_nEntities)
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
void MyOctant::KillBranches(void)
{
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->KillBranches();
		delete m_pChild[i];
		m_pChild[i] = nullptr;
	}
	m_uChildren = 0;
}

void MyOctant::ConstructTree(uint a_nMaxLevel)
{
	//break if not root
	if (m_uLevel != 0)
	{
		return;
	}

	m_uMaxLevel = a_nMaxLevel;

	m_uOctantCount = 1;

	m_EntityList.clear();

	//clear tree before setting up id
	KillBranches();
	m_lChild.clear();

	//subdivide if needed
	if (ContainsMoreThan(m_uIdealEntityCount))
	{
		Subdivide();
	}

	//assign ids to entities
	AssignIDtoEntity();

	//get list of leaves with objects inside
	ConstructList();
}

void MyOctant::AssignIDtoEntity(void)
{
	//only assign ids to leaves, not higher branches
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->AssignIDtoEntity();
	}

	if (IsLeaf())
	{
		uint entityCount = m_pEntityMngr->GetEntityCount();
		for (uint i = 0; i < entityCount; i++)
		{
			if (IsColliding(i))
			{
				//add the entity to the list of entities in the octant
				m_EntityList.push_back(i);

				m_pEntityMngr->AddDimension(i, m_uID);
			}
		}
	}
}

//returns the number total number of octants in the world
uint MyOctant::GetOctantCount(void)
{
	return m_uOctantCount;
}

void MyOctant::ConstructList(void)
{
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ConstructList();
	}

	if (m_EntityList.size() > 0)
	{
		m_pRoot->m_lChild.push_back(this);
	}
}
