#version 330 core

in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D uTexture1;
uniform sampler2D uTexture2;

uniform vec4 uColor = vec4(1.0, 1.0, 1.0, 1.0);

uniform float uMix;

void main()
{
	fragColor = mix(texture(uTexture1, texCoord), texture(uTexture1, texCoord), uMix) * uColor;
}