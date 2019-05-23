static char s_pixelShaderDX2a[] ="\n"
"sampler samContent : register(s0);            \n"
"sampler samWarp : register(s1);               \n"
"sampler samBlend : register(s2);              \n"
"sampler samCur : register(s3);              \n" // cur texture, rgb for color; if alpha<=0,5 2*alpha is for blending, bigger will invert pixel
"                                                \n"
"float4x4 matView : register(c0);                \n"
"float4 bBorder : register(c4);                  \n" // bBorder.x > 0.5 = border on, else off; bBorder.y > 0.5 = blend on, else off
"float4 params : register(c5);					 \n"  //x.. content width, y .. content height, z = 1/content width, w = 1/content height
"float4 offsScale : register(c6);				 \n"  //x.. offset x, y .. offset y, z = scale X, w = scale Y  (u',v')=( (u-x)*z, (v-y)*w )
"float4 offsScaleCur : register(c7);				\n"  //x.. offset x, y .. offset y, z = scale X, w = scale Y  (u',v')=( (u-x)*z, (v-y)*w )
"                                                \n"
"struct VS_OUT {                                 \n"
"    float4 pos : POSITION;                   \n"
"    float2 tex : TEXCOORD0;                     \n"
"};                  \n"
"																\n"
"                                                \n"
"//-------------------------------------------------------------\n"
"// Pixel Shaders												\n"
"//-------------------------------------------------------------\n"
"                                                \n"
"float4 tex2DBC(uniform sampler   texCnt,\n"
"               float2            vPos)\n"
"{\n"
"	vPos*= params.xy;\n"
"	float2 t = floor( vPos - 0.5 ) + 0.5;\n" // the nearest pixel
"	float2 w0 = 1;\n"
"	float2 w1 = vPos - t;\n"
"	float2 w2 = w1 * w1;\n"
"	float2 w3 = w2 * w1;\n"
"\n"
"	w0 = w2 - 0.5 * (w3 + w1);\n"
"	w1 = 1.5 * w3 - 2.5 * w2 + 1.0;\n"
"	w3 = 0.5 * (w3 - w2);\n"
"	w2 = 1.0 - w0 - w1 - w3;\n"
"\n"
"	float2 s0 = w0 + w1;\n"
"	float2 s1 = w2 + w3;\n"
"	float2 f0 = w1 / s0;\n"
"	float2 f1 = w3 / s1;\n"
"\n"
"	float2 t0 = t - 1 + f0;\n"
"	float2 t1 = t + 1 + f1;\n"
"	t0*= params.zw;\n"
"	t1*= params.zw;\n"
"\n"
"	return\n"
"		( tex2D( texCnt, t0 ) * s0.x +\n"
"		  tex2D( texCnt, float2( t1.x, t0.y ) ) * s1.x ) * s0.y +\n"
"		( tex2D( texCnt, float2( t0.x, t1.y ) ) * s0.x +\n"
"		  tex2D( texCnt, t1 ) * s1.x ) * s1.y;\n"
"}\n"
"                                                \n"
"float4 PS( VS_OUT vIn ) : COLOR                 \n"
"{                                               \n"
"    float4 color = tex2D( samContent, ( vIn.tex.xy + offsScale.xy ) * offsScale.zw ); \n"
"    return color;                               \n"
"}                                               \n"
"                                                \n"
"float4 PSWB( VS_OUT vIn ) : COLOR               \n"
"{                                               \n"
"	float4 tex = tex2D( samWarp, vIn.tex );      \n"
"	float4 blend = tex2D( samBlend, vIn.tex );   \n"
"	float4 vOut = 0;\n"
"	float4 vCur = 0;\n"
"	if( 0.1 < tex.z )\n"
"	{\n"
"		if( bBorder.x > 0.5 )                      \n"
"		{                                           \n"
"		    tex.x*= 1.02;                           \n"
"		    tex.x-= 0.01;                           \n"
"		    tex.y*= 1.02;                           \n"
"		    tex.y-= 0.01;                           \n"
"		}                                           \n"
"		tex.xy/= tex.z;\n"
"       vOut = tex2D( samContent, ( tex.xy - offsScale.xy ) * offsScale.zw ); \n"
"		vCur = tex2D( samCur, ( tex.xy - offsScaleCur.xy ) * offsScaleCur.zw );  \n"
"		vOut.rgb = vCur.a * vCur.rgb + vOut.rgb * ( 1.0 - vCur.a );\n"
"		if( bBorder.y > 0.5 )                      \n"
"			vOut.rgb*= blend.rgb;			        \n"
"	}\n"
"	vOut.a = 1;                                  \n"
"	return vOut;                                 \n"
"}                                               \n"
"                                                \n"
"float4 PSWB3D( VS_OUT vIn ) : COLOR             \n"
"{                                               \n"
"	float4 tex = tex2D( samWarp, vIn.tex );      \n"
"	float4 blend = tex2D( samBlend, vIn.tex );   \n"
"	blend.a = 1;								 \n"
"	float4 vOut = float4( 0,0,0,1);              \n"
"	float4 vCur = 0;\n"
"	if( 0.1 < tex.w )                            \n"
"	{                                            \n"
"		tex/= tex.w;                             \n"
"		tex = mul( tex, matView );               \n"
"		tex.xy/= tex.w;                          \n"
"		tex.x/=2;                                \n"
"		tex.y/=-2;                               \n"
"		tex.xy+= 0.5;                           \n"
"       vOut = tex2D( samContent, ( tex.xy - offsScale.xy ) * offsScale.zw ); \n"
"		vCur = tex2D( samCur, ( tex.xy - offsScaleCur.xy ) * offsScaleCur.zw );  \n"
"		vOut.rgb = vCur.a * vCur.rgb + vOut.rgb * ( 1.0 - vCur.a );\n"
"		if( bBorder.y > 0.5 )                      \n"
"			vOut.rgb*= blend.rgb;			        \n"
"	}                                           \n"
"	vOut.a = 1;                                \n"
"	return vOut;                                \n"
"}                                               \n"
"\n"
"float4 PSBC( VS_OUT vIn ) : COLOR                 \n"
"{                                               \n"
"    float4 color = tex2DBC(samContent, ( vIn.tex.xy - offsScale.xy ) * offsScale.zw ); \n"
"    return color;                               \n"
"}                                               \n"
"                                                \n"
"float4 PSWBBC( VS_OUT vIn ) : COLOR               \n"
"{                                               \n"
"	vIn.tex-= offsScale.xy;                      \n"
"	vIn.tex*= offsScale.zw;                      \n"
"	float4 tex = tex2D( samWarp, vIn.tex );     \n"
"	float4 blend = tex2D( samBlend, vIn.tex );  \n"
"	float4 vOut = 0;\n"
"	float4 vCur = 0;\n"
"	if( 0.1 < tex.z )\n"
"	{\n"
"		if( bBorder.x > 0.5 )                      \n"
"		{                                           \n"
"		    tex.x*= 1.02;                           \n"
"		    tex.x-= 0.01;                           \n"
"		    tex.y*= 1.02;                           \n"
"		    tex.y-= 0.01;                           \n"
"		}                                           \n"
"		tex.xy/= tex.z;\n"
"       vOut = tex2DBC( samContent, ( tex.xy - offsScale.xy ) * offsScale.zw ); \n"
"		vCur = tex2D( samCur, ( tex.xy - offsScaleCur.xy ) * offsScaleCur.zw );  \n"
"		vOut.rgb = vCur.a * vCur.rgb + vOut.rgb * ( 1.0 - vCur.a );\n"
"		if( bBorder.y > 0.5 )                      \n"
"			vOut.rgb*= blend.rgb;			        \n"
"	}\n"	
"	vOut.a = 1;                                 \n"
"	return vOut;                                \n"
"}                                               \n"
"                                                \n"
"float4 PSWB3DBC( VS_OUT vIn ) : COLOR             \n"
"{                                               \n"
"	float4 tex = tex2D( samWarp, vIn.tex );     \n"
"	float4 blend = tex2D( samBlend, vIn.tex );  \n"
"	blend.a = 1;  \n"
"	float4 vOut = float4( 0,0,0,1);             \n"
"	float4 vCur = 0;\n"
"	if( 0.1 < tex.w && ( 0 < blend.r || 0 < blend.g || 0 < blend.b ) )                            \n"
"	{                                           \n"
"		tex/= tex.w;                            \n"
"		tex = mul( tex, matView );              \n"
"		tex.xy/= tex.w;                         \n"
"		tex.x/=2;                              \n"
"		tex.y/=-2;                               \n"
"		tex.xy+= 0.5;                           \n"
"       vOut = tex2DBC( samContent, ( tex.xy - offsScale.xy ) * offsScale.zw ); \n"
"		vCur = tex2D( samCur, ( tex.xy - offsScaleCur.xy ) * offsScaleCur.zw );  \n"
"		vOut.rgb = vCur.a * vCur.rgb + vOut.rgb * ( 1.0 - vCur.a );\n"
"		if( bBorder.y > 0.5 )                      \n"
"			vOut.rgb*= blend.rgb;			        \n"
"	}                                           \n"
"	vOut.a = 1;                                \n"
"	return vOut;                                \n"
"}                                               \n";

