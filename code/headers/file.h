/*
 * Copyright (c) 2018 DDRDmakar
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

#ifndef __FILES
#define __FILES


char *read_text_file_cwd         (const char *filename);
char *read_text_file_resources   (const char *filename);
uint8_t *read_bin_file_cwd       (uint8_t *destination, const size_t maxlength, const char *filename);
uint8_t *read_bin_file_resources (uint8_t *destination, const size_t maxlength, const char *filename);
void write_text_file_cwd         (const char *filename, const char *str);
void write_text_file_resources   (const char *filename, const char *str);
void write_bin_file_cwd          (const char *filename, const uint8_t *buffer, const size_t len);
void write_bin_file_resources    (const char *filename, const uint8_t *buffer, const size_t len);


#endif
