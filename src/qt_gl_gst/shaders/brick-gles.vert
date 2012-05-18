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

const float mediump SpecularContribution = 0.3;
const float mediump DiffuseContribution  = 1.0 - SpecularContribution;

uniform mediump mat4 mvp_matrix;
uniform mediump mat4 mv_matrix;

attribute highp vec4 vertex;
attribute mediump vec3 normal;
attribute highp vec4 texCoord; // unused right now

varying float mediump LightIntensity;
varying vec3  mediump MCposition;

void main(void)
{
//    vec3 ecPosition = vec3 (gl_ModelViewMatrix * vertex);
    vec3 ecPosition = vec3 (mv_matrix * vertex);

//    vec3 tnorm      = normalize(gl_NormalMatrix * normal);
    vec3 tnorm      = normalize(mv_matrix * vec4(normal, 0.0));

    vec3 lightVec   = normalize(LightPosition - ecPosition);
    // Using the reflection vector:
//    vec3 reflectVec = reflect(-lightVec, tnorm);
    vec3 reflectVec = (2.0 * tnorm * dot(lightVec, tnorm)) - lightVec;

    // Using the half vector:
    //vec3 halfVec    = normalize(-ecPosition - LightPosition);

    vec3 viewVec    = normalize(-ecPosition);
    float diffuse   = max(dot(lightVec, tnorm), 0.0);
    float spec      = 0.0;

    if (diffuse > 0.0)
    {
        spec = max(dot(reflectVec, viewVec), 0.0);
        //spec = max(dot(halfVec, tnorm), 0.0);

        spec = pow(spec, 46.0);
    }

    LightIntensity  = DiffuseContribution * diffuse +
                      SpecularContribution * spec;

    MCposition      = vertex.xyz;
//    gl_Position     = ftransform();
//    gl_Position     = (gl_ModelViewProjectionMatrix * vertex);
    gl_Position     = (mvp_matrix * vertex);
}
