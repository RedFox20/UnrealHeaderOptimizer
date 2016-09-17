#include "Settings.h"
#include "CodeStructure.h"
using namespace uho;

static Settings      settings;
static CodeStructure structure;

int main(int argc, char** argv)
{
    if (!settings.load("optimizer.cfg"))
        return -1;

    clock_t t = clock();

    if (!folder_exists(settings.outputPath)) {
        printf("Creating dir '%s'\n", settings.outputPath.c_str());
        if (!create_folder(settings.outputPath)) {
            fprintf(stderr, "Failed to create output path!\n");
            return -1;
        }
    }

    printf("Collecting source files...");
    structure.recursiveSearch(settings.sourceRoot);
    printf("\nFound:\n");
    printf("    %4ld .h   files\n", structure.headers.size());
    printf("    %4ld .cpp files\n", structure.sources.size());

    printf("Analyzing inefficient #include \"*\" patterns...");
    int nincludes = structure.analyzeIncludePatterns();
    int nbadinc   = structure.numIncorrectIncludePatterns();
    printf("\nFound:\n");
    printf("    %4ld total includes\n", nincludes);
    printf("    %4ld broken includes\n", nbadinc);
    printf("\n");

    //printf("Writing all include patterns for debugging...");
    //structure.writeAllIncludePatterns("include_patterns.txt");
    //printf("\nWrote: include_patterns.txt\n");
    //printf("\n");


    printf("Writing fixed files to '%s'\n", settings.outputPath.c_str());

    double e = double(clock() - t) / CLOCKS_PER_SEC;
    printf("Total time fixing headers: %.2fs\n", e);
    return 0;
}
