//
//  file_io.h
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/17/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#ifndef FILE_IO_H
#define FILE_IO_H

/**
 Utility method to read the given file name into a single
 character buffer. The caller will be responsible for
 freeing the returned memeory.
 
 \param filename Name of the file to read in as a buffer.
 \return Buffer representation of the input file.
 */
char * read_file(const char * filename);

#endif