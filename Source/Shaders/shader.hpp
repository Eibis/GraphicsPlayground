#ifndef SHADER_HPP
#define SHADER_HPP

GLuint LoadShaders(const std::string& vertex_file_path, const std::string& fragment_file_path);

GLuint LoadBMP_custom(const char * imagepath);
GLuint LoadDDS(const char * imagepath);

#endif
