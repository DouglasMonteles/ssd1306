/*
    MIT License

    Copyright (c) 2018, Alexey Dynda

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "sdl_ssd1351.h"
#include "sdl_oled_basic.h"

static int s_activeColumn = 0;
static int s_activePage = 0;
static int s_columnStart = 0;
static int s_columnEnd = 127;
static int s_pageStart = 0;
static int s_pageEnd = 7;

static int sdl_ssd1351_detect(uint8_t data)
{
    return (data == 0xFD);
}

static uint8_t s_verticalMode = 1;

static void sdl_ssd1351_commands(uint8_t data)
{
    switch (s_commandId)
    {
        case 0xA0:
            if (s_cmdArgIndex == 0)
            {
                s_verticalMode = data & 0x01;
                s_commandId = SSD_COMMAND_NONE;
            }
            break;
        case 0x15:
            switch (s_cmdArgIndex)
            {
                case 0:
                     s_columnStart = data;
                     s_activeColumn = data;
                     break;
                case 1:
                     s_columnEnd = data;
                     s_commandId = SSD_COMMAND_NONE;
                     break;
                default: break;
            }
            break;
        case 0x75:
            switch (s_cmdArgIndex)
            {
                case 0:
                     s_activePage = data;
                     s_pageStart = data;
                     break;
                case 1:
                     s_pageEnd = data;
                     s_commandId = SSD_COMMAND_NONE;
                     break;
                default: break;
            }
            break;
        case 0x5C:
            s_ssd1351_writedata = 1;
        default:
            s_commandId = SSD_COMMAND_NONE;
            break;
    }
}


void sdl_ssd1351_data(uint8_t data)
{
    int y = s_activePage;
    int x = s_activeColumn;
    static uint8_t firstByte = 1;  /// SSD1351
    static uint8_t dataFirst = 0x00;  /// SSD1351
    if (!s_ssd1351_writedata)
    {
        if (s_commandId == SSD_COMMAND_NONE)
        {
            s_commandId = data;
            s_cmdArgIndex = -1; // no argument
        }
        else
        {
            s_cmdArgIndex++;
        }
        sdl_ssd1351_commands(data);
        return;
    }
    if (firstByte)
    {
        dataFirst = data;
        firstByte = 0;
        return;
    }
    firstByte = 1;
    SDL_SetRenderDrawColor( g_renderer, (dataFirst & 0b11111000)<<0,
                                        ((dataFirst & 0b00000111)<<5) | ((data&0b11100000)>>3),
                                        (data & 0b00011111)<<3,
                                        255 );

    SDL_Rect r;
    r.x = x * PIXEL_SIZE + BORDER_SIZE;
    r.y = y * PIXEL_SIZE + BORDER_SIZE + TOP_HEADER;
    r.w = PIXEL_SIZE;
    r.h = PIXEL_SIZE;
    // Render rect
    SDL_RenderFillRect( g_renderer, &r );

    if (s_verticalMode)
    {
        s_activePage++;
        if (s_activePage > s_pageEnd)
        {
            s_activePage = s_pageStart;
            s_activeColumn++;
            if (s_activeColumn > s_columnEnd)
            {
                s_activeColumn = s_columnStart;
            }
        }
    }
    else
    {
        s_activeColumn++;
        if (s_activeColumn > s_columnEnd)
        {
            s_activeColumn = s_columnStart;
            s_activePage++;
            if (s_activePage > s_pageEnd)
            {
                s_activePage = s_pageStart;
            }
        }
    }
}

sdl_oled_info sdl_ssd1351 =
{
    .width = 128,
    .height = 128,
    .detect = sdl_ssd1351_detect,
    .run_cmd = sdl_ssd1351_commands,
    .run_data = sdl_ssd1351_data,
};
