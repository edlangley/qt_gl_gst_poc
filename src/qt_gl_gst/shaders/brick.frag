//
// Fragment shader for procedural bricks
//
// Authors: Dave Baldwin, Steve Koren, Randi Rost
//          based on a shader by Darwyn Peachey
//
// Copyright (c) 2002-2004 3Dlabs Inc. Ltd.
//
// See 3Dlabs-License.txt for license information
//  

uniform highp vec3 BrickColor, MortarColor;
uniform highp vec3 BrickSize;
uniform highp vec3 BrickPct;

varying mediump vec3  v_MCposition;
varying highp float v_LightIntensity;

void main(void)
{
    highp vec3 color;
    mediump vec3 position, useBrick;
    
    position = v_MCposition / BrickSize;

    if (fract(position.y * 0.5) > 0.5)
        position.x += 0.5;

	if (fract(position.y * 0.5) > 0.5)
        position.z += 0.5;

    position = fract(position);

    useBrick = step(position, BrickPct);

    color  = mix(MortarColor, BrickColor, useBrick.x * useBrick.y * useBrick.z);
    color *= v_LightIntensity;
    gl_FragColor = vec4 (color, 1.0);
//    gl_FragColor = vec4 (1.0, 1.0, 0.0, 1.0);
}
