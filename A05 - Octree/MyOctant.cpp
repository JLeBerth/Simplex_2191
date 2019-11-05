#include "MyOctant.h"
using namespace Simplex;

//Constructor, will create an octant containing all MagnaEntities Instances in the Mesh
//manager	currently contains
Simplex::MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	//get a reference to the entity manager and mesh manager
	m_pEntityMngr = MyEntityManager::GetInstance();
	m_pMeshMngr = MeshManager::GetInstance();

	//if the root then add every entity to the entity list
	if (m_pParent == NULL)
	{
		m_uLevel = 1;
		for (int i = 0; i < m_pEntityMngr->GetEntityCount(); i++)
		{
			m_EntityList.push_back(i);
		}
	}

	//get max min and size
	m_v3Max = GetMaxGlobal();
	m_v3Min = GetMinGlobal();
	m_v3Center = GetCenterGlobal();
	m_fSize = GetSize();



}
//Constructor will create an octant within a specific space
Simplex::MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	//get a reference to the entity manager and mesh manager
	m_pEntityMngr = MyEntityManager::GetInstance();
	m_pMeshMngr = MeshManager::GetInstance();

	//assign center and size
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	//find min and max based on center and size
	m_v3Max = m_v3Center + (glm::normalize(vector3(1.0f, 1.0f, 1.0f)) * m_fSize);
	m_v3Min = m_v3Center + (glm::normalize(vector3(1.0f, 1.0f, 1.0f)) * -m_fSize);
}

Simplex::MyOctant::MyOctant(MyOctant const& other)
{

}
/*
MyOctant& Simplex::MyOctant::operator=(MyOctant const& other)
{
	// TODO: insert return statement here
}
*/

Simplex::MyOctant::~MyOctant(void)
{
}

void Simplex::MyOctant::Swap(MyOctant& other)
{
}

float Simplex::MyOctant::GetSize(void)
{
	//finds size based on min and max
	return glm::length((m_v3Max - m_v3Min) / 2.0f);
}

vector3 Simplex::MyOctant::GetCenterGlobal(void)
{
	//returns the center based on min and max
	return (m_v3Max + m_v3Min) / 2.0f;
}

vector3 Simplex::MyOctant::GetMinGlobal(void)
{
	float vecX = INT_MAX;
	float vecY = INT_MAX;
	float vecZ = INT_MAX;
	if (m_pParent == NULL)
	{
		for each (uint thisID in m_EntityList)
		{
			MyEntity* thisEntity = m_pEntityMngr->GetEntity(thisID);
			MyRigidBody thisRigidbody = *thisEntity->GetRigidBody();
			vector3 thisMax = thisRigidbody.GetMaxGlobal();

			if (thisMax.x < vecX)
			{
				vecX = thisMax.x;
			}
			if (thisMax.y < vecY)
			{
				vecY = thisMax.y;
			}
			if (thisMax.z < vecZ)
			{
				vecZ = thisMax.z;
			}
		}
	}

	return vector3(vecX, vecY, vecZ);
}

vector3 Simplex::MyOctant::GetMaxGlobal(void)
{
	float vecX = INT_MIN;
	float vecY = INT_MIN;
	float vecZ = INT_MIN;
	if (m_pParent == NULL)
	{
		for each (uint thisID in m_EntityList)
		{
			MyEntity *thisEntity = m_pEntityMngr->GetEntity(thisID);
			MyRigidBody thisRigidbody = *thisEntity->GetRigidBody();
			vector3 thisMax = thisRigidbody.GetMaxGlobal();

			if (thisMax.x > vecX)
			{
				vecX = thisMax.x;
			}
			if (thisMax.y > vecY)
			{
				vecY = thisMax.y;
			}
			if (thisMax.z > vecZ)
			{
				vecZ = thisMax.z;
			}
		}
	}

	return vector3(vecX, vecY, vecZ);
}

bool Simplex::MyOctant::IsColliding(uint a_uRBIndex)
{
	return false;
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
	return m_pChild[a_nChild];
}

//returns the pointer to the parent if not null
MyOctant* Simplex::MyOctant::GetParent(void)
{
	if (m_pParent != NULL)
	{
		return m_pParent;
	}
	return nullptr;
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
	if (m_EntityList.size() > a_nEntities)
	{
		return true;
	}
	return false;
}

void Simplex::MyOctant::KillBranches(void)
{
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
