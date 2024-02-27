#version 330 core

in vec2 texCoord;

uniform sampler2D uTexture1;
uniform sampler2D uTexture2;

out vec4 fragColor;

void main()
{
	fragColor = texture(uTexture1, texCoord);
	//fragColor = mix(texture(uTexture1, texCoord), texture(uTexture2, texCoord), 1.0);
}