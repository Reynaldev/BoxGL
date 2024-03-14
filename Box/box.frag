#version 330 core

in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D uTexture1;
uniform sampler2D uTexture2;

uniform vec4 uColor;

void main()
{
	fragColor = texture(uTexture1, texCoord) * uColor;
	//fragColor = mix(texture(uTexture1, texCoord), texture(uTexture2, texCoord), 1.0);
}