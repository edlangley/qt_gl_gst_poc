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

const mediump float SpecularContribution = 0.3;
const mediump float DiffuseContribution  = 1.0 - SpecularContribution;

uniform highp mat4 u_mvp_matrix;
uniform highp mat4 u_mv_matrix;

attribute highp vec4 a_vertex;
attribute highp vec3 a_normal;
attribute highp vec4 a_texCoord; // unused right now

varying mediump float v_LightIntensity;
varying mediump vec3  v_MCposition;

void main(void)
{
//    vec3 ecPosition = vec3 (gl_ModelViewMatrix * vertex);
    highp vec3 ecPosition = vec3 (u_mv_matrix * a_vertex);

//    vec3 tnorm      = normalize(gl_NormalMatrix * normal);
    highp vec3 tnorm      = normalize(u_mv_matrix * vec4(a_normal, 0.0)).xyz;

    highp vec3 lightVec   = normalize(LightPosition - ecPosition);
    // Using the reflection vector:
//    vec3 reflectVec = reflect(-lightVec, tnorm);
    highp vec3 reflectVec = (2.0 * tnorm * dot(lightVec, tnorm)) - lightVec;

    // Using the half vector:
    //vec3 halfVec    = normalize(-ecPosition - LightPosition);

    highp vec3 viewVec    = normalize(-ecPosition);
    highp float diffuse   = max(dot(lightVec, tnorm), 0.0);
    highp float spec      = 0.0;

    if (diffuse > 0.0)
    {
        spec = max(dot(reflectVec, viewVec), 0.0);
        //spec = max(dot(halfVec, tnorm), 0.0);

        spec = pow(spec, 46.0);
    }

    v_LightIntensity  = DiffuseContribution * diffuse +
                      SpecularContribution * spec;

    v_MCposition      = a_vertex.xyz;
//    gl_Position     = ftransform();
//    gl_Position     = (gl_ModelViewProjectionMatrix * vertex);
    gl_Position     = (u_mvp_matrix * a_vertex);
}
