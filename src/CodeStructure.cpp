#include "CodeStructure.h"
#include <ppl.h> // parallel_for_each
#include <rpp/binary_writer.h>

namespace uho
{
    ////////////////////////////////////////////////////////////////////////////////

    CodeFile::CodeFile(string&& filename) : filename(move(filename))
    {
    }

    CodeFile::~CodeFile()
    {
    }

    // @note Only modify code via its thread-safe methods! This is called in parallel loop
    int CodeFile::analyzeIncludePatterns(CodeStructure& code)
    {
        if (buffer_line_parser parser = file::read_all(filename))
        {
            int nline = 0;
            strview line;
            while (parser.read_line(line))
            {
                ++nline;
                line.trim_start();
                if (line.starts_with("#include"))
                {
                    strview include = line;
                    include.skip(line.starts_with("#include_next") ? 13 : 8);
                    include.trim_start();

                    if (include[0] == '"')
                    {
                        include = include.next('"');

                        string fullpath = path::folder_path(filename) + include;
                        IncludeType type = file_exists(fullpath) ? IncRelCorrect : IncRelIncorrect;
                        includes.push_back(IncludePattern{ this, nline, type, include });
                    }
                    else if (include[0] == '<')
                    {
                        include = include.next("<>");
                        // should we even handle global search includes?...
                    }
                    else // most likely #include FT_FREETYPE_H or something weird like that...
                    {
                        //printf("fishy code: %s\n", line.to_cstr());
                    }
                }
            }
        }
        return (int)includes.size();
    }

    ////////////////////////////////////////////////////////////////////////////////

    CodeStructure::CodeStructure()
    {
    }

    CodeStructure::~CodeStructure()
    {
        for (CodeFile* cf : headers) delete cf;
        for (CodeFile* cf : sources) delete cf;
    }

    void CodeStructure::recursiveSearch(const string& rootPath)
    {
        vector<string> files;
        if (path::list_files(files, rootPath))
        {
            for (const strview f : files) // kind-of optimized
            {
                vector<CodeFile*>*                v = nullptr;
                unordered_map<string, CodeFile*>* m = nullptr;

                if      (f.ends_withi(".h")   || f.ends_withi(".hpp")) v = &headers, m = &map_headers;
                else if (f.ends_withi(".cpp") || f.ends_withi(".c")  ) v = &sources, m = &map_sources;

                if (v)
                {
                    CodeFile* cf = new CodeFile{ rootPath + '/' + f };
                    v->push_back(cf);
                    (*m)[cf->filename] = cf;
                }
            }
        }

        // get all subdirectories:
        vector<string> folders;
        if (!path::list_dirs(folders, rootPath))
            return;

        for (const string& folder : folders) // and traverse them
        {
            recursiveSearch(rootPath + '/' + folder);
        }
    }

    int CodeStructure::analyzeIncludePatterns()
    {
        printf("  0%%");

        atomic_int count = 0;
        atomic_int nincludes = 0;

        auto analyze = [this,&count,&nincludes](CodeFile* cf) 
        {
            nincludes += cf->analyzeIncludePatterns(*this);
            updateProgress(++count);
        };
        Concurrency::parallel_for_each(begin(headers), end(headers), analyze);
        Concurrency::parallel_for_each(begin(sources), end(sources), analyze);

        printf("\b\b\b\b100%%");
        return nincludes;
    }

    void CodeStructure::updateProgress(int count)
    {
        if (++count % 1000 == 0) {
            const size_t total = headers.size() + sources.size();
            printf("\b\b\b\b%3.0f%%", (float(count) / total) * 100.0f);
        }
    }

    int CodeStructure::numIncorrectIncludePatterns() const
    {
        atomic_int nbadinc = 0;

        auto gather = [&nbadinc](const CodeFile* cf) {
            int n = 0;
            for (const IncludePattern& ip : cf->includes)
                if (ip.type == IncRelIncorrect) ++n;
            nbadinc += n;
        };
        Concurrency::parallel_for_each(begin(headers), end(headers), gather);
        Concurrency::parallel_for_each(begin(sources), end(sources), gather);
        return nbadinc;
    }

    void CodeStructure::writeAllIncludePatterns(const string& outFileName) const
    {
        file_bufferstream_writer out { outFileName };
        if (!out)
            return;

        for (CodeFile* cf : headers)
        {
            for (const IncludePattern& ip : cf->includes) {
                out << ip.str << '\n';
            }
        }

        for (CodeFile* cf : sources)
        {
            for (const IncludePattern& ip : cf->includes) {
                out << ip.str << '\n';
            }
        }

        out.flush();
    }

    ////////////////////////////////////////////////////////////////////////////////
}
