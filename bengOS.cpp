#include "bengOS.h"
class RegularFile : public File {
public:
    RegularFile(string name, time_t creationTime = time(nullptr)) : File(name, creationTime) {}
    void ls() const override {}
    bool isRegularFile()const override{return true;}
    bool isSoftLinkedFile()const override{return false;}
    void copyFile(const string& sourcePath, const string& destinationPath)override{}
    void lsrec(int depth=0) const override {}
    void mkdir(Directory* currentDir, const string& directoryName)override{}
    void cd(Directory *&currentDir, const string &path)override{}
    void rmdir(Directory* currentDir, const string& fileName) override{}
    void rm(Directory* currentDir, const string& fileName) override {}
    void link(const string& sourceFile, const string& destinationFile, Directory* currentDir) override {}
    void cat(const string& fileName) const override{}
    void cp(const string& sourceFile, const string& destinationFile) {}
    void setSize(streamsize a){bytes = a;}
    streamsize getSize() const{return bytes;}    
    void iterator() const override {}
    bool isDirectory() const override {return false;}
    void setContent(const string& content) override {this->content = content;}
    string getContent() const override {return content;}
private:
    vector<File*> entries;
    string content; 
    streamsize bytes=0;
};

class SoftLinkedFile : public File {
public:
    SoftLinkedFile(string name, time_t creationTime = time(nullptr)) : File(name, creationTime) {}
    void ls() const override {}
    bool isRegularFile()const override{return false;}
    bool isSoftLinkedFile()const override{return true;}
    void copyFile(const string& sourcePath, const string& destinationPath)override{}
    void lsrec(int depth=0) const override {}
    void mkdir(Directory* currentDir, const string& directoryName)override{}
    void cd(Directory *&currentDir, const string &path)override{}
    void rmdir(Directory* currentDir, const string& fileName) override{}
    void rm(Directory* currentDir, const string& fileName) override {}
    void link(const string& sourceFile, const string& destinationFile, Directory* currentDir) override {}
    void cat(const string& fileName) const override{}
    void cp(const string& sourceFile, const string& destinationFile) {}
    void iterator() const override {}
    bool isDirectory() const override {return false;}
    void setContent(const string& content) override {}
    const string& getTarget() const {return target;}
    void setTarget(const string& newTarget) {target = newTarget;}
    string getContent() const override { return "";}   
private:
    vector<File*> entries;
    string target;
};

