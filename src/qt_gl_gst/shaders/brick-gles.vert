//
// Vertex shader for procedural bricks
//
// Authors: Ed Langley
//          based on a shader by Dave Baldwin, Steve Koren,
//          Randi Rost
//          which was based on a shader by Darwyn Peachey
//
// Copyright (c) 2002-2004 3Dlabs Inc. Ltd. 
//
// See 3Dlabs-License.txt for license information
//

uniform vec3 LightPosition;

const float SpecularContribution = 0.3;
const float DiffuseContribution  = 1.0 - SpecularContribution;

uniform mediump mat4 mvp_matrix;

attribute highp vec4 vertex;
attribute mediump vec3 normal;
attribute highp vec4 texCoord; // unused right now

varying float LightIntensity;
varying vec3  MCposition;

void main(void)
{
    vec3 ecPosition = vec3 (gl_ModelViewMatrix * vertex);
    vec3 tnorm      = normalize(gl_NormalMatrix * normal);
    vec3 lightVec   = normalize(LightPosition - ecPosition);
    vec3 reflectVec = reflect(-lightVec, tnorm);
    vec3 viewVec    = normalize(-ecPosition);
    float diffuse   = max(dot(lightVec, tnorm), 0.0);
    float spec      = 0.0;

    if (diffuse > 0.0)
    {
        spec = max(dot(reflectVec, viewVec), 0.0);
        spec = pow(spec, 16.0);
    }

    LightIntensity  = DiffuseContribution * diffuse +
                      SpecularContribution * spec;

    MCposition      = vertex.xyz;
    gl_Position     = (gl_ModelViewProjectionMatrix * vertex);
//    gl_Position     = ftransform();
}
