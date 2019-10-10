Preamble:
	This "guide" file is written by an external user, not affilated with VIOSO, 
	who just tried to compile the VIOSO API from a Github repository without any further resources or support from VIOSO;
	I had quite a hassle with it, fixed/updated some aspects of the build environment and a little bit of source code, forked the repo and commited the results.

Current status: 
	I suceeded in Compiling the library, though had no time to aktuelly link and use it. So no guarantee that anything is correctly configured yet.


Warning regarding cmake:
	The CMakeLists.txt file still didn't generate a sucessfully compiling VS solution after trying for quite a while.
	There is too much Microsoft stuff going on in the background  (svn calls, source code generation from SVN output...) 
	I don't know how to setup this with cmake. I gave up and used the VS solution included in the repository instead. 
	This one was quite a hassle to make it compile, too, but at least it DID compile in the end.
	
	
What dou you need to have installed to make the VS solution compile:
	- tortoiseGit (generates revision info source code to be compiled into the library)
	- Microsoft DirectX SDK (June 2010) (though it might be omitted if instead some path settings are changed in the VS project; 
	  there are some directx binaries here that I found only later).
	  (Microsoft DirectX SDK (June 2010) can make some problems; 
	  I had to remove all Visual Studio 2010 redistributabiles first, and even then not all files from the installer 
	  could be written to the install directory (Txview.dll)  (the library still at least compiles,though)).
	
Usage example apps:	
	There a no usage examples apps yet in this repo. I received some older Version of this API per mail once that included example apps.
	I will will try to marry those examples with this source code library and commit the result upon success.
	  