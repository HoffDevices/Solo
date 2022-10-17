/*
* draw.c
* Contains LCD drawing methods
*/




/*
* Draw filled rectangle - this is the base LCD drawing access method used by everything else to draw
* We need to pivot the X and Y and reverse the X direction to get a proper landscape LTR setup
*/
void drawFilledRectangle(int left, int top, int width, int height, COLOR_MAP color) {
    uint16_t lcd_color;
    if      (color == COLOR_GREY) lcd_color = 0x7BEF;  //grey
    else if (color == COLOR_WHITE) lcd_color = 0xFFFF;  //white
    else if (color == COLOR_GREEN) lcd_color = 0x07E0;  //green
    else if (color == COLOR_CYAN) lcd_color = 0x07FF;  //cyan
    else if (color == COLOR_BLUE) lcd_color = 0x4A7F;  //blue (slightly pastel)
    else if (color == COLOR_RED) lcd_color = 0xFBEF;  //red (slightly pastel)
    LCD_SetWindow(top, LCD_WIDTH - (left + width), (top + height) - 1, LCD_WIDTH - left - 1);
    LCD_SetColor(lcd_color, width, height);
}


/*
* Draw filled round corners, xw and yh apart
*/
void drawRoundCorners(int left, int top, int width, int height, int radius, COLOR_MAP c, bool topright, bool topLeft, bool botRight, bool botLeft)
{
    //Draw a circle from(0, R) as a starting point
    POINT X_Center = left + width/2;
    POINT Y_Center = top + height/2;
    int offsetX = width/2 - radius;
    int offsetY = height/2 - radius;
    int16_t XCurrent, YCurrent;
    XCurrent = 0;
    YCurrent = radius;

    //Cumulative error, judge the next point
    int16_t Esp = 3 - (radius << 1 );

    while(XCurrent <= YCurrent ) {
        if (botRight) {
            drawFilledRectangle(X_Center + XCurrent + offsetX, Y_Center + YCurrent + offsetY, 1, 1, c);
            drawFilledRectangle(X_Center + YCurrent + offsetX, Y_Center + XCurrent + offsetY, 1, 1, c);
        }
        if (botLeft) {
            drawFilledRectangle(X_Center - XCurrent - offsetX, Y_Center + YCurrent + offsetY, 1, 1, c);
            drawFilledRectangle(X_Center - YCurrent - offsetX, Y_Center + XCurrent + offsetY, 1, 1, c);
        }
        if (topLeft) {
            drawFilledRectangle(X_Center - YCurrent - offsetX, Y_Center - XCurrent - offsetY, 1, 1, c);
            drawFilledRectangle(X_Center - XCurrent - offsetX, Y_Center - YCurrent - offsetY, 1, 1, c);
        }
        if (topright) {
            drawFilledRectangle(X_Center + XCurrent + offsetX, Y_Center - YCurrent - offsetY, 1, 1, c);
            drawFilledRectangle(X_Center + YCurrent + offsetX, Y_Center - XCurrent - offsetY, 1, 1, c);
        }
        if(Esp < 0 )
            Esp += 4 * XCurrent + 6;
        else {
            Esp += 10 + 4 * (XCurrent - YCurrent );
            YCurrent --;
        }
        XCurrent ++;
    }
}

/*
* Draw rounded rectangle
*/
void drawRoundedRectangle(int left, int top, int width, int height, int radius, COLOR_MAP color) {
    drawRoundCorners(left,top,width,height,radius,color, true, true, true, true);
    drawFilledRectangle(left+radius,top+0,width-radius-radius,1,color);
    drawFilledRectangle(left+radius,top+height,width-radius-radius,1,color);
    drawFilledRectangle(left+0,top+radius,1,height-radius-radius,color);
    drawFilledRectangle(left+width,top+radius,1,height-radius-radius,color);
}

