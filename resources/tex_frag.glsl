#version 330 core
uniform sampler2D Texture;

in vec2 vTexCoord;
in vec3 vColor;

in vec3 fragNor;
in vec3 worldPos;
in vec3 lPos;
in vec3 lIntensity;

out vec4 Outcolor;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float shine;
uniform vec3 eyePos;
uniform vec3 lightPos;
uniform vec3 lightIntensity;
uniform float hasTexture;

void main() {
    vec3 l = normalize(lightPos - worldPos);
    vec3 v = normalize(eyePos - worldPos);
    vec3 h = normalize(l + v);
    vec3 normal = normalize(fragNor);
    
    vec3 ambient = MatAmb * lIntensity;
    vec3 diffuse = MatDif * max(dot(normal, l), 0) * lIntensity;
    vec3 specular = MatSpec * pow(dot(normal, h), shine) * lIntensity;
    
    if (hasTexture == 1.0) {
        vec4 BaseColor = vec4(ambient + diffuse + specular, 1.0);
        vec4 texColor0 = texture(Texture, vTexCoord);
        Outcolor = vec4(texColor0.r*BaseColor.r, texColor0.g*BaseColor.g, texColor0.b*BaseColor.b, 1.0);
//        Outcolor = vec4(texColor0.r*(ambient.x+diffuse.x+specular.x), texColor0.g*(ambient.y+diffuse.y+specular.y), texColor0.b*(ambient.z+diffuse.z+specular.z), 1.0);
    } else {
        Outcolor = vec4(ambient + diffuse + specular, 1.0);
    }
}

