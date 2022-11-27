#include "renderer.hpp"
#include "renderer_error.hpp"
#include "shaders.hpp"
#include <memory>
#include <fmt/core.h>
#include <msbtfont/msbtfont.h>

ClassicVCom_Nova64::Renderer::Renderer() : renderer_error_str(ClassicVCom_Nova64::RendererError::NoError), CurrentScalingType(ClassicVCom_Nova64::ScalingType::Integer), GLContext(nullptr), PrimaryVertexShaderId(0), PrimaryFragmentShaderId(0), ClearFragmentShaderId(0), MainProgramId(0), ClearProgramId(0), CurrentProgramId(0), VAOId(0), VBOId(0), IBOId(0), GraphicsControlBufferUBOId(0), FramebufferControlUBOId(0), CurrentUBOId(0), MainFramebufferTextureId(0), MainFramebufferFBOId(0), CurrentFBOId(0), TextureUnitIds{ 0, 0 }, framebuffer_control{ 0, 0 }
{
	vertices = {
		VertexData { { -1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
		VertexData { { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
		VertexData { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
		VertexData { { 1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } }
	};
	indices = { 0, 2, 1, 2, 3, 1 };
}

ClassicVCom_Nova64::Renderer::~Renderer()
{
	if (GLContext != nullptr)
	{
		glUseProgram(0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		for (size_t i = 0; i < TextureUnitIds.size(); ++i)
		{
			if (TextureUnitIds[i] != 0)
			{
				glDeleteTextures(1, &TextureUnitIds[i]);
			}
		}
		if (MainFramebufferFBOId != 0)
		{
			glDeleteFramebuffers(1, &MainFramebufferFBOId);
		}
		if (MainFramebufferTextureId != 0)
		{
			glDeleteTextures(1, &MainFramebufferTextureId);
		}
		if (FramebufferControlUBOId != 0)
		{
			glBindBufferRange(GL_UNIFORM_BUFFER, 1, 0, 0, 0);
			glDeleteBuffers(1, &FramebufferControlUBOId);
		}
		if (GraphicsControlBufferUBOId != 0)
		{
			glBindBufferRange(GL_UNIFORM_BUFFER, 0, 0, 0, 0);
			glDeleteBuffers(1, &GraphicsControlBufferUBOId);
		}
		if (IBOId != 0)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glDeleteBuffers(1, &IBOId);
		}
		if (VBOId != 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDeleteBuffers(1, &VBOId);
		}
		if (VAOId != 0)
		{
			glBindVertexArray(0);
			glDeleteVertexArrays(1, &VAOId);
		}
		if (ClearProgramId != 0)
		{
			glDetachShader(ClearProgramId, PrimaryVertexShaderId);
			glDetachShader(ClearProgramId, ClearFragmentShaderId);
			glDeleteProgram(ClearProgramId);
		}
		if (MainProgramId != 0)
		{
			glDetachShader(MainProgramId, PrimaryVertexShaderId);
			glDetachShader(MainProgramId, PrimaryFragmentShaderId);
			glDeleteProgram(MainProgramId);
		}
		if (PrimaryVertexShaderId != 0)
		{
			glDeleteShader(PrimaryVertexShaderId);
		}
		if (ClearFragmentShaderId != 0)
		{
			glDeleteShader(ClearFragmentShaderId);
		}
		if (PrimaryFragmentShaderId != 0)
		{
			glDeleteShader(PrimaryFragmentShaderId);
		}
		SDL_GL_DeleteContext(GLContext);
	}
}

bool ClassicVCom_Nova64::Renderer::Setup(SDL_Window &Window)
{
	GLContext = SDL_GL_CreateContext(&Window);
	if (GLContext == nullptr)
	{
		renderer_error_str = RendererError::GLContextCreationFailed;
		return false;
	}
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		renderer_error_str = RendererError::GLEWInitializationFailed;
		return false;
	}
	if (!CompileShader(PrimaryVertexShaderId, GL_VERTEX_SHADER, Shader::PrimaryVertexShader))
	{
		renderer_error_str = RendererError::PrimaryVertexShaderCompilationFailed;
		return false;
	}
	if (!CompileShader(PrimaryFragmentShaderId, GL_FRAGMENT_SHADER, Shader::PrimaryFragmentShader))
	{
		renderer_error_str = RendererError::PrimaryFragmentShaderCompilationFailed;
		return false;
	}
	if (!CompileShader(ClearFragmentShaderId, GL_FRAGMENT_SHADER, Shader::ClearFragmentShader))
	{
		renderer_error_str = RendererError::ClearFragmentShaderCompilationFailed;
		return false;
	}
	std::vector<GLuint> shader_list;
	shader_list.push_back(PrimaryVertexShaderId);
	shader_list.push_back(PrimaryFragmentShaderId);
	if (!LinkProgram(MainProgramId, std::move(shader_list)))
	{
		renderer_error_str = RendererError::MainProgramLinkageFailed;
		return false;
	}
	shader_list.push_back(PrimaryVertexShaderId);
	shader_list.push_back(ClearFragmentShaderId);
	if (!LinkProgram(ClearProgramId, std::move(shader_list)))
	{
		renderer_error_str = RendererError::ClearProgramLinkageFailed;
		return false;
	}
	glGenVertexArrays(1, &VAOId);
	glGenBuffers(1, &VBOId);
	glGenBuffers(1, &IBOId);
	glGenBuffers(1, &GraphicsControlBufferUBOId);
	glGenBuffers(1, &FramebufferControlUBOId);
	glBindVertexArray(VAOId);
	glBindBuffer(GL_ARRAY_BUFFER, VBOId);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexData), vertices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint8_t), indices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), reinterpret_cast<void *>(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), reinterpret_cast<void *>(sizeof(float) * 3));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, GraphicsControlBufferUBOId, 0, sizeof(graphics_control_buffer));
	glBufferData(GL_UNIFORM_BUFFER, sizeof(graphics_control_buffer), &graphics_control_buffer, GL_STATIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, 1, FramebufferControlUBOId, 0, sizeof(framebuffer_control));
	glBufferData(GL_UNIFORM_BUFFER, sizeof(framebuffer_control), &framebuffer_control, GL_STATIC_DRAW);
	glUniformBlockBinding(ClearProgramId, 0, 1);
	glGenTextures(1, &MainFramebufferTextureId);
	glGenTextures(1, &TextureUnitIds[0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, MainFramebufferTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8UI, 1280, 720, 0, GL_RG_INTEGER, GL_UNSIGNED_BYTE, nullptr);
	glGenFramebuffers(1, &MainFramebufferFBOId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, MainFramebufferFBOId);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, MainFramebufferTextureId, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, TextureUnitIds[0]);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (GLEW_ARB_texture_storage)
	{
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8UI, 2048, 2048, 4);
	}
	else
	{
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8UI, 2048, 2048, 4, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
	}
	glBindTexture(GL_TEXTURE_2D_ARRAY, TextureUnitIds[1]);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (GLEW_ARB_texture_storage)
	{
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8UI, 4096, 4096, 2);
	}
	else
	{
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8UI, 4096, 4096, 2, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
	}
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, 1280, 720);
	glUseProgram(MainProgramId);
	CurrentProgramId = MainProgramId;
	return true;
}

