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
    else if (color == COLOR_RED) lcd_color = 0xF800;  //red
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
* Draw string to buffer and only update the screen with the deltas
* Only draws a single line - does not support multiple lines
* Buffer uses upper/lower nibbles with COLOR_MAP value of 7
*/
void drawStringBuffer(int cenX, int cenY, const char* text, sFONT *font, COLOR_MAP fore, COLOR_MAP back, int colMax) {
    int colTotal = 0;  //characters total excluding '\0'
    for (colTotal = 0; text[colTotal] != '\0'; colTotal++);  //find colTotal of text from current position t

    int text_width = font->Width * colMax;
    int text_height = font->Height;
    int text_left = cenX - (text_width/2);
    int text_top = cenY - (text_height/2);
    int text_cursor = 0;

    //blank sides
    if (colMax > 0 && colMax > colTotal) {  //draw filled rectangles to blank out old characters where the previous text was longer than current text
        int blank_width = (colMax-colTotal) * (font->Width) / 2;  //width of each blank (there are 2, one left and one right)
        int blank_height = font->Height;  //height of each blank
        int blank_left = cenX - (colMax) * (font->Width) / 2;  //left of screen drawing area
        int blank_right = cenX + (colMax) * (font->Width) / 2 - 1;  //right of screen drawing area
        int blank_top = cenY - (blank_height/2);  //top of screen drawing area
        text_cursor = blank_width;

        for (int y = 0; y < blank_height; y++) {
            for (int x = 0; x < blank_width; x++) {  //left side
                //calculate nibble
                int buffer_nibble = 0;
                int buffer_x = x;
                if (buffer_x % 2 == 0) buffer_nibble = BPM_BUFFER[buffer_x/2][y] & 0x0F;  //lower nibble
                else buffer_nibble = BPM_BUFFER[buffer_x/2][y] >> 4;  //upper nibble
                if (buffer_nibble != back) {  //buffer_nibble is not the same
                    drawFilledRectangle(blank_left + x, blank_top + y, 1, 1, COLOR_BLACK);  //update screen
                    buffer_nibble = back;  //update buffer_nibble
                }
                //update buffer
                if (buffer_x % 2 == 0) BPM_BUFFER[buffer_x/2][y] = ((BPM_BUFFER[buffer_x/2][y] & 0xF0) | buffer_nibble);  //keep upper nibble, update lower nibble
                else BPM_BUFFER[buffer_x/2][y] = ((BPM_BUFFER[buffer_x/2][y] & 0x0F) | (buffer_nibble << 4));  //keep lower nibble, update upper nibble
            }
            for (int x = 0; x < blank_width; x++) {  //right side
                //calculate nibble
                int buffer_nibble = 0;
                int buffer_x = ((colMax) * (font->Width) - 1 - x);
                if (buffer_x % 2 == 0) buffer_nibble = BPM_BUFFER[buffer_x/2][y] & 0x0F;  //lower nibble
                else buffer_nibble = BPM_BUFFER[buffer_x/2][y] >> 4;  //upper nibble
                if (buffer_nibble != back) {  //buffer_nibble is not the same
                    drawFilledRectangle(blank_right - x, blank_top + y, 1, 1, COLOR_BLACK);  //update screen
                    buffer_nibble = back;  //update buffer_nibble
                }
                //update buffer
                if ((buffer_x) % 2 == 0) BPM_BUFFER[buffer_x/2][y] = ((BPM_BUFFER[buffer_x/2][y] & 0xF0) | buffer_nibble);  //keep upper nibble, update lower nibble
                else BPM_BUFFER[buffer_x/2][y] = ((BPM_BUFFER[buffer_x/2][y] & 0x0F) | (buffer_nibble << 4));  //keep lower nibble, update upper nibble
            }
        }
    }
    //text
    while (*text != '\0') {
        uint32_t chrOffset = (*text - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
        const unsigned char *ptr = &font->table[chrOffset];
        for (int y = 0; y < font->Height; y++) {
            for (int x = 0; x < font->Width; x++) {
                //calculate nibble
                int buffer_nibble = 0;
                int buffer_x = (text_cursor + x);
                if (buffer_x % 2 == 0) buffer_nibble = BPM_BUFFER[buffer_x/2][y] & 0x0F;  //lower nibble
                else buffer_nibble = BPM_BUFFER[buffer_x/2][y] >> 4;  //upper nibble

                if (*ptr & (0x80 >> (x % 8))) {  //draw foreground pixel
                    if (buffer_nibble != fore) {  //buffer_nibble is not the same
                        drawFilledRectangle(text_left + text_cursor + x, text_top + y, 1, 1, fore);  //update screen
                        buffer_nibble = fore;  //update buffer_nibble
                    }
                }
                else {  //draw background pixel
                    if (buffer_nibble != back) {  //buffer_nibble is not the same
                        drawFilledRectangle(text_left + text_cursor + x, text_top + y, 1, 1, back);  //update screen
                        buffer_nibble = back;  //update buffer_nibble
                    }
                }
                if (x % 8 == 7) ptr++;  //One pixel is 8 bits

                //update buffer
                if (buffer_x % 2 == 0) BPM_BUFFER[buffer_x/2][y] = ((BPM_BUFFER[buffer_x/2][y] & 0xF0) | buffer_nibble);  //keep upper nibble, update lower nibble
                else BPM_BUFFER[buffer_x/2][y] = ((BPM_BUFFER[buffer_x/2][y] & 0x0F) | (buffer_nibble << 4));  //keep lower nibble, update upper nibble

            }
            if(font->Width % 8 != 0) ptr++;
        }
        text ++;
        text_cursor += font->Width;
    }
}


/*
* Initialises the screen buffer for faster BPM display updates
*/
void initBpmBuffer() {
    for (uint8_t y = 0; y < BPM_BUFFER_HEIGHT; y++)
        for (uint8_t x = 0; x < BPM_BUFFER_WIDTH; x++)
            BPM_BUFFER[x][y] = COLOR_BLACK;  //init buffer
}

/*
* Update button text, with blanking areas for up to maxChar characters
* Use t when B.name is blank - used for presets
*/
void drawButton(BUTTON *B) {
    uint16_t color = B->color;
    if (B->active == false) color = COLOR_GREY;
    if (B->frame == true) drawRoundedRectangle(B->left, B->top, B->width, B->height, B->radius, color);
    char *t = SCREENS[CURR_SCREEN_INDEX].name;
    if (B->name[0] != 0x00) t = B->name;  //use CURR_SCREEN.name when B.name is blank
    if (B->buffer == true) drawStringBuffer((B->left + B->width/2)+1, (B->top + B->height/2)+2, t, B->font, color, COLOR_BLACK, B->chars);
    else drawString((B->left + B->width/2)+1, (B->top + B->height/2)+2, t, B->font, color, COLOR_BLACK, B->chars);  //offset y+2 because capitals don't need space below the letters
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
