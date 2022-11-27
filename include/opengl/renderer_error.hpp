#ifndef _RENDERER_ERROR_HPP_
#define _RENDERER_ERROR_HPP_

namespace ClassicVCom_Nova64
{
	namespace RendererError
	{
		const char *NoError = "No error.";
		const char *GLContextCreationFailed = "Unable to create GL Context.";
		const char *GLEWInitializationFailed = "GLEW Initialization failed.";
		const char *PrimaryVertexShaderCompilationFailed = "Unable to compile Primary Vertex Shader.";
		const char *PrimaryFragmentShaderCompilationFailed = "Unable to compile Primary Fragment Shader.";
		const char *ClearFragmentShaderCompilationFailed = "Unable to compile Clear Fragment Shader.";
		const char *MainProgramLinkageFailed = "Unable to link Main Program.";
		const char *ClearProgramLinkageFailed = "Unable to link Clear Program.";
	}
}

#endif