class Directory : public File {
public:
    Directory(string name, time_t creationTime = time(nullptr)) : File(name, creationTime) {}
    Directory(string name, Directory* parent) : File(name, creationTime), parent(parent) {}
    bool isDirectory() const override { return true; }
    void setContent(const string &content) override { cerr << "Error: Directories cannot have content." << endl; }
    string getContent() const override { return ""; }
    vector<File *> getEntries() const override { return entries; }
    string getName() const override{return name;}
    Directory* getParent(Directory* currentDir) const{return parent;}
    void ls() const override {
        for (const auto &entry : entries) {
            printEntryInfo(entry);
        }
    }
    void lsrec(int depth=0) const {       
    ls();  // Print the contents of the current directory
    for (const auto &entry : entries) {
        if (entry->isDirectory()) {
            // If the entry is a directory, recursively call lsrec
            Directory *subDir = dynamic_cast<Directory *>(entry);
            if (!subDir->getEntries().empty()) {
                for (int i = 0; i < depth; ++i) {
                    cout << "  ";
                }
                cout << entry->getName() << ": " << endl;
                    for (int i = 0; i < depth; ++i) {
                        cout << "  ";
                }
                subDir->lsrec(depth+1);
        } else {
        }
    }
}
}
    void printEntryInfo(const File* entry, int depth = 0) const {
    time_t creationTime = entry->getCreationTime();
    struct tm* timeInfo = localtime(&creationTime);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%b %d %H:%M", timeInfo);
    cout << (entry->isDirectory() ? "D" : (entry->isSoftLinkedFile() ? "S" : "F")) << " "
         << entry->getName() << "      "
         << buffer << "      ";
        if (entry->isRegularFile()) {
        const RegularFile* regularFile = dynamic_cast<const RegularFile*>(entry);
        cout << regularFile->getSize() << "Bytes ";
    }
    for (int i = 0; i < depth; ++i) {
        cout << "  ";
    }
    cout << endl;
}
    void addDirectory(File *directoryName) {
        entries.push_back(directoryName);
        if (directoryName->isDirectory()) {
            Directory* dir = dynamic_cast<Directory*>(directoryName);
            dir->parent = this;
        }
    }
    void mkdir(Directory *currentDir, const string &directoryName) override {
    // Check if a directory with the same name already exists
    for (const auto &entry : currentDir->getEntries()) {
        if (entry->isDirectory() && entry->getName() == directoryName) {
            cerr << "mkdir: cannot create directory '" << directoryName << "': File exists" << endl;
            return; // Abort directory creation
        }
    }
    currentDir->addDirectory(new Directory(directoryName));
}
    void removeDirectory(const string &fileName) {
    auto it = remove_if(entries.begin(), entries.end(), [fileName](const File *entry) {
        return entry->getName() == fileName;
    });
    entries.erase(it, entries.end());
}
    void rm(Directory *currentDir, const string &fileName) override {
    auto entry = find_if(entries.begin(), entries.end(), [fileName](const File *entry) {
        return entry->getName() == fileName;
    });

    if (entry != entries.end()) {
        File *fileToRemove = *entry;
        // Ensure the file is a regular file (not a directory)
        if (fileToRemove->isRegularFile()) {
            entries.erase(entry);
            // Using the erase-remove idiom to remove elements from the vector
            entries.erase(std::remove_if(entries.begin(), entries.end(), [fileName](File* entry) {
                return entry->isSoftLinkedFile() && dynamic_cast<SoftLinkedFile*>(entry)->getTarget() == fileName;
            }), entries.end());
     
        }
        else if(fileToRemove->isSoftLinkedFile()){
            entries.erase(entry);
        } else {
            cout << "rm: cannot remove '" << fileName << "': Is a directory" << endl;
            entries.push_back(fileToRemove);  // Restore the entry since it's not removed
        }
    } else {
        cout << "rm: cannot remove '" << fileName << "': No such file or directory" << endl;
    }
}
    void removeDirectoryRecursively(Directory *dirToRemove) {
    for (auto entry : dirToRemove->getEntries()) {
        if (entry->isDirectory()) {
            removeDirectoryRecursively(dynamic_cast<Directory *>(entry));
        } else {
            // Remove regular file
            delete entry;
        }
    }
    // Remove the directory itself
    delete dirToRemove;
}

    void rmdir(Directory *currentDir, const string &dirName) override {
    auto entry = find_if(entries.begin(), entries.end(), [dirName](const File *entry) {
        return entry->getName() == dirName;
    });
    if (entry != entries.end() && (*entry)->isDirectory()) {
        Directory *dirToRemove = dynamic_cast<Directory *>(*entry);
        // Check if the directory has content
        if (!dirToRemove->getEntries().empty()) {
            cout << "rmdir: failed to remove '" << dirName << "': Directory not empty" << endl;
        } else {      
            currentDir->removeDirectory(dirName);
            removeDirectoryRecursively(dirToRemove);
            // Using the erase-remove idiom to remove elements from the vector
            entries.erase(std::remove_if(entries.begin(), entries.end(), [dirName](File* entry) {
                return entry->isSoftLinkedFile() && dynamic_cast<SoftLinkedFile*>(entry)->getTarget() == dirName;
            }), entries.end());
        }
    } else {
        cout << "rmdir: failed to remove '" << dirName << "': Not a directory" << endl;
    }
}

    void copyFile(const string& sourceFile, const string& destinationFile) {
        ifstream source(sourceFile, ios::binary);
        if (!source.is_open()) {
            cout<<"cp: cannot stat '"<<sourceFile<<"': No such file or directory"<<endl;
            return;
        }
        // Open the destination file for writing
        ofstream destination(destinationFile, ios::binary);
        if (!destination.is_open()) {
            throw invalid_argument("Unable to create or open destination file");
            return;
        }
        // Copy data from source to destination
        streamsize a;
        char buffer[2048];
        while (!source.eof()) {
            source.read(buffer, sizeof(buffer));
            streamsize bytesRead = source.gcount();
            if (bytesRead > 0) {
                destination.write(buffer, bytesRead);
                a = bytesRead;
            }
        }
        File* newFile = new RegularFile(destinationFile);
        newFile->setParent(parent);
        entries.push_back(newFile);
        RegularFile* b = dynamic_cast<RegularFile*>(newFile);
        b->setSize(a);
        // Close the files
        source.close();
        destination.close();
    }
    