/*
* Draw centered string with fore color
* Draw background (slower) between text pixels if colMax > 0 (slower)
* Draw blanking rectangles to overwrite older longer text (slower) if colMax > 0 (slower)
*/
void drawString(int cenX, int cenY, const char* text, sFONT *font, COLOR_MAP fore, COLOR_MAP back, int colMax) {
    int colTotal = 0;  //characters in one line excluding '\n' and '\0'
    int rowTotal = 0;  //number of lines (occurrances of '\n')
    int row = 0;  //text row counter
    int col = 0;  //text column counter
    int left = 0;  //calculated for each character placement
    int top = 0;  //calculated for each character placement
    int t = 0;  //string character counter
    for (int i = 0; text[i] != '\0'; i++) if (text[i] == '\n') rowTotal++;  //find rowTotal of text
    for (colTotal = 0; text[colTotal] != '\0' && text[colTotal] != '\n'; colTotal++);  //find colTotal of text from current position t
    if (colMax > 0 && colMax > colTotal) {  //draw filled rectangles to blank out old characters where the previous text was longer than current text
        left = cenX-1 - (colMax) * (font->Width) / 2;
        top = cenY-1 - (rowTotal + 1) * (font->Height) / 2;
        drawFilledRectangle(left, top, (colMax-colTotal) * (font->Width) / 2, font->Height, back);  //left side
        left = cenX-1 + (colMax) * (font->Width) / 2 - (colMax-colTotal) * (font->Width) / 2;
        top = cenY-1 - (rowTotal + 1) * (font->Height) / 2;
        drawFilledRectangle(left, top, (colMax-colTotal) * (font->Width) / 2, font->Height, back);  //right side
    }
    for (int t = 0; text[t] != '\0'; t++) {
        if (text[t] == '\n') {  //don't print \n, but goto new line
            row++;  //new row
            col = 0;  //reset colums
            for (colTotal = 0; text[colTotal+t+1] != '\0' && text[colTotal+t+1] != '\n'; colTotal++);  //find colTotal of text from current position t+1 to skip \n
            if (colMax != 0 && colMax > colTotal) {  //draw filled rectangles to blank out old characters where the previous text was longer than current text
                top = cenY-1 - (rowTotal + 1) * (font->Height) / 2 + (row) * (font->Height);
                left = cenX-1 - (colMax) * (font->Width) / 2;
                drawFilledRectangle(left, top, (colMax-colTotal) * (font->Width) / 2, font->Height, back);  //left side
                left = cenX-1 + (colMax) * (font->Width) / 2 - (colMax-colTotal) * (font->Width) / 2;
                drawFilledRectangle(left, top, (colMax-colTotal) * (font->Width) / 2, font->Height, back);  //right side
            }
        }
        else {
            left = cenX-1 - (colTotal) * (font->Width) / 2 + (col) * (font->Width);
            top = cenY-1 - (rowTotal + 1) * (font->Height) / 2 + (row) * (font->Height);
            uint32_t chrOffset = (text[t] - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
            const unsigned char *ptr = &font->table[chrOffset];
            for (int y = 0; y < font->Height; y++) {
                for (int x = 0; x < font->Width; x++) {
                    if (*ptr & (0x80 >> (x % 8))) drawFilledRectangle((left + x), (top + y), 1, 1, fore);
                    else if (colMax > 0) drawFilledRectangle((left + x), (top + y), 1, 1, back);
                    if (x % 8 == 7) ptr++;  //One pixel is 8 bits
                }
                if(font->Width % 8 != 0) ptr++;
            }
            col++;
        }
    }
}

/*
* Update button text, with blanking areas for up to maxChar characters
* Use t when B.name is blank - used for presets
*/
void drawButton(BUTTON B, uint8_t maxChar) {
    uint16_t color = B.color;
    if (B.active == false) color = COLOR_GREY;
    if (B.frame == true) drawRoundedRectangle(B.left, B.top, B.width, B.height, B.radius, color);
    char *t = SCREENS[CURR_SCREEN_INDEX].name;
    if (B.name[0] != 0x00) t = B.name;  //use CURR_SCREEN.name when B.name is blank
    if (maxChar > 0) drawString((B.left + B.width/2)+1, (B.top + B.height/2)+2, t, B.font, color, COLOR_BLACK, maxChar);  //offset y+2 because capitals don't need space below the letters
    else drawString((B.left + B.width/2)+1, (B.top + B.height/2)+2, t, B.font, color, 0, -1);  //offset y+2 because capitals don't need space below the letters
}

/*
* Show logo for (t) ms
*/
void showLogo(int t) {
    drawFilledRectangle(0,0,LCD_WIDTH,LCD_HEIGHT,COLOR_BLACK);
    for (int r=40; r<52; r++) {
        drawRoundCorners(LCD_WIDTH/2-r,LCD_HEIGHT/2-r,r*2,r*2,r,COLOR_RED, true, true, true, true);
    }
    drawFilledRectangle(LCD_WIDTH/2-20, LCD_HEIGHT/2-6, 40, 12, COLOR_RED);
    drawFilledRectangle(LCD_WIDTH/2-75, LCD_HEIGHT/2-51, 12, 102, COLOR_RED);
    drawFilledRectangle(LCD_WIDTH/2+64, LCD_HEIGHT/2-51, 12, 102, COLOR_RED);
    for (int r=45; r<57; r++) {
        drawRoundCorners(LCD_WIDTH/2-r + 88,LCD_HEIGHT/2-r+50,r*2,r*2,r,COLOR_RED, true, false, false, false);
        drawRoundCorners(LCD_WIDTH/2-r - 88,LCD_HEIGHT/2-r-51,r*2,r*2,r,COLOR_RED, false, false, false, true);
    }
    busy_wait_ms(t);
}

/*
* Show info for (t) ms
* Used for logging issues
*/
void showInfo(char *text, int t) {
    drawFilledRectangle(0,0,LCD_WIDTH,LCD_HEIGHT,COLOR_BLACK);
    drawString(160, 120, text, &Font30x17, COLOR_RED, 0, -1);
    busy_wait_ms(t);
}
