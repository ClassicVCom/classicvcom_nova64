#ifndef _SHADERS_HPP_
#define _SHADERS_HPP_

namespace ClassicVCom_Nova64
{
	namespace Shader
	{
		const char *PrimaryVertexShader = R"(#version 300 es

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex;

out vec2 outTex;

void main()
{
	gl_Position = vec4(pos, 1.0f);
	outTex = tex;
})";

		const char *PrimaryFragmentShader = R"(#version 300 es

precision highp float;
precision highp int;
precision highp usampler2D;

struct PaletteTableBufferData
{
	vec4 Palette[256];
};

layout(std140) uniform GraphicsControlBuffer
{
	PaletteTableBufferData palette_table_buffer[8];
};

in vec2 outTex;
out vec4 outColor;

uniform usampler2D MainFramebufferTexture;

void main()
{
	ivec2 texDim = textureSize(MainFramebufferTexture, 0);
	uvec4 colorData = texelFetch(MainFramebufferTexture, ivec2(int(outTex.x * float(texDim.x)), int(outTex.y * float(texDim.y))), 0);
	outColor = palette_table_buffer[colorData.g].Palette[colorData.r];
})";

		const char *ClearFragmentShader = R"(#version 300 es

precision highp int;

layout(std140) uniform FramebufferControlBuffer
{
	uint clear_color_index;
	uint clear_color_palette_table;
};

layout(location = 0) out uvec2 outColorData;

void main()
{
	outColorData = uvec2(clear_color_index, clear_color_palette_table);
})";
	}
}

#endif
