#ifndef BENGOS_H
#define BENGOS_H
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <stack>

using namespace std;
class Directory;
class SoftLinkedFile;
class RegularFile;

class File {
public:
    File(string name, time_t creationTime = time(nullptr)) : name(name), creationTime(creationTime) {}
    File(string name) : name(name) {}
    virtual void ls() const = 0;
    virtual void lsrec(int depth=0) const = 0;
    virtual void mkdir(Directory* currentDir, const string& directoryName) = 0;
    virtual void rm(Directory* currentDir, const string& fileName) = 0;
    virtual void rmdir(Directory* currentDir, const string& fileName) = 0;
    virtual void cp(const string& sourceFile, const string& destinationFile) = 0;
    virtual void link(const string& sourceFile, const string& destinationFile, Directory* currentDir) = 0;
    virtual void cd(Directory *&currentDir, const string &path) = 0;
    virtual void cat(const string& fileName) const = 0;
    virtual void iterator() const = 0;
    virtual string getName() const {return name;}
    void setName(const string& newName){name = newName;} // Virtual function to check if it's a directory
    virtual void setContent(const string& content) = 0;  // Virtual function to set content
    virtual string getContent() const = 0;  // Virtual function to get content
    virtual vector<File*> getEntries() const {return {};}
    void setParent(Directory* newParent) { parent = newParent; }
    virtual Directory* getParent() const {return parent;}
    virtual bool isDirectory() const = 0;
    virtual bool isRegularFile() const = 0;
    virtual bool isSoftLinkedFile() const = 0;
    virtual void copyFile(const string& sourcePath, const string& destinationPath) = 0;
    virtual time_t getCreationTime() const {
        return creationTime;
    }
protected:
    string name;
    time_t creationTime;
    Directory* parent;
};
#endif