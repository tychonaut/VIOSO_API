#if defined(VIOSOWARPBLEND_EXPORTS)
#define VIOSOWARPBLEND_API(ret,name,args) extern "C" __declspec(dllexport) ret name args
#elif defined(VIOSOWARPBLEND_DYNAMIC_DEFINE)
#define VIOSOWARPBLEND_API(ret,name,args) \
	typedef ret (*pfn_##name)args;\
	extern "C" pfn_##name name;
#undef VIOSOWARPBLEND_DYNAMIC_DEFINE
#elif defined(VIOSOWARPBLEND_DYNAMIC_IMPLEMENT)
#define VIOSOWARPBLEND_API(ret,name,args) \
	pfn_##name name = NULL;
#undef VIOSOWARPBLEND_DYNAMIC_IMPLEMENT
#elif defined(VIOSOWARPBLEND_DYNAMIC_DEFINE_IMPLEMENT)
#define VIOSOWARPBLEND_API(ret,name,args) \
	typedef ret (*pfn_##name)args;\
	pfn_##name name = NULL;
#undef VIOSOWARPBLEND_DYNAMIC_DEFINE_IMPLEMENT
#elif defined(VIOSOWARPBLEND_DYNAMIC_INITIALIZE)
#if !defined( VIOSOWARPBLEND_FILE )
#if defined( _M_X64 )
#ifdef UNICODE
#define VIOSOWARPBLEND_FILE L"ViosoWarpBlend64"
#else
#define VIOSOWARPBLEND_FILE "ViosoWarpBlend64"
#endif
#else
#ifdef UNICODE
#define VIOSOWARPBLEND_FILE L"ViosoWarpBlend"
#else
#define VIOSOWARPBLEND_FILE "ViosoWarpBlend"
#endif
#endif //def _M_X64

#endif // !defined( VIOSOWARPBLEND_FILE )
#ifdef WIN32
	HMODULE hMVIOSOWARPBLEND_DYNAMIC = ::LoadLibrary( VIOSOWARPBLEND_FILE );
#define VIOSOWARPBLEND_API(ret,name,args) \
	name = (pfn_##name)::GetProcAddress( hMVIOSOWARPBLEND_DYNAMIC, #name )
#else
	void *hMVIOSOWARPBLEND_DYNAMIC = dlopen(VIOSOWARPBLEND_FILE, RTLD_NOW);
#define VIOSOWARPBLEND_API(ret,name,args) \
	name = (pfn_##name)dlsym( hMVIOSOWARPBLEND_DYNAMIC, #name )
#endif //def WIN32
#undef VIOSOWARPBLEND_DYNAMIC_INITIALIZE
#else
#ifdef _MSC_VER
#define VIOSOWARPBLEND_API(ret,name,arg) extern "C" __declspec(dllimport) ret name arg
#elif defined __APPLE__
#define VIOSOWARPBLEND_API(ret,name,arg) extern "C" ret name arg
#elif defined __GNUC__
#define VIOSOWARPBLEND_API(ret,name,arg) extern "C" __attribute__((visibility("default"))) ret name arg
#else
#error compiler not implemented
#endif //def WIN32
#endif
