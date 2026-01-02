#ifndef RENDERING_DEVICE_H
#define RENDERING_DEVICE_H

#include <SDL/SDL.h>
#include "color.h"

class Device
{
public:
    Device(SDL_Surface* _screen);
    ~Device();

    // Clears the screen buffer to the given color
    void Clear(Color color);

	// Grabs the color from the screen at the given coordinates
	Color GetPixel(int x, int y);

    // Puts a pixel on the screen ignoring the depthbuffer and clip checks
    void PutPixel(int x, int y, Color c = Color(0xFFFFFF));

    // Puts a pixel on the screen only if it passes our depth buffer test and ignoring clipping
    void PutPixel(int x, int y, float z, Color c = Color(0xFFFFFF));

    // Draws a point on the screen if it's within the viewport, taking into account depth
    void DrawPoint(float x, float y, float z, Color color);

    // Draws a point on the screen if it's within the viewport, ignoring depth
    void DrawPoint(int x, int y, const Color& c);
    void ClearDepth()
    {
        for (int i = 0; i < renderWidth * renderHeight; ++i)
        {
            depthBuffer[i] = 0.0f;  
        }
    }

    int Width(){ return renderWidth; }
    int Height(){ return renderHeight; }

    void WriteToFile(const char* filename);

private:
    SDL_Surface* screen;
    float* depthBuffer;
    int renderWidth;
    int renderHeight;
};

#endif