    void copyDirectory(Directory* currentDir, const string& sourceDir, const string& destinationDirName) {
    // Create a new directory in the current directory
    	if (currentDir->findDirectory(destinationDirName) != nullptr) {
        	return;
    	}
        Directory* newDirectory = new Directory(destinationDirName);
        currentDir->addDirectory(newDirectory);
        // Get the source directory
        Directory* sourceDirectory = currentDir->findDirectory(sourceDir);
        if (sourceDirectory != nullptr) {
            // Copy the contents of the source directory to the new directory
            for (const auto& entry : sourceDirectory->getEntries()) {
                if (entry->isDirectory()) {
                    // If the entry is a directory, recursively call copyDirectory
                    Directory* subDir = dynamic_cast<Directory*>(entry);
                    newDirectory->copyDirectory(newDirectory, entry->getName(), entry->getName());
                } else {
                    // If the entry is a file, copy it
                    RegularFile* file = dynamic_cast<RegularFile*>(entry);
                    copyRegularFile(sourceDirectory,currentDir, entry->getName(), newDirectory->getName());
                }
            }
        } else {
            cerr << "copyDirectory: Source directory not found: " << sourceDir << endl;
        }
    }

    void addFile(File* file) {
        // Check if the file is not null
        if (file != nullptr) {
            // Set the parent directory for the file
            file->setParent(this);
            // Add the file to the list of entries
            entries.push_back(file);
        } else {
            cerr << "addFile: Attempting to add a null file." << endl;
        }
    }

    void copyRegularFile(Directory* prevDirectory, Directory* currentDir, const string& sourceFilePath, const string& destinationDirName) {
        // Get the source file
        File* sourceFile = prevDirectory->findFile(sourceFilePath);
        if (sourceFile != nullptr && sourceFile->isRegularFile()) {
            // Extract the file name from the source file path
            size_t lastSlash = sourceFilePath.find_last_of('/');
            string fileName = (lastSlash != string::npos) ? sourceFilePath.substr(lastSlash + 1) : sourceFilePath;
            // Get the destination directory
            Directory* destinationDir = currentDir->findDirectory(destinationDirName);
            if (destinationDir != nullptr) {
		 if (destinationDir->findFile(fileName) == nullptr) {
                // Create a new regular file in the destination directory with the same name
                RegularFile* newRegularFile = new RegularFile(fileName);
                newRegularFile->setSize(dynamic_cast<RegularFile*>(sourceFile)->getSize());
                newRegularFile->setContent(dynamic_cast<RegularFile*>(sourceFile)->getContent());
                destinationDir->addFile(newRegularFile);
                // Set the parent directory for the new regular file
                newRegularFile->setParent(destinationDir);
            } else {

            }
            } else {
                cerr << "copyRegularFile: Destination directory not found: " << destinationDirName << endl;
            }
        } else {
            cerr << "copyRegularFile: Source file not found or not a regular file: " << sourceFilePath << endl;
        }
    }

    void cp(const string& sourcePath, const string& destinationPath) {
        File* sourceFile = findFile(sourcePath);
        if (sourceFile != nullptr) {
            if (sourceFile->isDirectory()) {
                dynamic_cast<Directory*>(sourceFile)->copyDirectory(this, sourcePath, destinationPath);
            }
            else if (sourceFile->isRegularFile()) {            // If the source file is a regular file, call copyRegularFile
            copyRegularFile(dynamic_cast<Directory*>(sourceFile->getParent()), this, sourcePath, destinationPath);
            }
            else{
                cerr<<"cp: cannot stat '"<<sourcePath<<"': No such file or directory"<<endl;
                
            }
        } else {
        	if (findFile(destinationPath) != nullptr) {
        }else{ 
            this->copyFile(sourcePath, destinationPath);
            }
        }
    }

    void link(const string& sourceFile, const string& destinationFile, Directory* currentDir) {
    // Get or create the destination directory
        File* sourceEntry = currentDir->findFile(sourceFile);
            if (sourceEntry == nullptr) {
                cerr << "link: cannot create link '" << sourceFile << "' to '" << destinationFile<<": No such file or directory" << endl;
                return;
            }
        File* destinationDir = currentDir->findFile(destinationFile);
        if (destinationDir == nullptr && !sourceEntry->isSoftLinkedFile()) {
            SoftLinkedFile* softLinkedFile = new SoftLinkedFile(destinationFile);
            softLinkedFile->setTarget(sourceFile);
            currentDir->addFile(softLinkedFile);
        } else if(destinationDir == nullptr && sourceEntry->isSoftLinkedFile()){
            SoftLinkedFile* softLinkedFile = new SoftLinkedFile(destinationFile);
            SoftLinkedFile* sf = dynamic_cast<SoftLinkedFile*>(sourceEntry);
            string target = sf->getTarget();
            softLinkedFile->setTarget(target);
            currentDir->addFile(softLinkedFile);
        }else {
            cout << "link: cannot create link '" << sourceFile << "' to '" << destinationFile <<": Will be broken"<< endl;
        }
    }

