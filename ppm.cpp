#include <iostream>
#include <fstream>
#include <assert.h>
#include "ppm.h"

// Not declared in ppm.h
void readComment( FILE *stream ) {
 
    int c;

    while ( ( c = getc( stream ) ) != '#' ) { 
    }

    ungetc( c, stream );
    
    while ( ( c = getc( stream ) ) != '\n') {
    }

    ungetc( c, stream ); 
}

void readPPM( char *fileName, int **pixmap, PPMFile *metadata ) {

    FILE *stream = fopen(fileName, "r");

    char magicNumber[3] = "xx";
    int result = fscanf( stream, "%2s", magicNumber );
    assert( result == 1 );

    readComment( stream );

    result = fscanf( stream, "%d %d\n", &metadata->width, &metadata->height );
    assert( result == 2 );

    result = fscanf( stream, "%d\n", &metadata->maxColor );
    assert( result == 1 );

    metadata->mapSize = ( metadata->width )*( metadata->height )*3;
    *pixmap = ( int* ) malloc( metadata->mapSize * sizeof( int ) );
    assert( *pixmap != nullptr );

    int tempChannelValue;
    int index = 0;

    while ( index < metadata->mapSize && ( fscanf( stream, "%d", &tempChannelValue ) ) == 1 ) {

        ( *pixmap )[ index ] = tempChannelValue;
        index++;
    }

    fclose( stream );


}

void writePPM( char *fileName, uint8_t *pixMap, PPMFile *metadata ) {

    printf( "Writing %s\n", fileName );

    FILE *stream = fopen( fileName, "w" );

        fprintf( stream, "P3\n" );
        fprintf( stream, "# Created by Bob\n" );
        fprintf( stream, "%d %d\n", metadata->width, metadata->height );
        fprintf( stream, "%d\n", metadata->maxColor );

        for ( int index=0; index<metadata->mapSize; index++ ) {

            fprintf( stream, "%d\n", pixMap[ index ] );

        }

    fclose( stream );
    
}
