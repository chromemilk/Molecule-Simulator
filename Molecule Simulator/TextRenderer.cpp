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
    uniScale = glGetUniformLocation( textProgram, "uScale" );

}


void TextRenderer::DrawText( const std::string &text, const glm::vec3 &worldPos, int windowWidth, int windowHeight, float alpha, float scale) {
    glm::mat4 V = camera.GetViewMatrix();
    glm::mat4 P = glm::perspective( glm::radians( camera.Zoom ),
        float( windowWidth ) / windowHeight,
        0.1f, 100.0f );
    glm::vec4 clip = P * V * glm::vec4( worldPos, 1.0f );
    if (clip.w <= 0) return;
    glm::vec3 ndc = glm::vec3( clip ) / clip.w;

    float sx = (ndc.x * 0.5f + 0.5f) * windowWidth;
    float sy = (1.0f - (ndc.y * 0.5f + 0.5f)) * windowHeight;

    float totalW = float( text.size() ) * 8.0f * scale;
    sx -= totalW * 0.5f;

    float invScale = 1.0f / scale;
    static char buffer[ 9999 ];
    int quads = stb_easy_font_print( int( sx * invScale ),
        int( sy * invScale ),
        const_cast<char *>(text.c_str()),
        nullptr,
        buffer, sizeof( buffer ) );
    if (quads <= 0) return;

    glBindBuffer( GL_ARRAY_BUFFER, textVBO );
    glBufferData( GL_ARRAY_BUFFER, quads * 4 * sizeof( float ) * 4,
        buffer, GL_DYNAMIC_DRAW );

    GLboolean wasDepth = glIsEnabled( GL_DEPTH_TEST );
    glDisable( GL_DEPTH_TEST );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glUseProgram( textProgram );
    // set ortho
    glm::mat4 ortho = glm::ortho( 0.0f, float( windowWidth ),
        float( windowHeight ), 0.0f );
    glUniformMatrix4fv( uniOrtho, 1, GL_FALSE, glm::value_ptr( ortho ) );
    glUniform1f( uniScale, scale );
    glUniform4f( uniColor, 1, 1, 1, alpha );

    glBindVertexArray( textVAO );
    for (int i = 0; i < quads; ++i)
        glDrawArrays( GL_TRIANGLE_FAN, i * 4, 4 );
    glBindVertexArray( 0 );

    glUseProgram( 0 );
    if (wasDepth) glEnable( GL_DEPTH_TEST );
    else        glDisable( GL_DEPTH_TEST );
    glDisable( GL_BLEND );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}



void TextRenderer::DrawScreenText( const std::string &text, float x, float y, int windowWidth, int windowHeight, float alpha, float scale) {
    glm::mat4 ortho = glm::ortho( 0.0f, float( windowWidth ),
        float( windowHeight ), 0.0f );

    float invScale = 1.0f / scale;
    static char buffer[ 9999 ];
    int quads = stb_easy_font_print( int( x * invScale ),
        int( y * invScale ),
        const_cast<char *>(text.c_str()),
        nullptr,
        buffer, sizeof( buffer ) );
    if (quads <= 0) return;

    glBindBuffer( GL_ARRAY_BUFFER, textVBO );
    glBufferData( GL_ARRAY_BUFFER, quads * 4 * sizeof( float ) * 4,
        buffer, GL_DYNAMIC_DRAW );

    GLboolean wasDepth = glIsEnabled( GL_DEPTH_TEST );
    glDisable( GL_DEPTH_TEST );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glUseProgram( textProgram );
    glUniformMatrix4fv( uniOrtho, 1, GL_FALSE, glm::value_ptr( ortho ) );
    glUniform1f( uniScale, scale );
    glUniform4f( uniColor, 1, 1, 1, alpha );

    glBindVertexArray( textVAO );
    for (int i = 0; i < quads; ++i)
        glDrawArrays( GL_TRIANGLE_FAN, i * 4, 4 );
    glBindVertexArray( 0 );

    glUseProgram( 0 );
    if (wasDepth) glEnable( GL_DEPTH_TEST );
    else        glDisable( GL_DEPTH_TEST );
    glDisable( GL_BLEND );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}