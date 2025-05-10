#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform sampler2D diffuse;
uniform int transparency;
uniform float alpha;

void main()
{
    vec4 texColor = texture(diffuse, TexCoords);
    
    if(transparency == 1) {
        // Solo para la bola (usa transparencia)
        if(color.a < 0.1)
            discard;
        color = vec4(texColor.rgb, alpha);
    } else {
        // Para todos los demás modelos (opacos)
        color = vec4(color.rgb, 1.0);
    }
}




