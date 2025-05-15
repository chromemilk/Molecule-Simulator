#pragma once
#include <string>

unsigned int LoadShader( const std::string &vertexPath, const std::string &fragmentPath );

unsigned int LoadComputeShader(const std::string& computePath);

