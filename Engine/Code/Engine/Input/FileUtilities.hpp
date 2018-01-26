#pragma once
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
bool LoadBinaryFileToBuffer( const std::string& filePath, std::vector< unsigned char >& out_buffer );
bool SaveBinaryFileFromBuffer( const std::string& filePath, const std::vector< unsigned char >& buffer );
void* FileReadToBuffer(char const *filename, size_t *out_size);