    void lsrecToFile(ofstream& outFile, int depth) const {
    // Write directory entries to the file
        for (const auto& entry : entries) {
            time_t creationTime = entry->getCreationTime();
            struct tm* timeInfo = localtime(&creationTime);
            char buffer[80];
            strftime(buffer, sizeof(buffer), "%b %d %H:%M", timeInfo);
            for (int i = 0; i < depth; ++i) {
                outFile << "  ";
            }
            if (entry->isDirectory()) {
                outFile << "D ";
            } else if (entry->isRegularFile()) {
                outFile << "F ";
            } else if (entry->isSoftLinkedFile()) {
                outFile << "S ";
            }
            outFile << entry->getName() << " " << buffer << " ";
            if (entry->isSoftLinkedFile()) {
                SoftLinkedFile* softLink = dynamic_cast<SoftLinkedFile*>(entry);
                outFile << softLink->getTarget() << " ";
            }
            if (entry->isRegularFile()) {
                RegularFile* rf = dynamic_cast<RegularFile*>(entry);
                outFile << rf->getSize() << "Bytes ";
            }
            outFile << endl;
            // Recursively call lsrecToFile for subdirectories
            if (entry->isDirectory()) {
                Directory* subDir = dynamic_cast<Directory*>(entry);
                subDir->lsrecToFile(outFile, depth + 1);
            }
        }
    }

    void clearFileSystem() {
    // Recursively clear subdirectories
        for (auto& entry : entries) {
            if (entry->isDirectory()) {
                Directory* subDir = dynamic_cast<Directory*>(entry);
                subDir->clearFileSystem();
                delete entry;  // Delete the subdirectory object
            } else {
                // For regular files or soft linked files, simply delete them
                delete entry;
            }
        }
        // Clear the entries vector
        entries.clear();
    }
  
    string getAbsolutePath(const string& path, Directory* currentDir) {
        if (path.empty() || path[0] == '/') {
            return path;
        } else {
            // Split the input path into components
            vector<string> components;
            size_t start = 0, end;
            while ((end = path.find('/', start)) != string::npos) {
                string component = path.substr(start, end - start);
                if (component != "." && component != "..") {
                    components.push_back(component);
                } else if (component == "..") {
                    // Move up one level
                    currentDir = getParent(currentDir);
                }
                start = end + 1;
            }
            // Handle the last component
            string lastComponent = path.substr(start);
            if (lastComponent != "." && lastComponent != "..") {
                components.push_back(lastComponent);
            } else if (lastComponent == "..") {
                // Move up one level
                currentDir = getParent(currentDir);
            }
            // Reconstruct the absolute path
            string absolutePath;
            for (const string& component : components) {
                absolutePath += '/' + component;
            }
            return (currentDir == nullptr) ? absolutePath : currentDir->getName() + absolutePath;
        }
    }

    File* findFile(const string& fileName) const {
        for (const auto& entry : entries) {
            if (entry->getName() == fileName) {
                return entry;
            }
        }
        return nullptr; // File not found
    }

    void cd(Directory*& currentDir, const string& path) override {
        string newPath = getAbsolutePath(path, currentDir);
        if (path == ".") {
        } else if (path == "..") {
            Directory* parentDir = getParent(currentDir);
            if (parentDir != nullptr) {
                currentDir = parentDir;
            } else {
            }
        } else {
            // Extracts the last part of the path (after the last '/')
            size_t pos = newPath.rfind('/');
            string dirName = (pos != string::npos) ? newPath.substr(pos + 1) : newPath;
            File* targetFile = currentDir->findFile(dirName);
        if (targetFile != nullptr && targetFile->isRegularFile()) {
            cout << "bash: cd: "<< dirName<< ": Not a directory" << endl;
        } 
        else if (targetFile != nullptr && targetFile->isSoftLinkedFile()) {
            SoftLinkedFile* target = dynamic_cast<SoftLinkedFile*>(targetFile);
            string dir = target->getTarget();
            cd(currentDir,dir);
        }
        else{
            Directory* newDirectory = currentDir->findDirectory(dirName);
            if (newDirectory != nullptr) {
                currentDir = newDirectory;
            } else {
                cout << "bash: cd: "<< dirName << ": No such file or directory" << endl;
            }
        }
        }
    }
        streamsize calculateShellSize() const {
            streamsize totalSize = 0;

            for (const auto& entry : entries) {
                totalSize += static_cast<streamsize>(entry->getName().size());

                if (entry->isRegularFile()) {
                    // If the entry is a regular file, add its byte size to the total
                    totalSize += dynamic_cast<const RegularFile*>(entry)->getSize();
                } else if (entry->isDirectory()) {
                    // If the entry is a directory, recursively add its size
                    totalSize += dynamic_cast<const Directory*>(entry)->calculateShellSize();
                }
            }
            return totalSize;
        }



