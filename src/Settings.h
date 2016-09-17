#pragma once
#ifndef UHO_SETTINGS_H
#define UHO_SETTINGS_H
#include <string>

namespace uho
{
    using namespace std;

    struct Settings
    {
        string sourceRoot;
        string outputPath;

        // loads basic settings
        bool load(const string& settings);
    };
}

#endif // UHO_SETTINGS_H
