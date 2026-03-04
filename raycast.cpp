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

bool verbose = false; // Set to true to print debug statements

struct camera {

    float width;
    float height;

};

// Base class
struct shape {

    float *position;
    float *cDiff;
    float *cSpec;

    shape() {
        position = new float[3]{ 0.f, 0.f, 0.f };
        cDiff = new float[3]{ 0.f, 0.f, 0.f };
        cSpec = new float[3]{ 0.f, 0.f, 0.f };
    }

    virtual ~shape() {
        delete[] position;
        delete[] cDiff;
        delete[] cSpec;
    }

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
    virtual void setNormal( float *nml ) {
        std::cerr << "Error: Cannot assign normal to non-plane class.\n";
    }
    virtual void getNormal ( float *nml ) {
        std::cerr << "Error: Cannot get normal from non-plane class.\n";
    }

};

struct sphere : shape {

    float radius;

    sphere() {
        radius = 0.f;
    }

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

    plane() {
        normal = new float[3]{ 0.f, 0.f, 0.f };
    }

    ~plane() override {
        delete[] normal;
    }

    float intersect( float *R_o, float *R_d ) {
        
        float magnitude = v3_length( this->position );
        float t = -1.f * ( this->normal[0] * R_o[0] + this->normal[1] * R_o[1] + this->normal[2] * R_o[2] + magnitude ) 
                / ( this->normal[0] * R_d[0] + this->normal[1] * R_d[1] + this->normal[2] * R_d[2] );
        if ( t >= 0 ) 
            return t; 

        return std::numeric_limits<float>::infinity();
    }
    virtual std::string getShapeType() {
        std::string shapeType = "Plane";
        return shapeType;
    }
    void setNormal( float *nml ) {
        normal[0] = nml[0];
        normal[1] = nml[1];
        normal[2] = nml[2];
    }
    void getNormal( float *nml ) {
        nml[0] = normal[0];
        nml[1] = normal[1];
        nml[2] = normal[2];
    }

};

struct light {

    float *position;
    float *color;
    float *direction;
    float radialAtt0;
    float radialAtt1;
    float radialAtt2;
    float theta;
    float angularAtt0;
    
    light() {

        position = new float[3]{ 0.f, 0.f, 0.f };
        color = new float[3]{ 0.f, 0.f, 0.f };
        direction = new float[3]{ 0.f, 0.f, 0.f };

        radialAtt0 = 0.f;
        radialAtt1 = 0.f;
        radialAtt2 = 0.f;
        theta = 0.f;
        angularAtt0 = 0.f;

    }

    ~light() {
        delete[] position;
        delete[] color;
        delete[] direction;
    }

};

