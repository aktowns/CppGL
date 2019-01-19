#version 330 core
/*
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;

    //vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    //color = vec4(textColor, 1.0) * sampled;
    //color = vec4(1,1,1, texture2D(text, TexCoords).r) * vec4(textColor, 1.0);
}  
*/

in vec2 pos;
out vec4 color;

uniform sampler2D msdf;
uniform float pxRange;
uniform vec4 bgColor;
uniform vec4 fgColor;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

/* 
		vec2 msdfUnit = 4.0/vec2(textureSize(texture1, 0));
		vec3 samp = texture(texture1, texcoord.xy).rgb;
		float sigDist = median(samp.r, samp.g, samp.b);
		float w = fwidth(sigDist);
		float opacity = smoothstep(0.5 - w, 0.5 + w, sigDist);
		fragColor = mix(vec4(1.0,1.0,1.0,1.0), vec4(0., 1.0, 0.0, 1.0), opacity);
*/

void main() {
    vec2 pos2 = pos;
    color = texture(msdf, pos2);
    /*
    vec2 msdfUnit = 4.0/vec2(textureSize(msdf, 0));
    vec3 sample = texture(msdf, pos.xy).rgb;
    float sigDist = median(sample.r, sample.g, sample.b);
    //sigDist *= dot(msdfUnit, 0.5/fwidth(pos));
    float w = fwidth(sigDist);
	float opacity = smoothstep(0.5 - w, 0.5 + w, sigDist);
    color = mix(vec4(1.0, 1.0, 1.0, 1.0), vec4(0., 1.0, 0.0, 1.0), opacity);
    */
}