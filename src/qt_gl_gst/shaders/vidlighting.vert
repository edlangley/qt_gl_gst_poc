uniform vec3 u_lightPosition;

const float mediump SpecularContribution = 0.3;
const float mediump DiffuseContribution  = 1.0 - SpecularContribution;



uniform highp mat4 u_mvp_matrix;
uniform highp mat4 u_mv_matrix;

attribute highp vec4 a_vertex;
attribute highp vec3 a_normal;
attribute highp vec4 a_texCoord;

varying mediump float v_LightIntensity;
varying highp vec4 v_texCoord;

void main(void)
{
    vec3 ecPosition = vec3 (u_mv_matrix * a_vertex);

    vec3 tnorm      = normalize(u_mv_matrix * vec4(a_normal, 0.0));

    vec3 lightVec   = normalize(u_lightPosition - ecPosition);
    vec3 reflectVec = (2.0 * tnorm * dot(lightVec, tnorm)) - lightVec;

    vec3 viewVec    = normalize(-ecPosition);
    float diffuse   = max(dot(lightVec, tnorm), 0.0);
    float spec      = 0.0;

    if (diffuse > 0.0)
    {
        spec = max(dot(reflectVec, viewVec), 0.0);
        //spec = max(dot(halfVec, tnorm), 0.0);

        spec = pow(spec, 6.0);
    }

    v_LightIntensity  = DiffuseContribution * diffuse +
                      SpecularContribution * spec;

    gl_Position     = (u_mvp_matrix * a_vertex);
    v_texCoord = a_texCoord;
}

