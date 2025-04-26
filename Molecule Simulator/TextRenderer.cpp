#include "TextRenderer.h"
#include "../external/stb_easy_font.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ShaderLoader.h"
#include "Camera.h"
#include <GLFW/glfw3.h>

extern Camera camera;
extern GLFWwindow *window;

TextRenderer::TextRenderer()
    : textVAO( 0 ), textVBO( 0 ), textProgram( 0 ), uniOrtho( -1 ), uniColor( -1 ) {
}

TextRenderer::~TextRenderer() {
    glDeleteVertexArrays( 1, &textVAO );
    glDeleteBuffers( 1, &textVBO );
    glDeleteProgram( textProgram );
}

void TextRenderer::Init() {
    // Generate and configure VAO/VBO
    glGenVertexArrays( 1, &textVAO );
    glGenBuffers( 1, &textVBO );
    glBindVertexArray( textVAO );
    glBindBuffer( GL_ARRAY_BUFFER, textVBO );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 16, (void *)0 );
    glBindVertexArray( 0 );

    // Load text shaders
    textProgram = LoadShader( "text.vert", "text.frag" );
    uniOrtho = glGetUniformLocation( textProgram, "uProjection" );
    uniColor = glGetUniformLocation( textProgram, "uColor" );
}

void TextRenderer::DrawText( const std::string &text, const glm::vec3 &worldPos, int windowWidth, int windowHeight ) {
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 proj = glm::perspective( glm::radians( camera.Zoom ), (float)windowWidth / windowHeight, 0.1f, 100.0f );
    glm::mat4 VP = proj * view;

    glm::vec4 clipSpace = VP * glm::vec4( worldPos, 1.0f );

    if (clipSpace.w <= 0.0f) return;  // behind camera

    glm::vec3 ndc = glm::vec3( clipSpace ) / clipSpace.w; // perspective divide

    // Map NDC [-1,1] to window coordinates
    float screenX = (ndc.x * 0.5f + 0.5f) * windowWidth;
    float screenY = (1.0f - (ndc.y * 0.5f + 0.5f)) * windowHeight; // flip Y

    // Center the text horizontally
    float textPixelWidth = (float)text.length() * 8.0f; // stb_easy_font is ~8px per char
    screenX -= textPixelWidth * 0.5f;

    static char buffer[ 9999 ];
    int quads = stb_easy_font_print( (int)screenX, (int)screenY,
        const_cast<char *>(text.c_str()), nullptr, buffer, sizeof( buffer ) );
    if (quads <= 0) return;

    glBindBuffer( GL_ARRAY_BUFFER, textVBO );
    glBufferData( GL_ARRAY_BUFFER, quads * 4 * 16, buffer, GL_DYNAMIC_DRAW );

    GLboolean wasDepth = glIsEnabled( GL_DEPTH_TEST );
    glDisable( GL_DEPTH_TEST );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glUseProgram( textProgram );
    glm::mat4 ortho = glm::ortho( 0.0f, (float)windowWidth, (float)windowHeight, 0.0f ); // top-left origin
    glUniformMatrix4fv( uniOrtho, 1, GL_FALSE, glm::value_ptr( ortho ) );
    glUniform3f( uniColor, 1.0f, 1.0f, 1.0f );

    glBindVertexArray( textVAO );
    for (int i = 0; i < quads; ++i)
        glDrawArrays( GL_TRIANGLE_FAN, i * 4, 4 );
    glBindVertexArray( 0 );

    glUseProgram( 0 );
    if (wasDepth) glEnable( GL_DEPTH_TEST );
    else glDisable( GL_DEPTH_TEST );
    glDisable( GL_BLEND );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}



void TextRenderer::DrawScreenText( const std::string &text, float x, float y, int windowWidth, int windowHeight ) {
    // Same orthographic projection
    glm::mat4 ortho = glm::ortho( 0.0f, (float)windowWidth, (float)windowHeight, 0.0f );

    static char buffer[ 9999 ];
    int quads = stb_easy_font_print( (int)x, (int)y,
        const_cast<char *>(text.c_str()), nullptr, buffer, sizeof( buffer ) );
    if (quads <= 0) return;

    glBindBuffer( GL_ARRAY_BUFFER, textVBO );
    glBufferData( GL_ARRAY_BUFFER, quads * 4 * 16, buffer, GL_DYNAMIC_DRAW );

    GLboolean wasDepth = glIsEnabled( GL_DEPTH_TEST );
    glDisable( GL_DEPTH_TEST );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glUseProgram( textProgram );
    glUniformMatrix4fv( uniOrtho, 1, GL_FALSE, glm::value_ptr( ortho ) );
    glUniform3f( uniColor, 1.0f, 1.0f, 1.0f );

    glBindVertexArray( textVAO );
    for (int i = 0; i < quads; ++i)
        glDrawArrays( GL_TRIANGLE_FAN, i * 4, 4 );
    glBindVertexArray( 0 );

    glUseProgram( 0 );
    if (wasDepth) glEnable( GL_DEPTH_TEST );
    else glDisable( GL_DEPTH_TEST );
    glDisable( GL_BLEND );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

