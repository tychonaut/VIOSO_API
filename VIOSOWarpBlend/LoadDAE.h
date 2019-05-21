#include "mmath.h"

typedef struct TriangleMesh
{
#ifdef WIN32
	typedef __declspec( align(4) ) std::vector<VWB_float> FloatVec;
	typedef __declspec( align(4) ) std::vector<VWB_int> IntVec;
	typedef __declspec( align(4) ) struct TexT { VWB_float u,v; } TexT;
	typedef __declspec( align(4) ) struct ColT { VWB_float r,g,b,a; } ColT;
	typedef __declspec( align(4) ) struct MeshVertex
	{
		VWB_VEC3f pos;
		VWB_VEC3f norm;
		TexT tex;
	} MeshVertex;
	typedef __declspec( align(4) ) std::vector<MeshVertex> MeshVertices;
#else
    typedef std::vector<VWB_float> FloatVec __attribute__ ((aligned (4)));
    typedef std::vector<VWB_int> IntVec __attribute__ ((aligned (4)));
    typedef struct TexT { VWB_float u,v; } TexT __attribute__ ((aligned (4)));
    typedef struct ColT { VWB_float r,g,b,a; } ColT __attribute__ ((aligned (4)));
    typedef struct MeshVertex
    {
        VWB_VEC3f pos;
        VWB_VEC3f norm;
        TexT tex;
    } MeshVertex __attribute__ ((aligned (4)));
    typedef std::vector<MeshVertex> MeshVertices __attribute__ ((aligned (4)));
#endif
	typedef enum 
	{
		VALIDFLAG_NONE = 0,
		VALIDFLAG_POS = 1,
		VALIDFLAG_NORM = 2,
		VALIDFLAG_TEX = 4
	} VALIDFLAG;

	MeshVertices m_vertices;
	IntVec		m_indeces;
	VWB_int		m_validFlags;

	VWB_MAT44f	m_base;

	TriangleMesh()
	: m_base(VWB_MAT44f::I())
	, m_validFlags(0)
	{
	}

	TriangleMesh( TriangleMesh const& other )
	: m_vertices(other.m_vertices)
	, m_indeces(other.m_indeces)
	, m_base( other.m_base )
	, m_validFlags(other.m_validFlags)
	{
	}

	~TriangleMesh()
	{
	}

	static const MeshVertex _InvVertex;

} TriangleMesh;

typedef std::vector<TriangleMesh> Geometry;
typedef std::vector<Geometry> Scene;

VWB_ERROR loadDAE( Scene& scene, char const* path );
