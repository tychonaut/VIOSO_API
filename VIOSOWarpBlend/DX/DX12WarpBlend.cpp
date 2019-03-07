#include "DX11WarpBlend.h"
#include "pixelshader.h"

//#pragma comment( lib, "d3d11.lib" )

typedef struct ConstantBuffer
{
	FLOAT matView[16];
	FLOAT border[4];
	FLOAT params[4];
} ConstantBuffer;

const FLOAT _black[4] = {0,0,0,1};

DX11WarpBlend::DX11WarpBlend( ID3D11Device* pDevice )
: DXWarpBlend(),
  m_device( pDevice ),
  m_dc( NULL ),
  m_VertexShader(NULL),
  m_VertexBuffer(NULL),
  m_VertexBufferModel(NULL),
  m_IndexBufferModel(NULL),
  m_PixelShader(NULL),
  m_SSClamp(NULL),
  m_SSLin(NULL),
  m_ConstantBuffer(NULL),
  m_texBlend(NULL),
  m_texWarp(NULL),
  m_texBB(NULL),
  m_texWarpCalc(NULL),
  m_RasterState(NULL),
  m_Layout(NULL)
{
	if( NULL == m_device )
		throw( VWB_ERROR_PARAMETER );
	else
	{
		m_device->AddRef();
		m_device->GetImmediateContext( &m_dc );
		if( NULL == m_dc )
			throw( VWB_ERROR_GENERIC );
	}
	m_type4cc = '11XD';
}

DX11WarpBlend::~DX11WarpBlend(void)
{
	SAFERELEASE( m_Layout );
	SAFERELEASE( m_RasterState );
	SAFERELEASE( m_texWarpCalc );
	SAFERELEASE( m_texBB );
	SAFERELEASE( m_texWarp ); 
	SAFERELEASE( m_texBlend );
	SAFERELEASE( m_PixelShader );
	SAFERELEASE( m_VertexShader );
	SAFERELEASE( m_VertexBuffer );
	SAFERELEASE( m_VertexBufferModel );
	SAFERELEASE( m_IndexBufferModel );
	SAFERELEASE( m_SSLin );
	SAFERELEASE( m_SSClamp );
	SAFERELEASE( m_ConstantBuffer );
	SAFERELEASE( m_dc );
	SAFERELEASE( m_device );
	logStr( 1, "INFO: DX11-Warper destroyed.\n" );
}

