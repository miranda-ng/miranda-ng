#include <string>

struct VoiceDesc
{
    std::wstring engine;
    std::wstring voice;
    int         volume;
    int         pitch;
    int         rate;
};