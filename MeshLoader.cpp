//--------------------------------------------------------------------------------------
// File: MeshLoader.cpp
//
// Wrapper class for ID3DXMesh interface. Handles loading mesh data from an .obj file
// and resource management for material textures.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "SDKmisc.h"
#pragma warning(disable: 4995)
#include "meshloader.h"
#include <fstream>
using namespace std;
#pragma warning(default: 4995)


// Vertex declaration
D3DVERTEXELEMENT9 VERTEX_DECL[] =
{
	{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0},
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0},
	{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TANGENT,   0},
	{ 0, 36, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BINORMAL,   0},
    { 0, 48, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0},
    D3DDECL_END()
};


//--------------------------------------------------------------------------------------
CMeshLoader::CMeshLoader()
{
    m_pd3dDevice = NULL;
    m_pMesh = NULL;

    ZeroMemory( m_strMediaDir, sizeof( m_strMediaDir ) );
}


//--------------------------------------------------------------------------------------
CMeshLoader::~CMeshLoader()
{
    Destroy();
}


//--------------------------------------------------------------------------------------
void CMeshLoader::Destroy()
{
    for( int iMaterial = 0; iMaterial < m_Materials.GetSize(); iMaterial++ )
    {
        Material* pMaterial = m_Materials.GetAt( iMaterial );

        // Avoid releasing the same texture twice
        for( int x = iMaterial + 1; x < m_Materials.GetSize(); x++ )
        {
            Material* pCur = m_Materials.GetAt( x );
            if( pCur->pDiffuseTex == pMaterial->pDiffuseTex )
                pCur->pDiffuseTex = NULL;
        }

        SAFE_RELEASE( pMaterial->pDiffuseTex );
    }
	for( int iMaterial = 0; iMaterial < m_Materials.GetSize(); iMaterial++ )
	{
		Material* pMaterial = m_Materials.GetAt( iMaterial );

		// Avoid releasing the same texture twice
		for( int x = iMaterial + 1; x < m_Materials.GetSize(); x++ )
		{
			Material* pCur = m_Materials.GetAt( x );
			if( pCur->pSpecularTex == pMaterial->pSpecularTex )
				pCur->pSpecularTex = NULL;
		}

		SAFE_RELEASE( pMaterial->pSpecularTex );
	}
	for( int iMaterial = 0; iMaterial < m_Materials.GetSize(); iMaterial++ )
	{
		Material* pMaterial = m_Materials.GetAt( iMaterial );

		// Avoid releasing the same texture twice
		for( int x = iMaterial + 1; x < m_Materials.GetSize(); x++ )
		{
			Material* pCur = m_Materials.GetAt( x );
			if( pCur->pNormalTex == pMaterial->pNormalTex )
				pCur->pNormalTex = NULL;
		}

		SAFE_RELEASE( pMaterial->pNormalTex );
	}
	for( int iMaterial = 0; iMaterial < m_Materials.GetSize(); iMaterial++ )
	{
		Material* pMaterial = m_Materials.GetAt( iMaterial );
		SAFE_DELETE( pMaterial );
	}

    m_Materials.RemoveAll();
    m_Vertices.RemoveAll();
    m_Indices.RemoveAll();
    m_Attributes.RemoveAll();

    SAFE_RELEASE( m_pMesh );
    m_pd3dDevice = NULL;
}