int readScene( char *sceneFileName, shape ***objects, camera *camera, int *numberOfShapes, light ***lights, int *numberOfLights ) {

    printf("\nReading %s\n\n", sceneFileName);

    FILE *stream = fopen( sceneFileName, "r" );
    assert( stream != NULL );

    char magicChars[12];
    assert( fscanf( stream, "%11s ", magicChars ) == 1 );
    std::string sceneHeader = magicChars; // For comparison
    std::string identityHeader = "img410scene";
    if ( sceneHeader != identityHeader ) {
        std::cerr << "Error: Invalid file format.\n";
        return 1;
    }

    char tempObject[32];
    std::string tempObjString = tempObject;
    std::string endOfScene = "end";

    assert( fscanf( stream, "%31s ", tempObject ) == 1 ); // Primer
    tempObjString = tempObject;
    int objectsTableIndex = -1; // Start at -1 since it increments when a new one is read
    int lightsTableIndex = -1; // Start at -1 since it increments when a new one is read

    while ( tempObjString != endOfScene ) {

        if ( tempObjString == "sphere" ) {
            objectsTableIndex ++;
            ( *objects )[ objectsTableIndex ] = new sphere;
        }
        else if ( tempObjString == "plane" ) {
            objectsTableIndex ++;
            ( *objects )[ objectsTableIndex ] = new plane;
        }
        else if ( tempObjString == "light" ) {
            lightsTableIndex ++;
            ( *lights )[ lightsTableIndex ] = new light;
        }

        int c = fgetc( stream ); // Primer
        char tempProperty[64];
        std::string tempPropString = tempProperty;

        while ( c != '\n' && c != EOF )
        {

            //std::cout << "Segfault Check: \"End\" == " << endOfScene << std::endl;
            ungetc( c, stream );
            
            int tempInnerResult = fscanf( stream, "%31s", tempProperty );
            assert( tempInnerResult > 0 );
            tempPropString = tempProperty;

            if ( verbose == true ) 
                std::cout << "Scanned " << tempPropString << " for " << tempObjString << ": ";

            if ( tempPropString == "height:" ) {

                assert( fscanf( stream, "%f", &( camera->height ) ) == 1 );

                if ( verbose == true )
                    std::cout << camera->height << std::endl;

            }
            else if ( tempPropString == "width:" ) {

                assert( fscanf( stream, "%f", &( camera->width ) ) == 1 );

                if ( verbose == true )
                    std::cout << camera->width << std::endl;

            }
            else if ( tempPropString == "position:" ) {
                
                if ( tempObjString == "light" ) {

                    fscanf(stream, "%f %f %f",
                        &( *lights )[ lightsTableIndex ]->position[0],
                        &( *lights )[ lightsTableIndex ]->position[1],
                        &( *lights )[ lightsTableIndex ]->position[2] );

                    if ( verbose == true ) {
                        std::cout << ( *lights )[ lightsTableIndex ]->position[0] << " " 
                            << ( *lights )[ lightsTableIndex ]->position[1] << " " 
                            << ( *lights )[ lightsTableIndex ]->position[2] << std::endl;
                    }
                }
                else {

                    fscanf(stream, "%f %f %f",
                        &( *objects )[ objectsTableIndex ]->position[0],
                        &( *objects )[ objectsTableIndex ]->position[1],
                        &( *objects )[ objectsTableIndex ]->position[2] );

                    if ( verbose == true ) {
                        std::cout << ( *objects )[ objectsTableIndex ]->position[0] << " " 
                            << ( *objects )[ objectsTableIndex ]->position[1] << " " 
                            << ( *objects )[ objectsTableIndex ]->position[2] << std::endl;
                    }

                }

            }
            else if ( tempPropString == "radius:" ) {

                float radius;
                fscanf( stream, "%f", &radius );
                ( *objects )[ objectsTableIndex ]->setRadius( radius );

                if ( verbose == true )
                    std::cout << radius << std::endl;
                
            }
            else if ( tempPropString == "normal:" ) {

                float normal[3] ;
                fscanf( stream, "%f %f %f", &( normal[0] ), &( normal[1] ), &( normal[2] ) );
                ( *objects )[ objectsTableIndex ]->setNormal( normal );

                if ( verbose == true ) {
                    float scannedNormal[3];
                    ( *objects )[ objectsTableIndex ]->getNormal( scannedNormal );
                    std::cout << scannedNormal[0] << " " 
                        << scannedNormal[1] << " " 
                        << scannedNormal[2] << std::endl;
                }

            }
            else if ( tempPropString == "c_diff:" ) {

                fscanf(stream, "%f %f %f",
                    &( *objects )[ objectsTableIndex ]->cDiff[0],
                    &( *objects )[ objectsTableIndex ]->cDiff[1],
                    &( *objects )[ objectsTableIndex ]->cDiff[2] );

                if ( verbose == true ) {
                    std::cout << ( *objects )[ objectsTableIndex ]->cDiff[0] << " " 
                        << ( *objects )[ objectsTableIndex ]->cDiff[1] << " " 
                        << ( *objects )[ objectsTableIndex ]->cDiff[2] << std::endl;
                }

            }
            else if ( tempPropString == "c_spec:" ) {

                fscanf(stream, "%f %f %f",
                    &( *objects )[ objectsTableIndex ]->cSpec[0],
                    &( *objects )[ objectsTableIndex ]->cSpec[1],
                    &( *objects )[ objectsTableIndex ]->cSpec[2] );

                if ( verbose == true ) {
                    std::cout << ( *objects )[ objectsTableIndex ]->cSpec[0] << " " 
                        << ( *objects )[ objectsTableIndex ]->cSpec[1] << " " 
                        << ( *objects )[ objectsTableIndex ]->cSpec[2] << std::endl;
                }

            }
            else if ( tempPropString == "color:" ) {

                fscanf(stream, "%f %f %f",
                    &( *lights )[ lightsTableIndex ]->color[0],
                    &( *lights )[ lightsTableIndex ]->color[1],
                    &( *lights )[ lightsTableIndex ]->color[2] );

                if ( verbose == true ) {
                    std::cout << ( *lights )[ lightsTableIndex ]->color[0] << " " 
                        << ( *lights )[ lightsTableIndex ]->color[1] << " " 
                        << ( *lights )[ lightsTableIndex ]->color[2] << std::endl;
                }

            }
            else if ( tempPropString == "radial_a0:" ) {

                fscanf( stream, "%f", &( *lights )[ lightsTableIndex ]->radialAtt0 );

                if ( verbose == true ) 
                    std::cout << ( *lights )[ lightsTableIndex ]->radialAtt0 << std::endl;

            }
            else if ( tempPropString == "radial_a1:" ) {

                fscanf( stream, "%f", &( *lights )[ lightsTableIndex ]->radialAtt1 );

                if ( verbose == true ) 
                    std::cout << ( *lights )[ lightsTableIndex ]->radialAtt1 << std::endl;

            }
            else if ( tempPropString == "radial_a2:" ) {

                fscanf( stream, "%f", &( *lights )[ lightsTableIndex ]->radialAtt2 );

                if ( verbose == true ) 
                    std::cout << ( *lights )[ lightsTableIndex ]->radialAtt1 << std::endl;

            }
            else if ( tempPropString == "theta:" ) {

                fscanf( stream, "%f", &( *lights )[ lightsTableIndex ]->theta );

                if ( verbose == true ) 
                    std::cout << ( *lights )[ lightsTableIndex ]->theta << std::endl;

            }
            else if ( tempPropString == "angular_a0:" ) {

                fscanf( stream, "%f", &( *lights )[ lightsTableIndex ]->angularAtt0 );

                if ( verbose == true ) 
                    std::cout << ( *lights )[ lightsTableIndex ]->angularAtt0 << std::endl;

            }
            else if ( tempPropString == "direction:" ) {

                fscanf(stream, "%f %f %f",
                    &( *lights )[ lightsTableIndex ]->direction[0],
                    &( *lights )[ lightsTableIndex ]->direction[1],
                    &( *lights )[ lightsTableIndex ]->direction[2] );

                if ( verbose == true ) {
                    std::cout << ( *lights )[ lightsTableIndex ]->direction[0] << " " 
                        << ( *lights )[ lightsTableIndex ]->direction[1] << " " 
                        << ( *lights )[ lightsTableIndex ]->direction[2] << std::endl;
                }

            }

            c = fgetc( stream );

        }

        if ( fscanf( stream, "%31s ", tempObject ) != 1 )
            break;

        tempObjString = tempObject;

        if ( verbose == true ) 
            std::cout << "\n\nGot object string: " << tempObjString << std::endl;

        if ( tempObjString == endOfScene )
            break;
        
    }

    fclose( stream );

    *numberOfShapes = ( objectsTableIndex + 1 );
    *numberOfLights = ( lightsTableIndex + 1 );

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
        shape **objects = new shape*[ maxShapes ]();

        int maxLights = 128;
        light **lights = new light*[ maxLights ]();
        
        int numberOfShapes;
        int numberOfLights;
        camera camera;

        if ( readScene( argv[3], &objects, &camera, &numberOfShapes, &lights, &numberOfLights ) == 1 ) {
            return 1; // Invalid file format
        }
        else {

            assert( numberOfShapes > 0 );
            int imgWidth = std::stoi(argv[1] );
            int imgHeight = std::stoi( argv[2] );
            float R_o[3] = { 0, 0, 0 };
            uint8_t *pixmap = new uint8_t[ imgHeight * imgWidth * 3 ];

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

                        intersectedT = objects[ index ]->intersect( R_o, R_d );

                        if ( intersectedT < closestT ) {

                            closestT = intersectedT;
                            closestObjectIndex = index;

                        }

                        
                    }

                    uint8_t outputRGB[3] = { 0, 0, 0 };
                    int flippedY = imgHeight - 1 - imgY;
                    int pixmapIndex = ( flippedY * imgWidth * 3 + imgX * 3 );

                    if ( closestObjectIndex > -1 ) {
                        
                        outputRGB[0] = objects[ closestObjectIndex ]->cDiff[0] * 255;
                        outputRGB[1] = objects[ closestObjectIndex ]->cDiff[1] * 255;
                        outputRGB[2] = objects[ closestObjectIndex ]->cDiff[2] * 255;

                    }

                    for ( int lightIndex=0; lightIndex<numberOfLights; lightIndex++ ) {

                        if ( closestT == std::numeric_limits<float>::infinity() ) {

                            if ( verbose == true )
                                std::cout << "No intersecton, in shadow." << std::endl;

                        }
                        else {

                            if ( verbose == true )
                                std::cout << "\nCasting ray for intersection " << imgX << ", " << imgY << std::endl;

                            float L_o[3] = { 0, 0, 0 }; // Increment a tiny amount from the closest T to remove z-fighting
                            L_o[0] = R_o[0] + R_d[0] * ( closestT );
                            L_o[1] = R_o[1] + R_d[1] * ( closestT );
                            L_o[2] = R_o[2] + R_d[2] * ( closestT );

                            

                            float fromIntersectionToLight[3] = { 0, 0, 0 };
                            v3_from_points( fromIntersectionToLight, L_o, lights[ lightIndex ]->position );
                            float L_d[3] = { 0, 0, 0 };
                            v3_normalize( L_d, fromIntersectionToLight );
                            float toLightMagnitude = v3_length( fromIntersectionToLight );

                            float intersectedT = std::numeric_limits<float>::infinity();
                            bool inShadow = false;

                            for ( int objectIndex=0; objectIndex<numberOfShapes; objectIndex++ ) {

                                if ( objectIndex != closestObjectIndex ) {

                                    intersectedT = objects[ objectIndex ]->intersect( L_o, L_d );

                                    if ( verbose == true )
                                        std::cout << "IntersectedT is " << objects[ objectIndex ]->getShapeType() << " " << intersectedT << std::endl;

                                    if ( intersectedT < toLightMagnitude ) {

                                        inShadow = true;
                                        break;

                                    }

                                }

                                

                            }

                            if ( verbose == true ) {

                                std::cout << "L_o is: " << L_o[0] << " " << L_o[1] << " " << L_o[2] << std::endl;
                                std::cout << "L_d is: " << L_d[0] << " " << L_d[1] << " " << L_d[2] << std::endl;

                            }

                            if ( inShadow == false ) {

                                if ( verbose == true )
                                    std::cout << "In light." << std::endl;

                                
                                
                            }
                            else {

                                if ( verbose == true )
                                    std::cout << "In shadow." << std::endl;

                                // Make shadows black
                                outputRGB[0] = 0;
                                outputRGB[1] = 0;
                                outputRGB[2] = 0;

                            }

                            

                        }

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
            
            delete[] pixmap;

        }

        // Deallocate
        for ( int index=0; index<numberOfShapes; index++ )
            delete objects[ index ];

        delete[] objects;

        for ( int index=0; index<numberOfLights; index++ )
            delete lights[ index ];

        delete[] lights;

        

    }

    return 0;
}