        void cat(const string& fileName) const override{
            File* file = findFile(fileName);
            if (file != nullptr) {
                if (file->isRegularFile()) {
                    RegularFile* regularFile = dynamic_cast<RegularFile*>(file);
                    // Open the file for reading
                    ifstream fileStream(fileName);
                    if (fileStream.is_open()) {
                        // Print the content of the file
                        cout << fileStream.rdbuf();
                        cout << endl;
                        fileStream.close();
                    } else {
                        cerr << "cat: " << fileName << ": Unable to open file" << endl;
                    }
                } else if(file->isDirectory()){
                    Directory* dir = dynamic_cast<Directory*>(file);
                    dir->ls();
                } else if(file->isSoftLinkedFile()){
                    SoftLinkedFile* sf = dynamic_cast<SoftLinkedFile*>(file);
                    string rf=sf->getTarget();
                    cat(rf);
                }
            } else {
                cerr << "cat: " << fileName << ": No such file or directory" << endl;
            }
}

        void iterator() const override{}
        virtual bool isRegularFile()const override{return false;}
        virtual bool isSoftLinkedFile()const override{return false;}
private:
    vector<File *> entries;
    Directory* parent; 
    Directory *findDirectory(const string &name) const {
        for (auto entry : entries) {
            // Extract the last part of the path (after the last '/')
            size_t pos = entry->getName().rfind('/');
            string entryName = (pos != string::npos) ? entry->getName().substr(pos + 1) : entry->getName();

            if (entryName == name && entry->isDirectory()) {
                return dynamic_cast<Directory *>(entry);
            }
        }
        return nullptr; // Directory not found
    }
};

int getMonthIndex(const string& month) {
    // Convert month name to lowercase for case-insensitive comparison
    string lowerMonth = month;
    transform(lowerMonth.begin(), lowerMonth.end(), lowerMonth.begin(), ::tolower);

    if (lowerMonth == "Jan") {
        return 0;
    } else if (lowerMonth == "Feb") {
        return 1;
    } else if (lowerMonth == "Mar") {
        return 2;
    } else if (lowerMonth == "Apr") {
        return 3;
    } else if (lowerMonth == "May") {
        return 4;
    } else if (lowerMonth == "Jun") {
        return 5;
    } else if (lowerMonth == "Jul") {
        return 6;
    } else if (lowerMonth == "Aug") {
        return 7;
    } else if (lowerMonth == "Sep") {
        return 8;
    } else if (lowerMonth == "Oct") {
        return 9;
    } else if (lowerMonth == "Nov") {
        return 10;
    } else if (lowerMonth == "Dec") {
        return 11;
    } else {
        // Return an invalid value if the month name is not recognized
        return -1;
    }
}

void save(const string& fileName, const Directory& root) {
    ofstream outFile(fileName);
    
    if (!outFile.is_open()) {
        cerr << "Error: Unable to open file for saving: " << fileName << endl;
        return;
    }
    root.lsrecToFile(outFile,0);   
    outFile.close();
    cout << "File system saved to: " << fileName << endl;
}

