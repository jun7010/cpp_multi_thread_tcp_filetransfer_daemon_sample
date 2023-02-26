#ifndef __FILE_CONTROL_h__
#define __FILE_CONTROL_h__

class FileController
{
    char full_path[768];
    size_t fsize;
    bool file_exist;
    bool transfer_append_mode;

public:
    FileController(char *cFilePath, char *cFileName);
    ~FileController();
    bool fileExist();
    bool getFileTransferMode();
    char *getFilePath();
    size_t getFileSizeExist();
    void appendFileSizeSet(double new_file_size);
};

#endif