void ClassicVCom_Nova64::Renderer::SetVSync(bool toggle)
{
	SDL_GL_SetSwapInterval(toggle ? 1 : 0);
}

void ClassicVCom_Nova64::Renderer::SetResolution(uint16_t width, uint16_t height)
{
	if (width < 32)
	{
		width = 32;
	}
	else if (width > 1920)
	{
		width = 1920;
	}
	if (height < 32)
	{
		height = 32;
	}
	else if (height > 1080)
	{
		height = 1080;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8UI, width, height, 0, GL_RG_INTEGER, GL_UNSIGNED_BYTE, nullptr);
}

void ClassicVCom_Nova64::Renderer::UpdatePaletteTableBuffer(uint8_t index, const PaletteTableBufferData &data)
{
	if (index < 8)
	{
		graphics_control_buffer.palette_table_buffer[index] = data;
		if (CurrentUBOId != GraphicsControlBufferUBOId)
		{
			CurrentUBOId = GraphicsControlBufferUBOId;
			glBindBuffer(GL_UNIFORM_BUFFER, GraphicsControlBufferUBOId);
		}
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(graphics_control_buffer), &graphics_control_buffer);
	}
}

void ClassicVCom_Nova64::Renderer::UpdateFramebufferControl(uint8_t clear_color_index, uint8_t clear_color_palette_table)
{
	framebuffer_control.clear_color_index = clear_color_index;
	framebuffer_control.clear_color_palette_table = clear_color_palette_table;
	if (CurrentUBOId != FramebufferControlUBOId)
	{
		CurrentUBOId = FramebufferControlUBOId;
		glBindBuffer(GL_UNIFORM_BUFFER, FramebufferControlUBOId);
	}
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(framebuffer_control), &framebuffer_control);
}

void ClassicVCom_Nova64::Renderer::ClearMainFramebuffer()
{
	if (CurrentProgramId != ClearProgramId)
	{
		CurrentProgramId = ClearProgramId;
		glUseProgram(ClearProgramId);
	}
	if (CurrentFBOId != MainFramebufferFBOId)
	{
		CurrentFBOId = MainFramebufferFBOId;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, MainFramebufferFBOId);
	}
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_BYTE, reinterpret_cast<void *>(0));
}

void ClassicVCom_Nova64::Renderer::Render(SDL_Window &Window)
{
	if (CurrentProgramId != MainProgramId)
	{
		CurrentProgramId = MainProgramId;
		glUseProgram(MainProgramId);
	}
	if (CurrentFBOId != 0)
	{
		CurrentFBOId = 0;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_BYTE, reinterpret_cast<void *>(0));
	SDL_GL_SwapWindow(&Window);
}

const char *ClassicVCom_Nova64::Renderer::GetRendererError() const
{
	return renderer_error_str;
}

bool ClassicVCom_Nova64::Renderer::CompileShader(GLuint &shader, GLuint shader_type, const char *shader_code)
{
	if (shader != 0)
	{
		glDeleteShader(shader);
	}
	shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &shader_code, nullptr);
	glCompileShader(shader);
	int compile_status = 0, info_log_len = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status == GL_FALSE)
	{
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_len);
		std::vector<char> info_log_str(info_log_len);
		glGetShaderInfoLog(shader, info_log_len, nullptr, info_log_str.data());
		fmt::print("{}\n", info_log_str.data());
		return false;
	}
	return true;
}

bool ClassicVCom_Nova64::Renderer::LinkProgram(GLuint &program, std::vector<GLuint> shader_list)
{
	if (shader_list.size() == 0)
	{
		return false;
	}
	if (program != 0)
	{
		glDeleteProgram(program);
	}
	program = glCreateProgram();
	for (auto &i : shader_list)
	{
		glAttachShader(program, i);
	}
	glLinkProgram(program);
	int link_status = 0, info_log_len = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &link_status);
	if (link_status == GL_FALSE)
	{
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_len);
		std::vector<char> info_log_str(info_log_len);
		glGetProgramInfoLog(program, info_log_len, nullptr, info_log_str.data());
		fmt::print("{}\n", info_log_str.data());
		return false;
	}
	return true;
}