VWB_ERROR DX11WarpBlend::Init( VWB_WarpBlendSet& wbs )
{
	VWB_ERROR err = VWB_Warper_base::Init( wbs );
	HRESULT hr = E_FAIL;
	if( VWB_ERROR_NONE == err ) try
	{
		VWB_WarpBlend& wb = *wbs[calibIndex];

		ID3D11RenderTargetView* pVV = NULL;
		m_dc->OMGetRenderTargets(1, &pVV, NULL );
		hr = E_FAIL;
		if( pVV )
		{
			ID3D11Resource* pRes = NULL;
			pVV->GetResource( &pRes );
			if( pRes )
			{
				ID3D11Texture2D* pTex;
				if( SUCCEEDED( pRes->QueryInterface( __uuidof( ID3D11Texture2D ), (void**)&pTex ) ) )
				{
					D3D11_TEXTURE2D_DESC desc;
					pTex->GetDesc( &desc );
					pTex->Release();

					m_vp.Width = (FLOAT)desc.Width;
					m_vp.Height = (FLOAT)desc.Height;
					m_vp.TopLeftX = 0;
					m_vp.TopLeftY = 0;
					m_vp.MinDepth = 0.0f;
					m_vp.MaxDepth = 1.0f;
					logStr( 2, "INFO: Output buffer found. Viewport is %.0fx%.0f.\n", m_vp.Width, m_vp.Height );
					hr = S_OK;
				}
				pRes->Release();
			}
			pVV->Release();
		}
		if(FAILED(hr))
		{
			logStr( 0, "ERROR: Output buffer not found.\n" );
			return VWB_ERROR_GENERIC;
		}

		D3D11_TEXTURE2D_DESC descTex = {
			(UINT)m_sizeMap.cx,//UINT Width;
			(UINT)m_sizeMap.cy,//UINT Height;
			1,//UINT MipLevels;
			1,//UINT ArraySize;
			DXGI_FORMAT_R32G32B32A32_FLOAT,//DXGI_FORMAT Format;
			{1,0},//DXGI_SAMPLE_DESC SampleDesc;
			D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
			D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
			0,//UINT CPUAccessFlags;
			0,//UINT MiscFlags;
		};
		D3D11_SUBRESOURCE_DATA dataWarp = {
			wb.pWarp,
			sizeof( VWB_WarpRecord ) * m_sizeMap.cx,
			sizeof( VWB_WarpRecord ) * m_sizeMap.cx * m_sizeMap.cy
		};
		D3D11_SUBRESOURCE_DATA dataBlend = dataWarp;
		dataBlend.pSysMem = wb.pBlend;
		D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
		descSRV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		descSRV.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
		descSRV.Texture2D.MipLevels = 1;
		descSRV.Texture2D.MostDetailedMip = 0;
		ID3D11Texture2D* pTexWarp = NULL, *pTexBlend = NULL;
		if(	FAILED( m_device->CreateTexture2D( &descTex, &dataWarp, &pTexWarp ) ) ||
			FAILED( m_device->CreateTexture2D( &descTex, &dataBlend, &pTexBlend ) ) ||
			FAILED( m_device->CreateShaderResourceView( pTexWarp, &descSRV, &m_texWarp ) ) ||
			FAILED( m_device->CreateShaderResourceView( pTexBlend, &descSRV, &m_texBlend ) ) )
		{
			logStr( 0, "ERROR: Failed to create lookup textures.\n" );
			throw VWB_ERROR_SHADER;
		}
		SAFERELEASE( pTexWarp );
		SAFERELEASE( pTexBlend );

		ID3DBlob* pVSBlob = NULL;
		ID3DBlob* pErrBlob = NULL;
		hr = D3DCompile( s_pixelShaderDX4, sizeof(s_pixelShaderDX4), NULL, NULL, NULL, "VS", "vs_4_0", 0, 0, &pVSBlob, &pErrBlob );
		if( FAILED( hr ) )
		{
			logStr( 0, "ERROR: The vertex shader code cannot be compiled: %s\n", pErrBlob->GetBufferPointer() );
			SAFERELEASE( pErrBlob );
			throw VWB_ERROR_SHADER;
		}

		// Create the vertex shader0, 
		hr = m_device->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_VertexShader );
		if( FAILED( hr ) )
		{	
			pVSBlob->Release();
			logStr( 0, "ERROR: The vertex shader cannot be created: %08X\n", hr );
			throw VWB_ERROR_SHADER;
		}
		FLOAT dx = 0.5f/m_sizeMap.cx;
		FLOAT dy = 0.5f/m_sizeMap.cy;
		SimpleVertex quad[] = { 
			{ {  1.0f + dx,  1.0f + dy, 0.5f }, { 1.0f, 0.0f } },
			{ {  1.0f + dx, -1.0f - dy, 0.5f }, { 1.0f, 1.0f } },
			{ { -1.0f - dx, -1.0f - dy, 0.5f }, { 0.0f, 1.0f } },
			{ { -1.0f - dx, -1.0f - dy, 0.5f }, { 0.0f, 1.0f } },
			{ { -1.0f - dx,  1.0f + dy, 0.5f }, { 0.0f, 0.0f } },
			{ {  1.0f + dx,  1.0f + dy, 0.5f }, { 1.0f, 0.0f } },
		};

		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof( SimpleVertex, Pos ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof( SimpleVertex, Tex ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE( layout );

		// Create the input layout
		hr = m_device->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
												pVSBlob->GetBufferSize(), &m_Layout );
		pVSBlob->Release();
		if( FAILED( hr ) )
		{
			logStr( 0, "ERROR: Could not create shader input layout: %08X\n", hr );
			throw VWB_ERROR_SHADER;
		}

		D3D11_BUFFER_DESC bd = {0};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.ByteWidth = sizeof(quad);
		D3D11_SUBRESOURCE_DATA data = { quad, 0, 0 };
		hr = m_device->CreateBuffer( &bd, &data, &m_VertexBuffer );
		
		if( FAILED( hr ) )
		{
			logStr( 0, "ERROR: Could not create constant buffer: %08X\n", hr );
			throw VWB_ERROR_GENERIC;
		}

		// Turn off culling, so we see the front and back of the triangle
		D3D11_RASTERIZER_DESC rasterDesc;
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D11_CULL_NONE;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 1.0f;
		rasterDesc.DepthClipEnable = false;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = true;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0.0f;
		hr = m_device->CreateRasterizerState( &rasterDesc, &m_RasterState );
		if( FAILED( hr ) )
		{
			logStr( 0, "ERROR: Could not create raster state: %08X\n", hr );
			throw VWB_ERROR_GENERIC;
		}

		// Compile the pixel shader
        std::string pixelShader = "PS"; // or "TST"
#if 1
        if (m_bDynamicEye)
        {
            pixelShader = "PSWB3D";
        }
        else
        {
            pixelShader = "PSWB";
        }
		if( bBicubic )
			pixelShader.append( "BC" );
#endif
		ID3DBlob* pPSBlob = NULL;
		SAFERELEASE( pErrBlob );
		hr = D3DCompile( s_pixelShaderDX4, sizeof(s_pixelShaderDX4), NULL, NULL, NULL, pixelShader.c_str(), "ps_4_0", 0, 0, &pPSBlob, &pErrBlob );
		if( FAILED( hr ) )
		{
			logStr( 0, "ERROR: The pixel shader code cannot be compiled (%08X): %s\n", hr, pErrBlob->GetBufferPointer() );
			SAFERELEASE( pErrBlob );
			throw VWB_ERROR_SHADER;
		}

		// Create the pixel shader
		hr = m_device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_PixelShader );
		pPSBlob->Release();
		if( FAILED( hr ) )
		{	
			logStr( 0, "ERROR: The pixel shader cannot be created: %08X\n", hr );
			throw VWB_ERROR_SHADER;
		}
		
		// Create the constant buffer
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.ByteWidth = sizeof(ConstantBuffer);
		hr = m_device->CreateBuffer( &bd, NULL, &m_ConstantBuffer );
		if( FAILED( hr ) )
		{
			logStr( 0, "ERROR: Could not creare constant buffer: %08X\n", hr );
			throw VWB_ERROR_GENERIC;
		}

		D3D11_SAMPLER_DESC descSam = {
			D3D11_FILTER_MIN_MAG_MIP_LINEAR, //D3D11_FILTER Filter;
			D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressU;
			D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressV;
			D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressW;
			0, //FLOAT MipLODBias;
			1, //UINT MaxAnisotropy;
			D3D11_COMPARISON_NEVER, //D3D11_COMPARISON_FUNC ComparisonFunc;
			{0,0,0,0}, //FLOAT BorderColor[ 4 ];
			-FLT_MAX, //FLOAT MinLOD;
			FLT_MAX, //FLOAT MaxLOD;
		};
		m_device->CreateSamplerState( &descSam, &m_SSLin );
		descSam.AddressU = descSam.AddressV = descSam.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		m_device->CreateSamplerState( &descSam, &m_SSClamp );

		logStr( 1, "SUCCESS: DX11-Warper initialized.\n" );

		// transpose view matrices
		m_mBaseI.Transpose();
		m_mViewIG.Transpose();
	} catch( VWB_ERROR e )
	{
		err = e;
	}

	return err;
}

