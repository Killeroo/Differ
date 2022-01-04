#pragma warning(disable : 4996)


#include <string>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <map>

#include <openssl/sha.h>

void PrintUsage()
{
    std::cout << "Differ <path1> <path2>" << std::endl;
}

// TODO: implement adler32 instead
////////////////////////////////////////////////////////////////////////////
void sha256_hash_string(unsigned char hash[SHA256_DIGEST_LENGTH], char outputBuffer[65])
{
    int i = 0;

    for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }

    outputBuffer[64] = 0;
}

int GetFileHash(char* path, char outputBuffer[65])
{
    FILE* file = fopen(path, "rb");
    if (!file) return -534;

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    const int bufSize = 32768;
    unsigned char* buffer = (unsigned char*) malloc(bufSize);

    int bytesRead = 0;
    if (!buffer) return ENOMEM;
    while ((bytesRead = fread(buffer, 1, bufSize, file)))
    {
        SHA256_Update(&sha256, buffer, bytesRead);
    }
    SHA256_Final(hash, &sha256);

    sha256_hash_string(hash, outputBuffer);
    fclose(file);
    free(buffer);
    return 0;
}
////////////////////////////////////////////////////////////////////////////

void GetFileAtPath(std::string path, std::vector<std::string>* filePaths)
{
    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            if (entry.is_directory())
                GetFileAtPath(entry.path().string(), filePaths);
            else
                filePaths->push_back(entry.path().string());
        }
    }
    catch (std::filesystem::filesystem_error e)
    {
        std::cout << "Error " << e.code() << ": " << e.what() << std::endl;
    }
}

std::vector<std::string> StripPaths(std::vector<std::string> originalPaths, std::string pathToStrip)
{
    std::size_t len = pathToStrip.length();
    std::vector<std::string> strippedPaths;
    for (auto path : originalPaths)
    {
        strippedPaths.push_back(path.erase(0, len));
    }
    return strippedPaths;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout << "Not enough arguments" << std::endl << std::endl;;
        PrintUsage();
        return 1;
    }

    std::string path1 = argv[1];
    std::string path2 = argv[2];

    if (!std::filesystem::exists(path1) || !std::filesystem::exists(path2))
    {
        std::cout << "One or more of the paths does not exist" << std::endl;
        return 1;
    }

    std::vector<std::string> path1Contents, path2Contents;
    std::vector<std::string> path1StrippedContents, path2StrippedContents;
    //std::map<std::string, std::string> differences;
    std::vector<std::string> difference;

    GetFileAtPath(path1, &path1Contents);
    GetFileAtPath(path2, &path2Contents);

    path1StrippedContents = StripPaths(path1Contents, path1);
    path2StrippedContents = StripPaths(path2Contents, path2);

    std::sort(path1StrippedContents.begin(), path1StrippedContents.end());
    std::sort(path2StrippedContents.begin(), path2StrippedContents.end());

    std::set_difference(
        path1StrippedContents.begin(),
        path1StrippedContents.end(),
        path2StrippedContents.begin(),
        path2StrippedContents.end(),
        std::inserter(difference, difference.begin()));

    for (auto path : difference)
        std::cout << path1 << "\\" << path << std::endl;

    return 0;
}