static char s_pixelShaderDX4[] = "\n"
"Texture2D texContent : register(t0);            \n"
"Texture2D texWarp : register(t1);               \n"
"Texture2D texBlend : register(t2);              \n"
"Texture2D texCur : register(t3);              \n"
"                                                \n"
"cbuffer ConstantBuffer : register( b0 )                     \n"
"{																\n"
"	float4x4 matView;							\n"
"	float4 bBorder;								\n"
"	float4 params;								\n"  //x.. content width, y .. content height, z = 1/content width, w = 1/content height
"	float4 offsScale;	            			\n"  //x.. offset x, y .. offset y, z = scale X, w = scale Y  (u',v')=( (u-x)*z, (v-y)*w )
"	float4 offsScaleCur;						\n"  //x.. offset x, y .. offset y, z = scale X, w = scale Y  (u',v')=( (u-x)*z, (v-y)*w )
"}												\n"
"                                                \n"
"sampler samLin : register( s0 );               \n"
"sampler samWarp : register( s1 );               \n"
"                                                \n"
"//-------------------------------------------------------------\n"
"struct VS_INPUT												\n"
"{																\n"
"    float4 Pos : POSITION;										\n"
"    float2 Tex : TEXCOORD0;									\n"
"};																\n"
"																\n"
"struct VS_OUT {                                 \n"
"    float4 pos : SV_Position;                   \n"
"    float2 tex : TEXCOORD0;                     \n"
"};                  \n"
"																\n"
"//-------------------------------------------------------------\n"
"// Vertex Shader												\n"
"//-------------------------------------------------------------\n"
"VS_OUT VS( VS_INPUT input )									\n"
"{																\n"
"    VS_OUT output = (VS_OUT)0;								\n"
"    output.pos = input.Pos;									\n"
"    output.tex = input.Tex;								    \n"
"    return output;												\n"
"}																\n"
"                                                \n"
"//-------------------------------------------------------------\n"
"// Pixel Shaders												\n"
"//-------------------------------------------------------------\n"
"float4 tex2DBC(uniform Texture2D texCnt,\n"
"               float2            vPos)\n"
"{\n"
"	vPos*= params.xy;\n"
"	float2 t = floor( vPos - 0.5 ) + 0.5;\n" // the nearest pixel
"	float2 w0 = 1;\n"
"	float2 w1 = vPos - t;\n"
"	float2 w2 = w1 * w1;\n"
"	float2 w3 = w2 * w1;\n"
"\n"
"	w0 = w2 - 0.5 * (w3 + w1);\n"
"	w1 = 1.5 * w3 - 2.5 * w2 + 1.0;\n"
"	w3 = 0.5 * (w3 - w2);\n"
"	w2 = 1.0 - w0 - w1 - w3;\n"
"\n"
"	float2 s0 = w0 + w1;\n"
"	float2 s1 = w2 + w3;\n"
"	float2 f0 = w1 / s0;\n"
"	float2 f1 = w3 / s1;\n"
"\n"
"	float2 t0 = t - 1 + f0;\n"
"	float2 t1 = t + 1 + f1;\n"
"	t0*= params.zw;\n"
"	t1*= params.zw;\n"
"\n"
"	return\n"
"		( texCnt.Sample( samLin, t0 ) * s0.x +\n"
"		  texCnt.Sample( samLin, float2( t1.x, t0.y ) ) * s1.x ) * s0.y +\n"
"		( texCnt.Sample( samLin, float2( t0.x, t1.y ) ) * s0.x +\n"
"		  texCnt.Sample( samLin, t1 ) * s1.x ) * s1.y;\n"
"}\n"
"                                                \n"
"float4 PS( VS_OUT vIn ) : SV_Target                 \n"
"{                                               \n"
"	 float4 color = texContent.Sample( samLin, ( vIn.tex - offsScale.xy ) * offsScale.zw ); \n"
"    return color;                               \n"
"}                                               \n"
"                                                \n"
"float4 PSWB( VS_OUT vIn ) : SV_Target               \n"
"{                                               \n"
"	float4 tex = texWarp.Sample( samWarp, vIn.tex );     \n"
"	float4 blend = texBlend.Sample( samLin, vIn.tex );  \n"
"	float4 vOut = 0;\n"
"	float4 vCur = 0;\n"
"	if( 0.1 < tex.z )\n"
"	{\n"
"		if( bBorder.x > 0.5 )                      \n"
"		{                                           \n"
"		    tex.x*= 1.02;                           \n"
"		    tex.x-= 0.01;                           \n"
"		    tex.y*= 1.02;                           \n"
"		    tex.y-= 0.01;                           \n"
"		}                                           \n"
"		tex.xy/= tex.z;\n"
"		vOut = texContent.Sample( samLin, ( tex.xy - offsScale.xy ) * offsScale.zw );  \n"
"		vCur = texCur.Sample( samLin, ( tex.xy - offsScaleCur.xy ) * offsScaleCur.zw );  \n"
"		vOut.rgb = vCur.a * vCur.rgb + vOut.rgb * ( 1.0 - vCur.a );\n"
"		if( bBorder.y > 0.5 )                      \n"
"			vOut.rgb*= blend.rgb;			        \n"
"	}\n"
"	vOut.a = 1;                                 \n"
"	return vOut;                                \n"
"}                                               \n"
"                                                \n"
"float4 PSWB3D( VS_OUT vIn ) : SV_Target             \n"
"{                                               \n"
"	float4 tex = texWarp.Sample( samWarp, vIn.tex );     \n"
"	float4 blend = texBlend.Sample( samLin, vIn.tex );  \n"
"	blend.a = 1;  \n"
"	float4 vOut = float4( 0,0,0,1);             \n"
"	float4 vCur = 0;\n"
"	if( 0.1 < tex.w )                            \n"
"	{                                           \n"
"		tex/= tex.w;                            \n"
"		tex = mul( tex, matView );              \n"
"		tex.xy/= tex.w;                         \n"
"		tex.x/=2;                              \n"
"		tex.y/=-2;                               \n"
"		tex.xy+= 0.5;                           \n"
"		vOut = texContent.Sample( samLin, ( tex.xy - offsScale.xy ) * offsScale.zw );  \n"
"		vCur = texCur.Sample( samLin, ( tex.xy - offsScaleCur.xy ) * offsScaleCur.zw );  \n"
"		vOut.rgb = vCur.a * vCur.rgb + vOut.rgb * ( 1.0 - vCur.a );\n"
"		if( bBorder.y > 0.5 )                      \n"
"			vOut.rgb*= blend.rgb;			        \n"
"	}                                           \n"
"	vOut.a = 1;                                \n"
"	return vOut;                                \n"
"}                                               \n"
"\n"
"float4 PSBC( VS_OUT vIn ) : SV_Target                 \n"
"{                                               \n"
"    float4 color = tex2DBC( texContent, ( vIn.tex - offsScale.xy ) * offsScale.zw ); \n"
"    return color;                               \n"
"}                                               \n"
"                                                \n"
"float4 PSWBBC( VS_OUT vIn ) : SV_Target               \n"
"{                                               \n"
"	float4 tex = texWarp.Sample( samWarp, vIn.tex );     \n"
"	float4 blend = texBlend.Sample( samLin, vIn.tex );  \n"
"	float4 vOut = 0;\n"
"	float4 vCur = 0;\n"
"	if( 0.1 < tex.z )\n"
"	{\n"
"		if( bBorder.x > 0.5 )                      \n"
"		{                                           \n"
"		    tex.x*= 1.02;                           \n"
"		    tex.x-= 0.01;                           \n"
"		    tex.y*= 1.02;                           \n"
"		    tex.y-= 0.01;                           \n"
"		}                                           \n"
"		tex.xy/= tex.z;\n"
"		vOut = tex2DBC( texContent, ( tex.xy - offsScale.xy ) * offsScale.zw );  \n"
"		vCur = texCur.Sample( samLin, ( tex.xy - offsScaleCur.xy ) * offsScaleCur.zw );  \n"
"		vOut.rgb = vCur.a * vCur.rgb + vOut.rgb * ( 1.0 - vCur.a );\n"
"		if( bBorder.y > 0.5 )                      \n"
"			vOut.rgb*= blend.rgb;			        \n"
"	}\n"
"	vOut.a = 1;                                 \n"
"	return vOut;                                \n"
"}                                               \n"
"                                                \n"
"float4 PSWB3DBC( VS_OUT vIn ) : SV_Target             \n"
"{                                               \n"
"	float4 tex = texWarp.Sample( samWarp, vIn.tex );     \n"
"	float4 blend = texBlend.Sample( samLin, vIn.tex );  \n"
"	blend.a = 1;  \n"
"	float4 vOut = float4( 0,0,0,1);             \n"
"	float4 vCur = 0;\n"
"	if( 0.1 < tex.w && ( 0 < blend.r || 0 < blend.g || 0 < blend.b ) )                            \n"
"	{                                           \n"
"		tex/= tex.w;                            \n"
"		tex = mul( tex, matView );              \n"
"		tex.xy/= tex.w;                         \n"
"		tex.x/=2;                              \n"
"		tex.y/=-2;                               \n"
"		tex.xy+= 0.5;                           \n"
"		vOut = tex2DBC( texContent, ( tex.xy - offsScale.xy ) * offsScale.zw ); \n"
"		vCur = texCur.Sample( samLin, (tex.xy - offsScaleCur.xy) * offsScaleCur.zw );  \n"
"		vOut.rgb = vCur.a * vCur.rgb + vOut.rgb * ( 1.0 - vCur.a );\n"
"		if( bBorder.y > 0.5 )                      \n"
"			vOut.rgb*= blend.rgb;			        \n"
"	}                                           \n"
"	vOut.a = 1;                                \n"
"	return vOut;                                \n"
"}                                               \n";








































