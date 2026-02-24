#ifndef PPMHEADER
#define PPMHEADER

class PPMFile {
    public:
        int height;
        int width;
        int mapSize;
        int maxColor;
        const int channels = 3;
};

// Scans a file with the name given to find the PPM metadata and pixel map
//      First argument is file name
//      Set pixmap array to second argument
//      Set metadata dictionary to third argument
//      Dependents: blur.cpp  
void readPPM( char *fileName, int **pixmap, PPMFile *metadata );

// Writes a blurred version of the previous file to a new one with the given file name
//      First argument is file name
//      Second is the blurred pixel map
//      Third is metadata with an updated max color value
//      Dependents: blur.cpp
void writePPM( char *fileName, uint8_t *pixmap, PPMFile *metadata );

#endif