//--------------------------------------------------------------------------------------
HRESULT CMeshLoader::Create( IDirect3DDevice9* pd3dDevice, const WCHAR* strFilename )
{
    HRESULT hr;
    WCHAR str[ MAX_PATH ] = {0};

    // Start clean
    Destroy();

    // Store the device pointer
    m_pd3dDevice = pd3dDevice;

    // Load the vertex buffer, index buffer, and subset information from a file. In this case, 
    // an .obj file was chosen for simplicity, but it's meant to illustrate that ID3DXMesh objects
    // can be filled from any mesh file format once the necessary data is extracted from file.
    V_RETURN( LoadGeometryFromOBJ( strFilename ) );

    // Set the current directory based on where the mesh was found
    WCHAR wstrOldDir[MAX_PATH] = {0};
    GetCurrentDirectory( MAX_PATH, wstrOldDir );
    SetCurrentDirectory( m_strMediaDir );

    // Load material textures
    for( int iMaterial = 0; iMaterial < m_Materials.GetSize(); iMaterial++ )
    {
        Material* pMaterial = m_Materials.GetAt( iMaterial );
        if( pMaterial->strDiffuseTex[0] )
        {
            // Avoid loading the same texture twice
            bool bFound = false;
            for( int x = 0; x < iMaterial; x++ )
            {
                Material* pCur = m_Materials.GetAt( x );
                if( 0 == wcscmp( pCur->strDiffuseTex, pMaterial->strDiffuseTex ) )
                {
                    bFound = true;
                    pMaterial->pDiffuseTex = pCur->pDiffuseTex;
                    break;
                }
            }

            // Not found, load the texture
            if( !bFound )
            {
                V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, pMaterial->strDiffuseTex ) );
                V_RETURN( D3DXCreateTextureFromFile( pd3dDevice, pMaterial->strDiffuseTex,
                                                     &( pMaterial->pDiffuseTex ) ) );
            }
        }
		if( pMaterial->strSpecularTex[0] )
		{
			// Avoid loading the same texture twice
			bool bFound = false;
			for( int x = 0; x < iMaterial; x++ )
			{
				Material* pCur = m_Materials.GetAt( x );
				if( 0 == wcscmp( pCur->strSpecularTex, pMaterial->strSpecularTex ) )
				{
					bFound = true;
					pMaterial->pSpecularTex = pCur->pSpecularTex;
					break;
				}
			}

			// Not found, load the texture
			if( !bFound )
			{
				V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, pMaterial->strSpecularTex ) );
				V_RETURN( D3DXCreateTextureFromFile( pd3dDevice, pMaterial->strSpecularTex,
					&( pMaterial->pSpecularTex ) ) );
			}
		}
		if( pMaterial->strNormalTex[0] )
		{
			// Avoid loading the same texture twice
			bool bFound = false;
			for( int x = 0; x < iMaterial; x++ )
			{
				Material* pCur = m_Materials.GetAt( x );
				if( 0 == wcscmp( pCur->strNormalTex, pMaterial->strNormalTex ) )
				{
					bFound = true;
					pMaterial->pNormalTex = pCur->pNormalTex;
					break;
				}
			}

			// Not found, load the texture
			if( !bFound )
			{
				V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, pMaterial->strNormalTex ) );
				V_RETURN( D3DXCreateTextureFromFile( pd3dDevice, pMaterial->strNormalTex,
					&( pMaterial->pNormalTex ) ) );
			}
		}
    }

    // Restore the original current directory
    SetCurrentDirectory( wstrOldDir );

	// compute TBN data
	FillTBN();

    // Create the encapsulated mesh
    ID3DXMesh* pMesh = NULL;
    V_RETURN( D3DXCreateMesh( m_Indices.GetSize() / 3, m_Vertices.GetSize(),
                              D3DXMESH_MANAGED | D3DXMESH_32BIT, VERTEX_DECL,
                              pd3dDevice, &pMesh ) );

    // Copy the vertex data
    VERTEX* pVertex;
    V_RETURN( pMesh->LockVertexBuffer( 0, ( void** )&pVertex ) );
    memcpy( pVertex, m_Vertices.GetData(), m_Vertices.GetSize() * sizeof( VERTEX ) );
    pMesh->UnlockVertexBuffer();
    m_Vertices.RemoveAll();

    // Copy the index data
    DWORD* pIndex;
    V_RETURN( pMesh->LockIndexBuffer( 0, ( void** )&pIndex ) );
    memcpy( pIndex, m_Indices.GetData(), m_Indices.GetSize() * sizeof( DWORD ) );
    pMesh->UnlockIndexBuffer();
    m_Indices.RemoveAll();

    // Copy the attribute data
    DWORD* pSubset;
    V_RETURN( pMesh->LockAttributeBuffer( 0, &pSubset ) );
    memcpy( pSubset, m_Attributes.GetData(), m_Attributes.GetSize() * sizeof( DWORD ) );
    pMesh->UnlockAttributeBuffer();
    m_Attributes.RemoveAll();

    // Reorder the vertices according to subset and optimize the mesh for this graphics 
    // card's vertex cache. When rendering the mesh's triangle list the vertices will 
    // cache hit more often so it won't have to re-execute the vertex shader.
    DWORD* aAdjacency = new DWORD[pMesh->GetNumFaces() * 3];
    if( aAdjacency == NULL )
        return E_OUTOFMEMORY;

    V( pMesh->GenerateAdjacency( 1e-6f, aAdjacency ) );
    V( pMesh->OptimizeInplace( D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, aAdjacency, NULL, NULL, NULL ) );

    SAFE_DELETE_ARRAY( aAdjacency );
    m_pMesh = pMesh;

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CMeshLoader::LoadGeometryFromOBJ( const WCHAR* strFileName )
{
    WCHAR strMaterialFilename[MAX_PATH] = {0};
    WCHAR wstr[MAX_PATH];
    char str[MAX_PATH];
    HRESULT hr;

    // Find the file
    V_RETURN( DXUTFindDXSDKMediaFileCch( wstr, MAX_PATH, strFileName ) );
    WideCharToMultiByte( CP_ACP, 0, wstr, -1, str, MAX_PATH, NULL, NULL );

    // Store the directory where the mesh was found
    wcscpy_s( m_strMediaDir, MAX_PATH - 1, wstr );
    WCHAR* pch = wcsrchr( m_strMediaDir, L'\\' );
    if( pch )
        *pch = NULL;

    // Create temporary storage for the input data. Once the data has been loaded into
    // a reasonable format we can create a D3DXMesh object and load it with the mesh data.
    CGrowableArray <D3DXVECTOR3> Positions;
    CGrowableArray <D3DXVECTOR2> TexCoords;
    CGrowableArray <D3DXVECTOR3> Normals;

    // The first subset uses the default material
    Material* pMaterial = new Material();
    if( pMaterial == NULL )
        return E_OUTOFMEMORY;

    InitMaterial( pMaterial );
    wcscpy_s( pMaterial->strName, MAX_PATH - 1, L"default" );
    m_Materials.Add( pMaterial );

    DWORD dwCurSubset = 0;

    // File input
    WCHAR strCommand[256] = {0};
    wifstream InFile( str );
    if( !InFile )
        return DXTRACE_ERR( L"wifstream::open", E_FAIL );

    for(; ; )
    {
        InFile >> strCommand;
        if( !InFile )
            break;

        if( 0 == wcscmp( strCommand, L"#" ) )
        {
            // Comment
        }
        else if( 0 == wcscmp( strCommand, L"v" ) )
        {
            // Vertex Position
            float x, y, z;
            InFile >> x >> y >> z;
            Positions.Add( D3DXVECTOR3( x, y, z ) );
        }
        else if( 0 == wcscmp( strCommand, L"vt" ) )
        {
            // Vertex TexCoord
            float u, v;
            InFile >> u >> v;
            TexCoords.Add( D3DXVECTOR2( u, v ) );
        }
        else if( 0 == wcscmp( strCommand, L"vn" ) )
        {
            // Vertex Normal
            float x, y, z;
            InFile >> x >> y >> z;
            Normals.Add( D3DXVECTOR3( x, y, z ) );
        }
        else if( 0 == wcscmp( strCommand, L"f" ) )
        {
            // Face
            UINT iPosition, iTexCoord, iNormal;
            VERTEX vertex;

            for( UINT iFace = 0; iFace < 3; iFace++ )
            {
                ZeroMemory( &vertex, sizeof( VERTEX ) );

                // OBJ format uses 1-based arrays
                InFile >> iPosition;
                vertex.position = Positions[ iPosition - 1 ];

                if( '/' == InFile.peek() )
                {
                    InFile.ignore();

                    if( '/' != InFile.peek() )
                    {
                        // Optional texture coordinate
                        InFile >> iTexCoord;
                        vertex.texcoord = TexCoords[ iTexCoord - 1 ];
                    }

                    if( '/' == InFile.peek() )
                    {
                        InFile.ignore();

                        // Optional vertex normal
                        InFile >> iNormal;
                        vertex.normal = Normals[ iNormal - 1 ];
                    }
                }

                // If a duplicate vertex doesn't exist, add this vertex to the Vertices
                // list. Store the index in the Indices array. The Vertices and Indices
                // lists will eventually become the Vertex Buffer and Index Buffer for
                // the mesh.
                DWORD index = AddVertex( iPosition, &vertex );
                if ( index == (DWORD)-1 )
                    return E_OUTOFMEMORY;

                m_Indices.Add( index );
            }
            m_Attributes.Add( dwCurSubset );
        }
        else if( 0 == wcscmp( strCommand, L"mtllib" ) )
        {
            // Material library
            InFile >> strMaterialFilename;
        }
        else if( 0 == wcscmp( strCommand, L"usemtl" ) )
        {
            // Material
            WCHAR strName[MAX_PATH] = {0};
            InFile >> strName;

            bool bFound = false;
            for( int iMaterial = 0; iMaterial < m_Materials.GetSize(); iMaterial++ )
            {
                Material* pCurMaterial = m_Materials.GetAt( iMaterial );
                if( 0 == wcscmp( pCurMaterial->strName, strName ) )
                {
                    bFound = true;
                    dwCurSubset = iMaterial;
                    break;
                }
            }

            if( !bFound )
            {
                pMaterial = new Material();
                if( pMaterial == NULL )
                    return E_OUTOFMEMORY;

                dwCurSubset = m_Materials.GetSize();

                InitMaterial( pMaterial );
                wcscpy_s( pMaterial->strName, MAX_PATH - 1, strName );

                m_Materials.Add( pMaterial );
            }
        }
        else
        {
            // Unimplemented or unrecognized command
        }

        InFile.ignore( 1000, '\n' );
    }

    // Cleanup
    InFile.close();
    DeleteCache();

    // If an associated material file was found, read that in as well.
    if( strMaterialFilename[0] )
    {
        V_RETURN( LoadMaterialsFromMTL( strMaterialFilename ) );
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
DWORD CMeshLoader::AddVertex( UINT hash, VERTEX* pVertex )
{
    // If this vertex doesn't already exist in the Vertices list, create a new entry.
    // Add the index of the vertex to the Indices list.
    bool bFoundInList = false;
    DWORD index = 0;

    // Since it's very slow to check every element in the vertex list, a hashtable stores
    // vertex indices according to the vertex position's index as reported by the OBJ file
    if( ( UINT )m_VertexCache.GetSize() > hash )
    {
        CacheEntry* pEntry = m_VertexCache.GetAt( hash );
        while( pEntry != NULL )
        {
            VERTEX* pCacheVertex = m_Vertices.GetData() + pEntry->index;

            // If this vertex is identical to the vertex already in the list, simply
            // point the index buffer to the existing vertex
            if( 0 == memcmp( pVertex, pCacheVertex, sizeof( VERTEX ) ) )
            {
                bFoundInList = true;
                index = pEntry->index;
                break;
            }

            pEntry = pEntry->pNext;
        }
    }

    // Vertex was not found in the list. Create a new entry, both within the Vertices list
    // and also within the hashtable cache
    if( !bFoundInList )
    {
        // Add to the Vertices list
        index = m_Vertices.GetSize();
        m_Vertices.Add( *pVertex );

        // Add this to the hashtable
        CacheEntry* pNewEntry = new CacheEntry;
        if( pNewEntry == NULL )
            return (DWORD)-1;

        pNewEntry->index = index;
        pNewEntry->pNext = NULL;

        // Grow the cache if needed
        while( ( UINT )m_VertexCache.GetSize() <= hash )
        {
            m_VertexCache.Add( NULL );
        }

        // Add to the end of the linked list
        CacheEntry* pCurEntry = m_VertexCache.GetAt( hash );
        if( pCurEntry == NULL )
        {
            // This is the head element
            m_VertexCache.SetAt( hash, pNewEntry );
        }
        else
        {
            // Find the tail
            while( pCurEntry->pNext != NULL )
            {
                pCurEntry = pCurEntry->pNext;
            }

            pCurEntry->pNext = pNewEntry;
        }
    }

    return index;
}


//--------------------------------------------------------------------------------------
void CMeshLoader::DeleteCache()
{
    // Iterate through all the elements in the cache and subsequent linked lists
    for( int i = 0; i < m_VertexCache.GetSize(); i++ )
    {
        CacheEntry* pEntry = m_VertexCache.GetAt( i );
        while( pEntry != NULL )
        {
            CacheEntry* pNext = pEntry->pNext;
            SAFE_DELETE( pEntry );
            pEntry = pNext;
        }
    }

    m_VertexCache.RemoveAll();
}


//--------------------------------------------------------------------------------------
HRESULT CMeshLoader::LoadMaterialsFromMTL( const WCHAR* strFileName )
{
    HRESULT hr;

    // Set the current directory based on where the mesh was found
    WCHAR wstrOldDir[MAX_PATH] = {0};
    GetCurrentDirectory( MAX_PATH, wstrOldDir );
    SetCurrentDirectory( m_strMediaDir );

    // Find the file
    WCHAR strPath[MAX_PATH];
    char cstrPath[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( strPath, MAX_PATH, strFileName ) );
    WideCharToMultiByte( CP_ACP, 0, strPath, -1, cstrPath, MAX_PATH, NULL, NULL );

    // File input
    WCHAR strCommand[256] = {0};
    wifstream InFile( cstrPath );
    if( !InFile )
        return DXTRACE_ERR( L"wifstream::open", E_FAIL );

    // Restore the original current directory
    SetCurrentDirectory( wstrOldDir );

    Material* pMaterial = NULL;

    for(; ; )
    {
        InFile >> strCommand;
        if( !InFile )
            break;

        if( 0 == wcscmp( strCommand, L"newmtl" ) )
        {
            // Switching active materials
            WCHAR strName[MAX_PATH] = {0};
            InFile >> strName;

            pMaterial = NULL;
            for( int i = 0; i < m_Materials.GetSize(); i++ )
            {
                Material* pCurMaterial = m_Materials.GetAt( i );
                if( 0 == wcscmp( pCurMaterial->strName, strName ) )
                {
                    pMaterial = pCurMaterial;
                    break;
                }
            }
        }

        // The rest of the commands rely on an active material
        if( pMaterial == NULL )
            continue;

        if( 0 == wcscmp( strCommand, L"#" ) )
        {
            // Comment
        }
        else if( 0 == wcscmp( strCommand, L"Ka" ) )
        {
            // Ambient color
            float r, g, b;
            InFile >> r >> g >> b;
            pMaterial->vAmbient = D3DXVECTOR3( r, g, b );
        }
        else if( 0 == wcscmp( strCommand, L"Kd" ) )
        {
            // Diffuse color
            float r, g, b;
            InFile >> r >> g >> b;
            pMaterial->vDiffuse = D3DXVECTOR3( r, g, b );
        }
        else if( 0 == wcscmp( strCommand, L"Ks" ) )
        {
            // Specular color
            float r, g, b;
            InFile >> r >> g >> b;
            pMaterial->vSpecular = D3DXVECTOR3( r, g, b );
        }
        else if( 0 == wcscmp( strCommand, L"d" ) ||
                 0 == wcscmp( strCommand, L"Tr" ) )
        {
            // Alpha
            InFile >> pMaterial->fAlpha;
        }
        else if( 0 == wcscmp( strCommand, L"Ns" ) )
        {
            // Shininess
            int nShininess;
            InFile >> nShininess;
            pMaterial->nShininess = nShininess;
        }
        else if( 0 == wcscmp( strCommand, L"illum" ) )
        {
            // Specular on/off
            int illumination;
            InFile >> illumination;
            pMaterial->bSpecular = ( illumination == 2 );
        }
        else if( 0 == wcscmp( strCommand, L"map_Kd" ) )
        {
            // Texture
            InFile >> pMaterial->strDiffuseTex;
        }
		else if (0 == wcscmp( strCommand, L"map_Ks" ))
		{
			InFile >> pMaterial->strSpecularTex;
		}
		else if (0 == wcscmp( strCommand, L"map_bump" ))
		{
			WCHAR buf[100];
			InFile >> buf;
			InFile >> pMaterial->bumpVal;
			InFile >> pMaterial->strNormalTex;
		}

        else
        {
            // Unimplemented or unrecognized command
        }

        InFile.ignore( 1000, L'\n' );
    }

    InFile.close();

    return S_OK;
}


//--------------------------------------------------------------------------------------
void CMeshLoader::InitMaterial( Material* pMaterial )
{
    ZeroMemory( pMaterial, sizeof( Material ) );

    pMaterial->vAmbient = D3DXVECTOR3( 0.2f, 0.2f, 0.2f );
    pMaterial->vDiffuse = D3DXVECTOR3( 0.8f, 0.8f, 0.8f );
    pMaterial->vSpecular = D3DXVECTOR3( 1.0f, 1.0f, 1.0f );
    pMaterial->nShininess = 0;
    pMaterial->fAlpha = 1.0f;
	pMaterial->bSpecular = false;
	pMaterial->pDiffuseTex = NULL;
	pMaterial->pSpecularTex = NULL;
	pMaterial->pNormalTex = NULL;
	pMaterial->bumpVal = 0.3;
	pMaterial->strDiffuseTex[0] = 0;
	pMaterial->strSpecularTex[0] = 0;
	pMaterial->strNormalTex[0] = 0;
}

void CMeshLoader::ComputeTBN( VERTEX& v0, VERTEX& v1, VERTEX& v2 )
{
	D3DXVECTOR3 p10 = v1.position - v0.position;
	D3DXVECTOR3 p20 = v2.position - v0.position;

	D3DXVECTOR2 t10 = v1.texcoord - v0.texcoord;
	D3DXVECTOR2 t20 = v2.texcoord - v0.texcoord;

	float a11 = t10.x, a12 = t10.y;
	float a21 = t20.x, a22 = t20.y;
	float delta = a11 * a22 - a21 * a12;
	float b1, b2, x1, x2;

	D3DXVECTOR3 tangent, bitangent;

	b1 = p10.x; b2 = p20.x;
	x1 = abs(delta) > 1e-6 ? (b1*a22 - b2*a12)/delta : 0;
	x2 = abs(delta) > 1e-6 ? (a11*b2 - a21*b1)/delta : 0;
	tangent.x = x1;
	bitangent.x = x2;

	b1 = p10.y; b2 = p20.y;
	x1 = abs(delta) > 1e-6 ? (b1*a22 - b2*a12)/delta : 0;
	x2 = abs(delta) > 1e-6 ? (a11*b2 - a21*b1)/delta : 0;
	tangent.y = x1;
	bitangent.y = x2;

	b1 = p10.z; b2 = p20.z;
	x1 = abs(delta) > 1e-6 ? (b1*a22 - b2*a12)/delta : 0;
	x2 = abs(delta) > 1e-6 ? (a11*b2 - a21*b1)/delta : 0;
	tangent.z = x1;
	bitangent.z = x2;

	v0.tangent = v1.tangent = v2.tangent = tangent;
	v0.bitangent = v1.bitangent = v2.bitangent = bitangent;
}

void CMeshLoader::FillTBN()
{
	for (int i = 0; i < m_Indices.GetSize(); i+=3)
	{
		DWORD id0 = m_Indices[i];
		DWORD id1 = m_Indices[i+1];
		DWORD id2 = m_Indices[i+2];

		ComputeTBN(m_Vertices[id0], m_Vertices[id1], m_Vertices[id2]);
	}
}
