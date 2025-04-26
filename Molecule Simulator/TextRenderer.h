#pragma once
#include <string>
#include <glm/glm.hpp>

class TextRenderer
{
public:
    TextRenderer();
    ~TextRenderer();

    void Init();
    void DrawText( const std::string &text, const glm::vec3 &worldPos, int windowWidth, int windowHeight );

    void DrawScreenText( const std::string &text, float x, float y, int windowWidth, int windowHeight );

private:
    unsigned int textVAO;
    unsigned int textVBO;
    unsigned int textProgram;
    int uniOrtho;
    int uniColor;
};
