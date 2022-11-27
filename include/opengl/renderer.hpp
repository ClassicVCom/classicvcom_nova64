#ifndef _RENDERER_HPP_
#define _RENDERER_HPP_

#include <GL/glew.h>
#include <SDL.h>
#include <array>
#include <vector>
#include "renderer_type.hpp"

namespace ClassicVCom_Nova64
{
	struct VertexData
	{
		std::array<float, 3> pos;
		std::array<float, 2> tex;
	};

	struct PaletteData
	{
		float red;
		float green;
		float blue;
		float alpha;
	};

	using PaletteTableBufferData = std::array<PaletteData, 256>;

	enum class ScalingType
	{
		Integer, FloatingPoint
	};

	struct GraphicsControlBufferData
	{
		std::array<PaletteTableBufferData, 8> palette_table_buffer;
	};

	struct FramebufferControlData
	{
		uint32_t clear_color_index;
		uint32_t clear_color_palette_table;
	};

	const RendererType renderer_type = RendererType::OpenGL;

	class Renderer
	{
		public:
			Renderer();
			~Renderer();
			bool Setup(SDL_Window &Window);
			void SetVSync(bool toggle);
			void SetResolution(uint16_t width, uint16_t height);
			void UpdatePaletteTableBuffer(uint8_t index, const PaletteTableBufferData &data);
			void UpdateFramebufferControl(uint8_t clear_color_index, uint8_t clear_color_palette_table);
			void ClearMainFramebuffer();
			void Render(SDL_Window &Window);
			const char *GetRendererError() const;
		private:
			const char *renderer_error_str;
			ScalingType CurrentScalingType;
			SDL_GLContext GLContext;
			GLuint PrimaryVertexShaderId, PrimaryFragmentShaderId, ClearFragmentShaderId;
			GLuint MainProgramId, ClearProgramId;
			GLuint CurrentProgramId;
			GLuint VAOId;
			GLuint VBOId;
			GLuint IBOId;
			GLuint GraphicsControlBufferUBOId, FramebufferControlUBOId;
			GLuint CurrentUBOId;
			GLuint MainFramebufferTextureId;
			GLuint MainFramebufferFBOId;
			GLuint CurrentFBOId;
			std::array<GLuint, 2> TextureUnitIds;
			std::array<VertexData, 4> vertices;
			std::array<uint8_t, 6> indices;
			GraphicsControlBufferData graphics_control_buffer;
			FramebufferControlData framebuffer_control;

			bool CompileShader(GLuint &shader, GLuint shader_type, const char *shader_code);
			bool LinkProgram(GLuint &program, std::vector<GLuint> shader_list);
	};
}

#endif
