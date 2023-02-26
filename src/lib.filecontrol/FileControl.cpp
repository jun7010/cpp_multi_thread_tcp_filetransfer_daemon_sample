#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstring>
#include "FileControl.h"

using namespace std;

bool FileController::fileExist()
{
    if (fsize == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

FileController::FileController(char *cFilePath, char *cFileName)
{
    file_exist = true;
    transfer_append_mode = false;
    fsize = 0;
    FILE *file;

    try
    {

        sprintf(full_path, cFilePath);
        strcat(full_path, "/");
        strcat(full_path, cFileName);

        file = fopen64(full_path, "rb");

        cout << "FileController Path : " << full_path << endl;

        if (file != NULL)
        {
            cout << "FileController File Exist" << endl;
            fseek(file, 0, SEEK_END);
            fsize = ftello64(file);
            transfer_append_mode = true;
            fseek(file, 0, SEEK_SET);
            cout << "FileController File Size : " << fsize << endl;
            fclose(file);
        }
        else
        {
            cout << "FileController File Not Exist" << endl;
            fsize = 0;
            file_exist = false;
        }
    }
    catch (const exception &e)
    {
        cout << "FileController Exception" << endl;
        fsize = 0;
        file_exist = false;

        if (file != NULL)
        {
            fclose(file);
        }
    }

    cout << "FileController Constructor End" << endl;
}

FileController::~FileController()
{
}

bool FileController::getFileTransferMode()
{
    return transfer_append_mode;
    // true == 이미 파일 존재
    // false == 파일 없음 / 새로 받아야할 파일
}

size_t FileController::getFileSizeExist()
{
    return fsize;
}

char *FileController::getFilePath()
{
    return full_path;
}

void FileController::appendFileSizeSet(double new_file_size)
{
    cout << "FileController new file size : " << fsize << " -> " << new_file_size << endl;
    fsize = new_file_size;
}
