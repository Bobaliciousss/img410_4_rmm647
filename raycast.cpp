#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <cstring>
#include <string>
#include <limits>
#include <cmath>
#include "ppm.h"

extern "C" {
    #include "v3math.h"
}

struct camera {

    float width;
    float height;

};

// Base class
struct shape {

    float *position;
    float *cDiff;

    virtual ~shape() {}

    // float *rayOrigin, float *rayDirection
    virtual float intersect( float *R_o, float *R_d ) {
        std::cerr << "Error: Cannot call intersect of base class \'Shape.\'\n";
        return 0.0f;
    }
    virtual std::string getShapeType() {
        std::string shapeType = "Base";
        return shapeType;
    }
    virtual void setRadius( float radius ) {
        std::cerr << "Error: Cannot assign radius to non-sphere class.\n";
    }
    virtual void setNormal( float *normal ) {
        std::cerr << "Error: Cannot assign normal to non-plane class.\n";
    }

};

struct sphere : shape {

    float radius;

    float intersect( float *R_o, float *R_d ) {

        //float A = ( R_d[0] * R_d[0] ) + ( R_d[1] * R_d[1] ) + ( R_d[2] * R_d[2] ); // Should always equal 1
        //float A = 1;
        float B = 2.0f * ( R_d[0] * ( R_o[0] - this->position[0] ) 
                         + R_d[1] * ( R_o[1] - this->position[1] ) 
                         + R_d[2] * ( R_o[2] - this->position[2] ) );
        float C = ( R_o[0] - this->position[0] ) * ( R_o[0] - this->position[0] )
                + ( R_o[1] - this->position[1] ) * ( R_o[1] - this->position[1] )
                + ( R_o[2] - this->position[2] ) * ( R_o[2] - this->position[2] )
                - ( this->radius * this->radius );

        float discriminant = ( B * B ) - ( 4 * C );
        
        if ( discriminant < 0 ) {
            return std::numeric_limits<float>::infinity();
        }

        float t0 = ( -1 * B - std::sqrt( discriminant ) ) / 2.0f;
        float t1 = ( -1 * B + std::sqrt( discriminant ) ) / 2.0f;
        
        if ( t1 < t0 && t1 >= 0 ) 
            return t1;
        else if ( t0 >= 0 ) 
            return t0;
        
        return std::numeric_limits<float>::infinity();

    }
    virtual std::string getShapeType() {
        std::string shapeType = "Sphere";
        return shapeType;
    }
    void setRadius(  float rds ) {
        this->radius = rds;
    }

};

struct plane : shape {

    float *normal;

    float intersect( float *R_o, float *R_d ) {
        
        float magnitude = v3_length( this->position );
        float t = ( this->normal[0] * R_o[0] + this->normal[1] * R_o[1] + this->normal[2] * R_o[2] + magnitude ) 
                / ( this->normal[0] * R_d[0] + this->normal[1] * R_d[1] + this->normal[2] * R_d[2] );

        if ( t >= 0 ) 
            return t; 

        return std::numeric_limits<float>::infinity();
    }
    virtual std::string getShapeType() {
        std::string shapeType = "Plane";
        return shapeType;
    }
    void setNormal(  float *nml ) {
        this->normal = nml;
    }

};

int readScene( char *sceneFileName, shape ***objects, camera *camera, int *numberOfShapes ) {

    printf("\nReading %s\n\n", sceneFileName);

    FILE *stream = fopen( sceneFileName, "r" );
    assert( stream != NULL );

    char magicChars[12];
    assert( fscanf( stream, "%s ", magicChars ) == 1 );
    std::string sceneHeader = magicChars; // For comparison
    std::string identityHeader = "img410scene";
    if ( sceneHeader != identityHeader ) {
        std::cerr << "Error: Invalid file format.\n";
        return 1;
    }

    char tempObject[10];
    std::string tempObjString = tempObject;
    std::string endOfScene = "end";

    assert( fscanf( stream, "%s ", tempObject ) == 1 ); // Primer
    tempObjString = tempObject;
    int objectsTableIndex = -1; // Start at -1 since it increments when a new one is read

    while ( tempObjString != endOfScene ) {

        if ( tempObjString == "sphere" ) {
            objectsTableIndex ++;
            ( *objects )[ objectsTableIndex ] = new sphere;
        }
        else if ( tempObjString == "plane" ) {
            objectsTableIndex ++;
            ( *objects )[ objectsTableIndex ] = new plane;
        }

        int c = fgetc( stream ); // Primer
        char tempProperty[10];
        std::string tempPropString = tempProperty;

        while ( c != '\n' )
        {
            
            ungetc( c, stream );
            
            int tempInnerResult = fscanf( stream, "%s", tempProperty );
            assert( tempInnerResult > 0 && tempInnerResult < 5 );
            tempPropString = tempProperty;

            if ( tempPropString == "height:" ) {

                assert( fscanf( stream, "%f", &( camera->height ) ) == 1 );

            }
            else if ( tempPropString == "width:" ) {

                assert( fscanf( stream, "%f", &( camera->width ) ) == 1 );

            }
            else if ( tempPropString == "position:" ) {

                float *position = new float[3];
                fscanf( stream, "%f %f %f", &( position[0] ), &( position[1] ), &( position[2] ) );
                ( *objects )[ objectsTableIndex ]->position = position;

            }
            else if ( tempPropString == "radius:" ) {

                float radius;
                fscanf( stream, "%f", &radius );
                ( *objects )[ objectsTableIndex ]->setRadius( radius );
                
            }
            else if ( tempPropString == "normal:" ) {

                float *normal = new float[3];
                fscanf( stream, "%f %f %f", &( normal[0] ), &( normal[1] ), &( normal[2] ) );
                ( *objects )[ objectsTableIndex ]->setNormal( normal );
                

            }
            else if ( tempPropString == "c_diff:" ) {

                float *cDiff = new float[3];
                fscanf( stream, "%f %f %f", &( cDiff[0] ), &( cDiff[1] ), &( cDiff[2] ) );
                ( *objects )[ objectsTableIndex ]->cDiff = cDiff;

            }

            c = fgetc( stream );

        }

        assert( fscanf( stream, "%s ", tempObject ) == 1 );
        tempObjString = tempObject;
        
    }

    fclose( stream );

    *numberOfShapes = ( objectsTableIndex + 1 );

    return 0;

}