void load(const string& fileName, Directory& root) {
    ifstream inFile(fileName);
    time_t creationTime;

    if (!inFile.is_open()) {
        cerr << "Error: Unable to open file for loading: " << fileName << endl;
        return;
    }
    root.clearFileSystem();
    stack<Directory*> parentStack;
    parentStack.push(&root);
    string line;
    while (getline(inFile, line)) {        
        istringstream iss(line);
        string entryType, entryName, month, day, timeString;
        iss >> entryType >> entryName >> month >> day >> timeString;
        size_t colonPos = timeString.find(':');
        if (colonPos != string::npos && colonPos > 0 && colonPos < timeString.length() - 1) {
            string hourStr = timeString.substr(0, colonPos);
            string minuteStr = timeString.substr(colonPos + 1);
            try {
                struct tm timeStruct = {};
                timeStruct.tm_mon = getMonthIndex(month); // Convert month name to numerical value
                timeStruct.tm_mday = stoi(day);
                timeStruct.tm_hour = stoi(hourStr);
                timeStruct.tm_min = stoi(minuteStr);
                creationTime = mktime(&timeStruct);
            } catch (const std::invalid_argument& e) {
                cerr << "Error: Invalid creation time format: " << entryName << endl;
                creationTime = time(nullptr);
            }
        } else {
            cerr << "Error: Invalid time format: " << timeString << endl;
            creationTime = time(nullptr); 
        }
        int depth = 0;
        while (line[depth] == ' ') {
            depth++;
        }

        if (entryType == "D") {
            Directory* newDir = new Directory(entryName, creationTime);
            while (depth + 1 < static_cast<int>(parentStack.size())) {
                parentStack.pop();
            }
            Directory* parentDir = parentStack.top();
            parentDir->addDirectory(newDir);
            parentStack.push(newDir);
            } else if (entryType == "F") {
                string contentSizeStr;
                iss >> contentSizeStr;
                string bytesString;
                iss >> bytesString;
                size_t contentSize = 0;
                try {
                    contentSize = stoul(contentSizeStr);
                } catch (const std::invalid_argument& e) {
                    cerr << "Error: Invalid content size format: " << entryName << endl;
                }
                size_t lastSlash = entryName.find_last_of('/');
                string dirName = (lastSlash != string::npos) ? entryName.substr(0, lastSlash) : "";
                while (depth + 1 < static_cast<int>(parentStack.size())) {
                    parentStack.pop();
                }
                Directory* parentDir = (!parentStack.empty()) ? parentStack.top() : &root;
                RegularFile* newFile = new RegularFile(entryName, creationTime);
                newFile->setSize(contentSize);
                parentDir->addFile(newFile);
            }
            else if (entryType == "S") {
            string target;
            iss >> target;
            size_t lastSlash = entryName.find_last_of('/');
            string dirName = (lastSlash != string::npos) ? entryName.substr(0, lastSlash) : "";
            while (depth + 1 < static_cast<int>(parentStack.size())) {
                parentStack.pop();
            }
            Directory* parentDir = (!parentStack.empty()) ? parentStack.top() : &root;
            SoftLinkedFile* newSoftLink = new SoftLinkedFile(entryName, creationTime);
            newSoftLink->setTarget(target);
            parentDir->addFile(newSoftLink);
            }else {
            cerr << "Error: Invalid entry type in the save file." << endl;
        }
    }
    inFile.close();
    cout << "File system loaded from: " << fileName << endl;
}

void Shell(Directory* currentDir){
        streamsize bytesize=0;
        while (true) {
        bytesize = currentDir->calculateShellSize();
        if(bytesize>1048576){
            cout<<"Your disk space is 10mb.Your disk space is full."<<endl;
            break;
        }
        string line;
        cout << "> ";
        getline(cin, line);
        istringstream iss(line);
        string command;
        iss >> command;
        if (command == "ls") {
        string option;
        iss >> option;
        if (option == "-R") {
            currentDir->lsrec(0);
        }
        else{
            currentDir->ls();
        }}else if (command == "cd") {
            string path;
            iss >> path;
            currentDir->cd(currentDir, path);
        } else if (command == "mkdir") {
            string dirName;
            iss >> dirName;
            currentDir->mkdir(currentDir, dirName);
        } else if (command == "cp") {
            string source, destination;
            iss >> source >> destination;
            currentDir->cp(source, destination);
        } else if (command == "rmdir") {
            string dir;
            iss >> dir;
            currentDir->rmdir(currentDir, dir);
        } else if (command == "cat") {
            string file;
            iss >> file;
            currentDir->cat(file);
        } else if (command == "exit") {
            cout << "Exiting shell." << endl;
            break;
        } else if (command == "rm") {
            string file;
            iss >> file;
            currentDir->rm(currentDir, file);
        } 
        else if (command == "link") {
            string source, destination;
            iss >> source >> destination;
            currentDir->link(source, destination, currentDir);
            }else {
            cout << "Invalid command. Try again." << endl;
        }
        if(bytesize>1048576){
            cout<<"Your disk space is 10mb!Your disk space is full."<<endl;
            break;

        }
    }    
}
