#version 330 core

// Shader applies effects: pixelate, grayscale, blur

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform bool greyscale = false;
uniform bool pixelate  = false;
uniform bool blur      = false;

out vec4 finalColor;

vec4 applyBlur(vec2 uv, vec2 texelSize)
{
    vec4 colorSum = vec4(0.0);
    const int radius = 4;
    const int kernelSize = (radius * 2 + 1) * (radius * 2 + 1);

    for (int x = -radius; x <= radius; x++)
    {
        for (int y = -radius; y <= radius; y++)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            colorSum += texture(texture0, uv + offset);
        }
    }
    return colorSum / float(kernelSize);
}

void main()
{
    ivec2 texSize = textureSize(texture0, 0);
    vec2 texelSize = 1.0 / vec2(texSize);

    vec2 uv = fragTexCoord;

    if (pixelate)
    {
        vec2 pixelSize = 8.0 * texelSize;
        uv = floor(uv / pixelSize) * pixelSize;
    }

    vec4 texColor = blur ? applyBlur(uv, texelSize) : texture(texture0, uv);

    texColor *= fragColor;

    if (greyscale)
    {
        float gray = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));
        texColor = vec4(vec3(gray), texColor.a);
    }

    finalColor = texColor;
}