VWB_ERROR DX11WarpBlend::Render( VWB_param inputTexture, VWB_uint stateMask )
{
	HRESULT res = E_FAIL;
	if( VWB_STATEMASK_STANDARD == stateMask )
		stateMask = VWB_STATEMASK_DEFAULT;

	if( NULL == m_PixelShader || NULL == m_device )
		return VWB_ERROR_GENERIC;

	ID3D11DepthStencilView* pDSV = NULL;
	ID3D11RenderTargetView* pRTV = NULL;
	m_dc->OMGetRenderTargets( 1, &pRTV, &pDSV );

	// do backbuffer copy if necessary
	if( NULL == inputTexture )
	{
		if( pRTV )
		{
			ID3D11Resource* pRes = NULL;
			pRTV->GetResource( &pRes );
			if( pRes )
			{
				ID3D11Texture2D* pTex;
				if( SUCCEEDED( pRes->QueryInterface( __uuidof( ID3D11Texture2D ), (void**)&pTex ) ) )
				{
					D3D11_TEXTURE2D_DESC desc;
					pTex->GetDesc( &desc );
					pTex->Release();
					if( NULL == m_texBB ||
						desc.Width != m_sizeIn.cx ||
						desc.Height != m_sizeIn.cy )
					{
						pTex = NULL;
						desc.Usage = D3D11_USAGE_DEFAULT;
						desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
						desc.CPUAccessFlags = 0;
						desc.ArraySize = 1;
						desc.MipLevels = 1;
						desc.MiscFlags = 0;
						res = m_device->CreateTexture2D( &desc, NULL, &pTex );
						m_sizeIn.cx = desc.Width;
						m_sizeIn.cy = desc.Height;
						if( SUCCEEDED( res ) )
						{
							D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
							descSRV.Format = desc.Format;
							descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
							descSRV.Texture2D.MipLevels= 1;
							descSRV.Texture2D.MostDetailedMip = 0;
							m_device->CreateShaderResourceView( (ID3D11Resource*)pTex, &descSRV, &m_texBB );
							pTex->Release();
						}
						else
							return VWB_ERROR_GENERIC;
					}
				}
				else
					return VWB_ERROR_GENERIC;

				if( NULL != m_texBB )
				{
					ID3D11Resource* pResDst = NULL;
					m_texBB->GetResource( &pResDst );
					m_dc->CopyResource( pResDst, pRes );
					SAFERELEASE( pResDst );
				}
				pRes->Release();
			}
			else
				return VWB_ERROR_GENERIC;
		}
		else
			return VWB_ERROR_GENERIC;
	}
	else
	{
		ID3D11Texture2D* pResIn = (ID3D11Texture2D*)inputTexture;

		if( NULL != m_texBB )
		{
			ID3D11Resource* pRes = NULL;
			m_texBB->GetResource( &pRes );
			if( pRes != pResIn )
			{
				m_texBB->Release();
				m_texBB = NULL;
			}
			SAFERELEASE( pRes );
		}
		if( NULL == m_texBB )
		{
			D3D11_TEXTURE2D_DESC descTex;
			pResIn->GetDesc( &descTex );
			D3D11_SHADER_RESOURCE_VIEW_DESC desc;
			desc.Format = descTex.Format;
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipLevels = 1;
			desc.Texture2D.MostDetailedMip = 0;

			res = m_device->CreateShaderResourceView( pResIn, &desc, &m_texBB );
            if (FAILED(res))
                return VWB_ERROR_GENERIC;

			m_sizeIn.cx = descTex.Width;
			m_sizeIn.cy = descTex.Height;
		}
	}

/////////////// save state
	ID3D11Buffer* pOldVtx = NULL;
	UINT oldStride = 0;
	UINT oldOffset = 0;
	ID3D11InputLayout* pOldLayout = NULL;
	D3D11_PRIMITIVE_TOPOLOGY oldTopo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	ID3D11RasterizerState* pOldRS = NULL;

	ID3D11VertexShader* pOldVS = NULL;
	ID3D11Buffer* pOldCB = NULL;
	ID3D11PixelShader* pOldPS = NULL;
	ID3D11ShaderResourceView* ppOldSRV[3] = {0};
	ID3D11SamplerState* ppOldSS[3] = {0};

	if( VWB_STATEMASK_VERTEX_BUFFER & stateMask )
		m_dc->IAGetVertexBuffers( 0, 1, &pOldVtx, &oldStride, &oldOffset );
	if( VWB_STATEMASK_INPUT_LAYOUT & stateMask )
		m_dc->IAGetInputLayout( &pOldLayout );
	if( VWB_STATEMASK_PRIMITIVE_TOPOLOGY & stateMask )
	    m_dc->IAGetPrimitiveTopology( &oldTopo );
	if( VWB_STATEMASK_RASTERSTATE & stateMask )
		m_dc->RSGetState( &pOldRS );

	if( VWB_STATEMASK_VERTEX_SHADER & stateMask )
		m_dc->VSGetShader( &pOldVS, NULL, NULL );
	if( VWB_STATEMASK_PIXEL_SHADER & stateMask )
		m_dc->PSGetShader( &pOldPS, NULL, NULL );
	if( VWB_STATEMASK_SHADER_RESOURCE & stateMask )
		m_dc->PSGetShaderResources( 0, 3, ppOldSRV );
	if( VWB_STATEMASK_SAMPLER & stateMask )
		m_dc->PSGetSamplers( 0, 3, ppOldSS );
	if( VWB_STATEMASK_CONSTANT_BUFFER & stateMask )
		m_dc->PSGetConstantBuffers( 0, 1, &pOldCB );

////////////// set state
	UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    m_dc->IASetVertexBuffers( 0, 1, &m_VertexBuffer, &stride, &offset );
	m_dc->IASetInputLayout( m_Layout );
    m_dc->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	m_dc->VSSetShader( m_VertexShader, NULL, 0 );

	m_dc->RSSetState( m_RasterState );

	ConstantBuffer cb;
	memcpy( cb.matView, m_mVP.Transposed(), sizeof( cb.matView ) );
	cb.border[0] = m_bBorder;
	cb.border[1] = bDoNotBlend ? 0.0f : 1.0f;
	cb.params[0] = (FLOAT)m_sizeIn.cx;
	cb.params[1] = (FLOAT)m_sizeIn.cy;
	cb.params[2] = 1.0f/(FLOAT)m_sizeIn.cx;
	cb.params[3] = 1.0f/(FLOAT)m_sizeIn.cy;
	m_dc->PSSetShader( m_PixelShader, NULL, 0 );
	m_dc->UpdateSubresource( m_ConstantBuffer, 0, NULL, &cb, 0, 0 );
	m_dc->PSSetConstantBuffers( 0, 1, &m_ConstantBuffer );
	m_dc->PSSetShaderResources( 0, 1, &m_texBB );
	m_dc->PSSetShaderResources( 1, 1, &m_texWarp );
	m_dc->PSSetShaderResources( 2, 1, &m_texBlend );
	m_dc->PSSetSamplers( 0, 1, &m_SSLin );
	m_dc->PSSetSamplers( 1, 1, &m_SSClamp );
	m_dc->PSSetSamplers( 2, 1, &m_SSLin );

////////////// draw
	if( pDSV )
	{
		m_dc->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0 );
		pDSV->Release();
	}
	if( pRTV )
	{
		m_dc->ClearRenderTargetView( pRTV, _black );
		pRTV->Release();
	}
	m_dc->Draw( 6, 0 );
	res = S_OK;

