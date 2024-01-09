#version 330

uniform sampler2D screen_texture;
uniform vec2 player_pos;
uniform float time;
uniform int is_phase_anim;
uniform int is_fail_anim;
uniform float spot_max;

in vec2 texcoord;

layout(location = 0) out vec4 color;

//
// Description : Array and textureless GLSL 2D simplex noise function.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
//

vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
    return mod289(((x*34.0)+1.0)*x);
}

float snoise(vec2 v)
{
    const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
    0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
    -0.577350269189626,  // -1.0 + 2.0 * C.x
    0.024390243902439); // 1.0 / 41.0
    // First corner
    vec2 i  = floor(v + dot(v, C.yy) );
    vec2 x0 = v -   i + dot(i, C.xx);

    // Other corners
    vec2 i1;
    //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
    //i1.y = 1.0 - i1.x;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    // x0 = x0 - 0.0 + 0.0 * C.xx ;
    // x1 = x0 - i1 + 1.0 * C.xx ;
    // x2 = x0 - 1.0 + 2.0 * C.xx ;
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;

    // Permutations
    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
    + i.x + vec3(0.0, i1.x, 1.0 ));

    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;

    // Gradients: 41 points uniformly over a line, mapped onto a diamond.
    // The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt( a0*a0 + h*h );
    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

    // Compute final noise value at P
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

float rand(vec2 co)
{
    return fract(sin(dot(co.xy,vec2(12.9898,78.233))) * 43758.5453);
}

vec4 spotlight(vec4 in_color, vec2 texcoord) {

    vec3 color = in_color.rgb;
    float ambient_c = 0.2;
    // Calculate radial distance from the center
    float distance = length(texcoord - player_pos);
    float fade_factor = smoothstep(spot_max, 0.1, distance);
    fade_factor += ambient_c;
    return vec4(color * fade_factor, in_color.a);      
}

float staticV(vec2 uv) {
    float staticHeight = snoise(vec2(9.0,time*1.2+3.0))*0.3+5.0;
    float staticAmount = snoise(vec2(1.0,time*1.2-6.0))*0.1+0.3;
    float staticStrength = snoise(vec2(-9.75,time*0.6-3.0))*2.0+2.0;
    return (1.0-step(snoise(vec2(5.0*pow(time,2.0)+pow(uv.x*7.0,1.2),pow((mod(time,100.0)+100.0)*uv.y*0.3+3.0,staticHeight))),staticAmount))*staticStrength;
}

void main()
{
    if (is_phase_anim == 1) {
        // Adapted from dyvoid's shader code at https://www.shadertoy.com/view/XtK3W3
        // Create large, incidental noise waves
        float noise = max(0.0, snoise(vec2(time, texcoord.y * 0.3)) - 0.3) * (5.0 / 3.0);

        // Offset by smaller, constant noise waves
        noise = noise + (snoise(vec2(time*10.0, texcoord.y * 2.4)) - 0.5) * 0.15;

        // Apply the noise as x displacement for every line
        float xpos = texcoord.x - noise * noise * 0.10;
        color = texture(screen_texture, vec2(xpos, texcoord.y));

        // Mix in some random interference for lines
        color.rgb = mix(color.rgb, vec3(rand(vec2(texcoord.y * time))), noise * 0.3).rgb;

        // Apply a line pattern every 4 pixels
        if (floor(mod(texcoord.y * 0.25, 2.0)) == 0.0)
        {
            color.rgb *= 1.0 - (0.15 * noise);
        }

        // Shift green/blue channels (using the red channel)
//        color.g = mix(color.r, texture(screen_texture, vec2(xpos + noise * 0.05, texcoord.y)).g, 0.25);
//        color.b = mix(color.r, texture(screen_texture, vec2(xpos - noise * 0.05, texcoord.y)).b, 0.25);
    } else if (is_fail_anim == 1) {
        // Adapted from ehj1's shader code at https://www.shadertoy.com/view/ldXGW4
        float fuzzOffset = snoise(vec2(time*15.0,texcoord.y*80.0))*0.003;
        float largeFuzzOffset = snoise(vec2(time*1.0,texcoord.y*25.0))*0.004;
        float y = mod(texcoord.y,1.0);

        float xOffset = (fuzzOffset + largeFuzzOffset);

        float staticVal = 0.0;

        for (float y = -1.0; y <= 1.0; y += 1.0) {
            float maxDist = 5.0/200.0;
            float dist = y/200.0;
            staticVal += staticV(vec2(texcoord.x,texcoord.y+dist))*(maxDist-abs(dist))*1.5;
        }


        float red 	=   texture(	screen_texture, 	vec2(texcoord.x + xOffset -0.01,y)).r+staticVal;
        float green = 	texture(	screen_texture, 	vec2(texcoord.x + xOffset,	  y)).g+staticVal;
        float blue 	=	texture(	screen_texture, 	vec2(texcoord.x + xOffset +0.01,y)).b+staticVal;

        color.rgb = vec3(red,green,blue);
        float scanline = sin(texcoord.y*800.0)*0.04;
        color -= scanline;
    } else {
        color = texture(screen_texture, texcoord);
    }
    color = spotlight(color, texcoord);
    // color = color;

}
//void main()
//{
//	// vec2 coord = distort(texcoord);
//    vec4 in_color = texture(screen_texture, texcoord);
////    color = color_shift(in_color, texcoord);
//    // color = in_color;
//}

