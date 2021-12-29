/* date = December 21st 2021 4:53 am */

#ifndef FILEIO_H
#define FILEIO_H

typedef struct binary_file
{
    b32 exists;
    size_t byteSize;
    char *data;
} binary_file;

internal u32
get_file_size(FILE *file)
{
    fseek(file, 0, SEEK_END);
    
    u32 fileSize = ftell(file);
    
    fseek(file, 0, SEEK_SET);
    
    return fileSize;
}

internal binary_file
read_entire_file(char *fileName)
{
    binary_file result = {0};
    
    FILE *file = 0;
    fopen_s(&file, fileName, "rb");
    
    if (file)
    {
        u32 fileSize = get_file_size(file);
        result.byteSize = fileSize + 1;
        
        result.data = (char *)malloc(result.byteSize * sizeof(char));
        size_t bytesRead = fread(result.data, 1, result.byteSize, file);
        
        if (bytesRead > 0 && bytesRead == result.byteSize)
        {
            result.exists = 1;
            
            result.data[fileSize] = 0;
        }
        
        fclose(file);
    }
    else
    {
        // TODO: Log
#if 0
        char msg[512];
        sprintf_s(msg, sizeof(msg), "Failed to open file: %s\n", fileName); 
        log_message(msg);
#endif
    }
    
    return result;
}

#endif //FILEIO_H