/////////// restore state
	if( VWB_STATEMASK_CONSTANT_BUFFER & stateMask )
		m_dc->PSSetConstantBuffers( 0, 1, &pOldCB );

	if( VWB_STATEMASK_SAMPLER & stateMask )
	{
		m_dc->PSSetSamplers( 0, 3, ppOldSS );
		SAFERELEASE( ppOldSS[0] );
		SAFERELEASE( ppOldSS[1] );
		SAFERELEASE( ppOldSS[2] );
	}

	if( VWB_STATEMASK_SHADER_RESOURCE & stateMask )
	{
		m_dc->PSSetShaderResources( 0, 3, ppOldSRV );
		SAFERELEASE( ppOldSRV[0] );
		SAFERELEASE( ppOldSRV[1] );
		SAFERELEASE( ppOldSRV[2] );
	}

	if( VWB_STATEMASK_PIXEL_SHADER & stateMask )
		if( pOldPS )
		{
			m_dc->PSSetShader( pOldPS, NULL, NULL );
			pOldPS->Release();
		}

	if( VWB_STATEMASK_VERTEX_SHADER & stateMask )
		if( pOldVS )
		{
			m_dc->VSSetShader( pOldVS, NULL, NULL );
			pOldVS->Release();
		}

	if( VWB_STATEMASK_RASTERSTATE & stateMask )
		if( pOldRS )
		{
			m_dc->RSSetState( pOldRS );
			pOldRS->Release();
		}

	if( VWB_STATEMASK_PRIMITIVE_TOPOLOGY & stateMask )
		m_dc->IASetPrimitiveTopology( oldTopo );

	if( VWB_STATEMASK_INPUT_LAYOUT & stateMask )
		if( pOldLayout )
		{
			m_dc->IASetInputLayout( pOldLayout );
			pOldLayout->Release();
		}

	if( VWB_STATEMASK_VERTEX_BUFFER & stateMask )
		if( pOldVtx )
		{
			m_dc->IASetVertexBuffers( 0, 1, &pOldVtx, &oldStride, &oldOffset );
			pOldVtx->Release();
		}

	return SUCCEEDED(res) ? VWB_ERROR_NONE : VWB_ERROR_GENERIC;
}