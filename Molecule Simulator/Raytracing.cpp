#include "Raytracing.h"
#include <sstream>
#include <fstream>
#include <iostream>

static GLuint buildProgram(const char* vert, const char* frag)
{
    auto compile = [](const char* src, GLenum type)->GLuint {
        GLuint sh = glCreateShader(type);
        glShaderSource(sh, 1, &src, nullptr);
        glCompileShader(sh);
        GLint ok; glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[512]; glGetShaderInfoLog(sh, 512, nullptr, log);
            std::cerr << (type == GL_VERTEX_SHADER ? "VS" : "FS") << " error:\n" << log;
        }
        return sh;
        };
    GLuint v = compile(vert, GL_VERTEX_SHADER),
        f = compile(frag, GL_FRAGMENT_SHADER);
    GLuint p = glCreateProgram();
    glAttachShader(p, v); glAttachShader(p, f); glLinkProgram(p);
    glDeleteShader(v);  glDeleteShader(f);
    return p;
}

void Raytracer::init()
{
    glGenVertexArrays(1, &vao);

    const char* vs =
        "#version 330 core\n"
        "const vec2 v[3]=vec2[3](vec2(-1,-1),vec2(3,-1),vec2(-1,3));\n"
        "void main(){gl_Position=vec4(v[gl_VertexID],0,1);}";

    const char* fs =
        "#version 330 core\n"
        "out vec4 Frag;\n"
        "uniform int  uCount;\n"
        "uniform vec4 uSpheres[128];              // vec4(c.xyz,r)\n"
        "uniform vec3 uCamPos;\n"
        "uniform mat4 uInvPV;\n"
        "uniform vec2 uRes;                       // ? NEW\n"
        "\n"
        "vec3 primaryDir(vec2 frag){\n"
        "  vec2 uv = (frag / uRes) * 2.0 - 1.0;\n"
        "  vec4 clip  = vec4(uv, -1.0, 1.0);\n"
        "  vec4 world = uInvPV * clip;\n"
        "  return normalize(world.xyz / world.w - uCamPos);\n"
        "}\n"
        "\n"
        "bool hitSphere(vec3 O, vec3 D, vec4 s, out float t){\n"
        "  vec3 oc = O - s.xyz;\n"
        "  float b = dot(oc,D), c = dot(oc,oc) - s.w*s.w, h = b*b - c;\n"
        "  if(h < 0.0) return false; h = sqrt(h);\n"
        "  t = -b - h; if(t < 0.0) t = -b + h; return t > 0.0;\n"
        "}\n"
        "\n"
        "void main(){\n"
        "  vec3  O    = uCamPos;\n"
        "  vec3  D    = primaryDir(gl_FragCoord.xy);\n"
        "  vec3  col  = vec3(0.6,0.8,1.0);\n"
        "  float tMin = 1e30;\n"
        "  for(int i=0;i<uCount;++i){\n"
        "    float t; if(hitSphere(O,D,uSpheres[i],t) && t<tMin){\n"
        "      tMin=t; vec3 hit=O+D*t; vec3 N=normalize(hit-uSpheres[i].xyz);\n"
        "      col = 0.5 + 0.5*N; }\n"
        "  }\n"
        "  Frag = vec4(col,1);\n"
        "}\n";


    prog = buildProgram(vs, fs);
}

void Raytracer::setScene(const std::vector<GPUSphere>& s)
{
    sphereCount = (int)std::min<size_t>(s.size(), 128);   // hard limit for GL 3.3
    sphereFlat.resize(sphereCount * 4);
    for (int i = 0;i < sphereCount;++i) {
        sphereFlat[i * 4 + 0] = s[i].c.x;
        sphereFlat[i * 4 + 1] = s[i].c.y;
        sphereFlat[i * 4 + 2] = s[i].c.z;
        sphereFlat[i * 4 + 3] = s[i].r;
    }
}

void Raytracer::render(const Camera& cam, int fbW, int fbH)
{
    glDisable(GL_DEPTH_TEST);                 // full-screen pass
    glUseProgram(prog);
    glBindVertexArray(vao);

    glm::mat4 proj = glm::perspective(glm::radians(cam.Zoom),
        (float)fbW / fbH, 0.1f, 100.0f);
    glm::mat4 invPV = glm::inverse(proj * cam.GetViewMatrix());

    glUseProgram(prog);
    glUniform3fv(glGetUniformLocation(prog, "uCamPos"), 1, &cam.Position[0]);
    glUniformMatrix4fv(glGetUniformLocation(prog, "uInvPV"), 1, GL_FALSE, &invPV[0][0]);
    glUniform2f(glGetUniformLocation(prog, "uRes"), (float)fbW, (float)fbH);   
    glUniform1i(glGetUniformLocation(prog, "uCount"), sphereCount);
    if (sphereCount)
        glUniform4fv(glGetUniformLocation(prog, "uSpheres"),
            sphereCount, sphereFlat.data());


    glDrawArrays(GL_TRIANGLES, 0, 3);
    glEnable(GL_DEPTH_TEST);
}