int main(int argc, char *argv[])
{
    
    if ( argc != 5 ) {

        printf( "Usage: raycast width height input.scene output.ppm\n" );
        return 1;

    }
    else {

        int maxShapes = 128;
        shape **objects = new shape*[ maxShapes ];
        int numberOfShapes;
        camera camera;

        if ( readScene( argv[3], &objects, &camera, &numberOfShapes ) == 1 ) {
            return 1; // Invalid file format
        }
        else {

            assert( numberOfShapes > 0 );
            int imgWidth = std::stof(argv[1] );
            int imgHeight = std::stof( argv[2] );
            float R_o[3] = { 0, 0, 0 };
            uint8_t pixmap[ imgHeight * imgWidth * 3 ];

            for ( int imgY=0; imgY<imgHeight; imgY++ ) {

                float rDistY = -0.5f * camera.height + imgY * ( camera.height / imgHeight ) + ( camera.height / imgHeight ) / 2.0f;
                
                for ( int imgX=0; imgX<imgWidth; imgX++ ) {

                    float rDistX = -0.5f * camera.width + imgX * ( camera.width / imgWidth ) + ( camera.width / imgWidth ) / 2.0f;
                    float rVector[3] = { rDistX, rDistY, -1 }; 
                    float R_d[3] = { 0, 0, 0 };
                    v3_normalize( R_d, rVector );
                    float closestT = std::numeric_limits<float>::infinity();
                    int closestObjectIndex = -1;

                    for ( int index=0; index<numberOfShapes; index++ ) {

                        std::string objectType = objects[ index ]->getShapeType();
                        float intersectedT;

                        if ( objectType == "Sphere" ) {

                            intersectedT = objects[ index ]->intersect( R_o, R_d );

                        }
                        else if ( objectType == "Plane" ) {

                            intersectedT = objects[ index ]->intersect( R_o, R_d );

                        }
                        else {

                            std::cerr << "Error: Intersection called for invalid object.";
                            return 1;

                        }

                        if ( intersectedT < closestT ) {

                            closestT = intersectedT;
                            closestObjectIndex = index;

                        }

                        
                    }

                    uint8_t outputRGB[3] = { 0, 0, 0 };
                    int pixmapIndex = ( imgY * imgWidth * 3 + imgX * 3 );

                    if ( closestObjectIndex > -1 ) {
                        
                        outputRGB[0] = objects[ closestObjectIndex ]->cDiff[0] * 255;
                        outputRGB[1] = objects[ closestObjectIndex ]->cDiff[1] * 255;
                        outputRGB[2] = objects[ closestObjectIndex ]->cDiff[2] * 255;

                    }

                    pixmap[ pixmapIndex ] = outputRGB[0];
                    pixmap[ pixmapIndex + 1 ] = outputRGB[1];
                    pixmap[ pixmapIndex + 2 ] = outputRGB[2];

                }
            }

            PPMFile metadata;
            metadata.width = imgWidth;
            metadata.height = imgHeight;
            metadata.mapSize = imgWidth * imgHeight * metadata.channels;
            metadata.maxColor = 255;
            writePPM( argv[4], pixmap, &metadata );
            
        }

        // Deallocate
        for ( int index=0; index<numberOfShapes; index++ )
            delete objects[ index ];
        delete[] objects;

    }

    return 0;
}
