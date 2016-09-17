#include "Settings.h"
#include <rpp/file_io.h>
using namespace rpp;

namespace uho
{
    static string pathString(strview& line)
    {
        return line.next('"').trim().to_string();
    }

    bool Settings::load(const string& settings)
    {
        if (buffer_line_parser parser = file::read_all(settings))
        {
            strview line;
            while (parser.read_line(line))
            {
                if (line.empty())
                    continue;

                strview key = line.next('=').trim();
                if (key.equalsi("SourceRoot")) {
                    path::normalize(sourceRoot = pathString(line));
                }
                else if (key.equalsi("OutputPath")) {
                    path::normalize(outputPath = pathString(line));
                }

            }

            if (folder_exists(sourceRoot))
                return true;
            fprintf(stderr, "Error: folder '%s' does not exist!\n", sourceRoot.c_str());
            return false;
        }
        fprintf(stderr, "Error: failed to open '%s'!\n", settings.c_str());
        return false;
    }

}