static char s_pixelShaderDX4_vFlip[] = "\n"
"Texture2D texContent : register(t0);            \n"
"Texture2D texWarp : register(t1);               \n"
"Texture2D texBlend : register(t2);              \n"
"Texture2D texCur : register(t3);              \n"
"                                                \n"
"cbuffer ConstantBuffer : register( b0 )                     \n"
"{																\n"
"	float4x4 matView;							\n"
"	float4 bBorder;								\n"
"	float4 params;								\n"  //x.. content width, y .. content height, z = 1/content width, w = 1/content height
"	float4 offsScale;	            			\n"  //x.. offset x, y .. offset y, z = scale X, w = scale Y  (u',v')=( (u-x)*z, (v-y)*w )
"	float4 offsScaleCur;						\n"  //x.. offset x, y .. offset y, z = scale X, w = scale Y  (u',v')=( (u-x)*z, (v-y)*w )
"}												\n"
"                                                \n"
"sampler samLin : register( s0 );               \n"
"sampler samWarp : register( s1 );               \n"
"                                                \n"
"//-------------------------------------------------------------\n"
"struct VS_INPUT												\n"
"{																\n"
"    float4 Pos : POSITION;										\n"
"    float2 Tex : TEXCOORD0;									\n"
"};																\n"
"																\n"
"struct VS_OUT {                                 \n"
"    float4 pos : SV_Position;                   \n"
"    float2 tex : TEXCOORD0;                     \n"
"};                  \n"
"																\n"
"//-------------------------------------------------------------\n"
"// Vertex Shader												\n"
"//-------------------------------------------------------------\n"
"VS_OUT VS( VS_INPUT input )									\n"
"{																\n"
"    VS_OUT output = (VS_OUT)0;								\n"
"    output.pos = input.Pos;									\n"
//"    output.tex = float2( input.Tex.x, 1 - input.Tex.y );	    \n"
"    output.tex = input.Tex;	    \n"
"    return output;												\n"
"}																\n"
"                                                \n"
"//-------------------------------------------------------------\n"
"// Pixel Shaders												\n"
"//-------------------------------------------------------------\n"
"float4 tex2DBC(uniform Texture2D texCnt,\n"
"               float2            vPos)\n"
"{\n"
"	vPos*= params.xy;\n"
"	float2 t = floor( vPos - 0.5 ) + 0.5;\n" // the nearest pixel
"	float2 w0 = 1;\n"
"	float2 w1 = vPos - t;\n"
"	float2 w2 = w1 * w1;\n"
"	float2 w3 = w2 * w1;\n"
"\n"
"	w0 = w2 - 0.5 * (w3 + w1);\n"
"	w1 = 1.5 * w3 - 2.5 * w2 + 1.0;\n"
"	w3 = 0.5 * (w3 - w2);\n"
"	w2 = 1.0 - w0 - w1 - w3;\n"
"\n"
"	float2 s0 = w0 + w1;\n"
"	float2 s1 = w2 + w3;\n"
"	float2 f0 = w1 / s0;\n"
"	float2 f1 = w3 / s1;\n"
"\n"
"	float2 t0 = t - 1 + f0;\n"
"	float2 t1 = t + 1 + f1;\n"
"	t0*= params.zw;\n"
"	t1*= params.zw;\n"
"\n"
"	return\n"
"		( texCnt.Sample( samLin, t0 ) * s0.x +\n"
"		  texCnt.Sample( samLin, float2( t1.x, t0.y ) ) * s1.x ) * s0.y +\n"
"		( texCnt.Sample( samLin, float2( t0.x, t1.y ) ) * s0.x +\n"
"		  texCnt.Sample( samLin, t1 ) * s1.x ) * s1.y;\n"
"}\n"
"                                                \n"
"float4 PS( VS_OUT vIn ) : SV_Target                 \n"
"{                                               \n"
"	 float4 color = texContent.Sample( samLin, ( vIn.tex - offsScale.xy ) * offsScale.zw ); \n"
"    return color;                               \n"
"}                                               \n"
"                                                \n"
"float4 PSWB( VS_OUT vIn ) : SV_Target               \n"
"{                                               \n"
"	float4 tex = texWarp.Sample( samWarp, vIn.tex );     \n"
"	float4 blend = texBlend.Sample( samLin, vIn.tex );  \n"
"	float4 vOut = 0;\n"
"	float4 vCur = 0;\n"
"	if( 0.1 < tex.z )\n"
"	{\n"
"		if( bBorder.x > 0.5 )                      \n"
"		{                                           \n"
"		    tex.x*= 1.02;                           \n"
"		    tex.x-= 0.01;                           \n"
"		    tex.y*= 1.02;                           \n"
"		    tex.y-= 0.01;                           \n"
"		}                                           \n"
"		tex.xy/= tex.z;\n"
"		tex.y = 1 - tex.y;\n"
"		vOut = texContent.Sample( samLin, ( tex.xy - offsScale.xy ) * offsScale.zw );  \n"
"		vCur = texCur.Sample( samLin, ( tex.xy - offsScaleCur.xy ) * offsScaleCur.zw );  \n"
"		vOut.rgb = vCur.a * vCur.rgb + vOut.rgb * ( 1.0 - vCur.a );\n"
"		if( bBorder.y > 0.5 )                      \n"
"			vOut.rgb*= blend.rgb;			        \n"
"	}\n"
//"   float4 vOut = texWarp.Sample( samWarp, vIn.tex );\n"
"	vOut.a = 1;                                 \n"
"	return vOut;                                \n"
"}                                               \n"
"                                                \n"
"float4 PSWB3D( VS_OUT vIn ) : SV_Target             \n"
"{                                               \n"
"	float4 tex = texWarp.Sample( samWarp, vIn.tex );     \n"
"	float4 blend = texBlend.Sample( samLin, vIn.tex );  \n"
"	blend.a = 1;  \n"
"	float4 vOut = float4( 0,0,0,1);             \n"
"	float4 vCur = 0;\n"
"	if( 0.1 < tex.w )                            \n"
"	{                                           \n"
"		tex/= tex.w;                            \n"
"		tex = mul( tex, matView );              \n"
"		tex.xy/= tex.w;                         \n"
"		tex.x/=2;                              \n"
"		tex.y/=2;                               \n"
"		tex.xy+= 0.5;                           \n"
"		vOut = texContent.Sample( samLin, ( tex.xy - offsScale.xy ) * offsScale.zw );  \n"
"		vCur = texCur.Sample( samLin, ( tex.xy - offsScaleCur.xy ) * offsScaleCur.zw );  \n"
"		vOut.rgb = vCur.a * vCur.rgb + vOut.rgb * ( 1.0 - vCur.a );\n"
"		if( bBorder.y > 0.5 )                      \n"
"			vOut.rgb*= blend.rgb;			        \n"
"	}                                           \n"
"	vOut.a = 1;                                \n"
"	return vOut;                                \n"
"}                                               \n"
"\n"
"float4 PSBC( VS_OUT vIn ) : SV_Target                 \n"
"{                                               \n"
"    float4 color = tex2DBC( texContent, ( vIn.tex - offsScale.xy ) * offsScale.zw ); \n"
"    return color;                               \n"
"}                                               \n"
"                                                \n"
"float4 PSWBBC( VS_OUT vIn ) : SV_Target               \n"
"{                                               \n"
"	float4 tex = texWarp.Sample( samWarp, vIn.tex );     \n"
"	float4 blend = texBlend.Sample( samLin, vIn.tex );  \n"
"	float4 vOut = 0;\n"
"	float4 vCur = 0;\n"
"	if( 0.1 < tex.z )\n"
"	{\n"
"		if( bBorder.x > 0.5 )                      \n"
"		{                                           \n"
"		    tex.x*= 1.02;                           \n"
"		    tex.x-= 0.01;                           \n"
"		    tex.y*= 1.02;                           \n"
"		    tex.y-= 0.01;                           \n"
"		}                                           \n"
"		tex.xy/= tex.z;\n"
"		tex.y = 1 - tex.y;\n"
"		vOut = tex2DBC( texContent, ( tex.xy - offsScale.xy ) * offsScale.zw );  \n"
"		vCur = texCur.Sample( samLin, ( tex.xy - offsScaleCur.xy ) * offsScaleCur.zw );  \n"
"		vOut.rgb = vCur.a * vCur.rgb + vOut.rgb * ( 1.0 - vCur.a );\n"
"		if( bBorder.y > 0.5 )                      \n"
"			vOut.rgb*= blend.rgb;			        \n"
"	}\n"
"	vOut.a = 1;                                 \n"
"	return vOut;                                \n"
"}                                               \n"
"                                                \n"
"float4 PSWB3DBC( VS_OUT vIn ) : SV_Target             \n"
"{                                               \n"
"	float4 tex = texWarp.Sample( samWarp, vIn.tex );     \n"
"	float4 blend = texBlend.Sample( samLin, vIn.tex );  \n"
"	blend.a = 1;  \n"
"	float4 vOut = float4( 0,0,0,1);             \n"
"	float4 vCur = 0;\n"
"	if( 0.1 < tex.w && ( 0 < blend.r || 0 < blend.g || 0 < blend.b ) )                            \n"
"	{                                           \n"
"		tex/= tex.w;                            \n"
"		tex = mul( tex, matView );              \n"
"		tex.xy/= tex.w;                         \n"
"		tex.x/=2;                              \n"
"		tex.y/=2;                               \n"
"		tex.xy+= 0.5;                           \n"
"		vOut = tex2DBC( texContent, ( tex.xy - offsScale.xy ) * offsScale.zw ); \n"
"		vCur = texCur.Sample( samLin, (tex.xy - offsScaleCur.xy) * offsScaleCur.zw );  \n"
"		vOut.rgb = vCur.a * vCur.rgb + vOut.rgb * ( 1.0 - vCur.a );\n"
"		if( bBorder.y > 0.5 )                      \n"
"			vOut.rgb*= blend.rgb;			        \n"
"	}                                           \n"
"	vOut.a = 1;                                \n"
"	return vOut;                                \n"
"}                                               \n";
