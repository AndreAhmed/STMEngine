#include "device.h"
#include <float.h>

Device::Device(SDL_Surface* _screen)
    :screen(_screen), renderWidth(screen->w), renderHeight(screen->h)
{
    depthBuffer = new float[renderWidth * renderHeight];
}

Device::~Device()
{
    if (depthBuffer)
    {
        delete[] depthBuffer;
    }
}

// Clears the screen buffer to the given color
void Device::Clear(Color color)
{
    Uint32* pixels = (Uint32 *)screen->pixels;
    Uint32 screenColor = SDL_MapRGBA(screen->format, color.r, color.g, color.b, color.a);

    for (int i = 0; i < renderWidth * renderHeight; ++i)
    {
        pixels[i] = screenColor;
        depthBuffer[i] = 0.0f;
    }
}

Color Device::GetPixel(int x, int y)
{
	Uint32 index = x + y * renderWidth;
	Uint32* pixels = (Uint32 *)screen->pixels;
	Color ret;
	SDL_GetRGBA(pixels[index], screen->format, &(ret.r), &(ret.g), &(ret.b), &(ret.a));
	return ret;
}

// Draws a pixel to the screen ignoring the depthbuffer
void Device::PutPixel(int x, int y, Color c)
{
    Uint32* pixels = (Uint32 *)screen->pixels;
    Uint32 index = x + y * renderWidth;
    pixels[index] = SDL_MapRGBA(screen->format, c.r, c.g, c.b, c.a);
}

// Draws a pixel to the screen only if it passes our depth buffer test
void Device::PutPixel(int x, int y, float z, Color c)
{
    Uint32* pixels = (Uint32 *)screen->pixels;
    Uint32 index = x + y * renderWidth;
    if (z <= depthBuffer[index])  
    {
        return;
    }

    depthBuffer[index] = z;
    pixels[index] = SDL_MapRGBA(screen->format, c.r, c.g, c.b, c.a);
}

// Draws a point to the screen if it is within the viewport
void Device::DrawPoint(float x, float y, float z, Color color)
{
    // Clipping what's visible on screen
    if (x >= 0 && y >= 0 && x < renderWidth && y < renderHeight)
    {
        // Drawing a point
        PutPixel((int)x, (int)y, z, color);
    }
}

void Device::DrawPoint(int x, int y, const Color& c)
{
    if (x >= 0 && x < screen->w && y >= 0 && y < screen->h)
    {
        PutPixel(x, y, c);
    }
}


void WriteHexString(SDL_RWops* file, char *s)
{
    Uint32 value;
    char hex[3];

    for (int i = 0; i < SDL_strlen(s); i += 2) {
        hex[0] = s[i];
        hex[1] = s[i + 1];
        hex[2] = '\0';
        SDL_sscanf(hex, "%X", &value);
        SDL_WriteU8(file, value);
    }
}

void Device::WriteToFile(const char* filename)
{
    SDL_RWops* file = SDL_RWFromFile(filename, "w+b");
    if (file)
    {
        // Writing out a tiff (Code taken from http://paulbourke.net/dataformats/tiff/) switch with a lib later
        Uint32 numbytes = renderWidth * renderHeight * 3;

        // Header of the file
        SDL_RWwrite(file, "MM", 2, 1);
        SDL_WriteBE16(file, 42);
        Uint32 offset = numbytes + 8; // 8 bytes are from the header including this offset
        SDL_WriteBE32(file, offset);

        // Then the actual data
        Uint32* pixels = (Uint32 *)screen->pixels;

        // to avoid a bunch of file io and hopefully speed up the function we're gonna buffer pixel writes and do them at once
        Uint8* buffer = new Uint8[numbytes];
        Uint32 bufferOffset = 0;

        for (int y = 0; y < renderHeight; ++y)
        {
            for (int x = 0; x < renderWidth; ++x)
            {
                Uint32 pixelIndex = x + y * renderWidth;

                Uint32 pixel = pixels[pixelIndex];
                Color color = Color(pixel);

                buffer[bufferOffset++] = color.r;
                buffer[bufferOffset++] = color.g;
                buffer[bufferOffset++] = color.b;
            }
        }

        SDL_RWwrite(file, buffer, numbytes, 1);
        delete buffer;

        // Finally the IFD
        // The number of directory entries
        SDL_WriteBE16(file, 14);  

        /* Width tag, short int */
        WriteHexString(file, "0100000300000001");
        SDL_WriteBE16(file, renderWidth);
        WriteHexString(file, "0000");

        /* Height tag, short int */
        WriteHexString(file, "0101000300000001");
        SDL_WriteBE16(file, renderHeight);
        WriteHexString(file, "0000");

        /* Bits per sample tag, short int */
        WriteHexString(file, "0102000300000003");
        SDL_WriteBE32(file, numbytes + 182);

        /* Compression flag, short int */
        WriteHexString(file, "010300030000000100010000");

        /* Photometric interpolation tag, short int */
        WriteHexString(file, "010600030000000100020000");

        /* Strip offset tag, long int */
        WriteHexString(file, "011100040000000100000008");

        /* Orientation flag, short int */
        WriteHexString(file, "011200030000000100010000");

        /* Sample per pixel tag, short int */
        WriteHexString(file, "011500030000000100030000");

        /* Rows per strip tag, short int */
        WriteHexString(file, "0116000300000001");
        SDL_WriteBE16(file, renderHeight);
        WriteHexString(file, "0000");

        /* Strip byte count flag, long int */
        WriteHexString(file, "0117000400000001");
        SDL_WriteBE32(file, numbytes);

        /* Minimum sample value flag, short int */
        WriteHexString(file, "0118000300000003");
        SDL_WriteBE32(file, numbytes + 188);

        /* Maximum sample value tag, short int */
        WriteHexString(file, "0119000300000003");
        SDL_WriteBE32(file, numbytes + 194);

        /* Planar configuration tag, short int */
        WriteHexString(file, "011c00030000000100010000");

        /* Sample format tag, short int */
        WriteHexString(file, "0153000300000003");
        SDL_WriteBE32(file, numbytes + 200);

        /* End of the directory entry */
        WriteHexString(file, "00000000");

        /* Bits for each colour channel */
        WriteHexString(file, "000800080008");

        /* Minimum value for each component */
        WriteHexString(file, "000000000000");

        /* Maximum value per channel */
        WriteHexString(file, "00ff00ff00ff");

        /* Samples per pixel for each channel */
        WriteHexString(file, "000100010001");

        SDL_RWclose(file);
    }
}
