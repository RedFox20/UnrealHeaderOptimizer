#pragma once
#ifndef UHO_CODESTRUCTURE_H
#define UHO_CODESTRUCTURE_H
#include <vector>
#include <string>
//#include <memory> // unique_ptr
#include <thread>
#include <unordered_map>
#include <rpp/file_io.h>

namespace uho
{
    using namespace std;
    using namespace rpp;


    enum IncludeType 
    {
        IncNone,         // unknown/default
        IncRelCorrect,   // someone used #include "file.h" and the file exists in immediate dir
        IncRelIncorrect, // someone used #include "file.h" but it doesn't exist in immediate dir
    };
    

    struct IncludePattern      // holds information regarding a single include
    {
        struct CodeFile* file; // in header/source file
        int              line; // at line
        IncludeType      type; // what pattern?
        string           str;  // include string contents without " or <>, ex: Engine.h

        IncludePattern(struct CodeFile* file, int line, IncludeType type, strview s)
            : file(file), line(line), type(type), str(s.str, s.len)
        {
        }
    };

    
    struct CodeFile      // gives information on a single code file
    {
        string filename; // full file path
        vector<IncludePattern> includes;

        CodeFile(string&& filename);
        ~CodeFile();
        int analyzeIncludePatterns(class CodeStructure& code);
    };


    struct TypeDefiniton     // handles class, struct, union, typedef, using
    {
        string name;         // name of the type, ex: struct FString --> "FString"
        CodeFile* codefile;
    };


    class CodeStructure  // contains the entire analyzed code structure
    {
    public:

        // not going to bother with unique_ptr<> here
        vector<CodeFile*> headers; // all header files
        vector<CodeFile*> sources; // all code files

        unordered_map<string, CodeFile*> map_headers; // fast lookup
        unordered_map<string, CodeFile*> map_sources;

        unordered_map<string, TypeDefiniton> types;

        CodeStructure();
        ~CodeStructure();

        // find all .h and .cpp files for analysis
        void recursiveSearch(const string& rootPath);

        // gets all the metadata from headers and sources, regarding #include patterns
        int analyzeIncludePatterns();
        void updateProgress(int count);

        int numIncorrectIncludePatterns() const;


        void writeAllIncludePatterns(const string& outFileName) const;

    };

}

#endif // UHO_CODESTRUCTURE_H
