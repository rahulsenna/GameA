#include "basic_shapes.h"
#include "shader.h"

void uploadToGPU(shape *data, std::vector<float> vertices, std::vector<unsigned int> indices,
                 std::vector<float> normals, std::vector<float> text_coord = {})
{
    data->VAO = createVAO();

    u32 VertVBO = createVBO(vertices);
    createEBO(indices);

    linkAttrib(VertVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void *) 0);

    u32 NormVBO = createVBO(normals);
    linkAttrib(NormVBO, 1, 3, GL_FLOAT, 3 * sizeof(float), (void *) 0);

    u32 UV_VBO = createVBO(text_coord);
    linkAttrib(UV_VBO, 2, 2, GL_FLOAT, 2 * sizeof(float), (void *) 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

shape CreateFrustum(std::vector<vec4> &f)
{
    shape box = {};

    box.numIndices = 36;

    // clang-format off
    std::vector<float> vertices = 
    {    
        f[0].x, f[0].y,f[0].z,
        f[1].x, f[1].y,f[1].z,
        f[2].x, f[2].y,f[2].z,
        f[3].x, f[3].y,f[3].z,
        f[4].x, f[4].y,f[4].z,
        f[5].x, f[5].y,f[5].z,
        f[6].x, f[6].y,f[6].z,
        f[7].x, f[7].y,f[7].z,
    };

    


    // Indices
    std::vector<unsigned int> indices =
    {
        6,2,1,
        1,5,6,
        7,4,0,
        0,3,7,
        6,5,4,
        4,7,6,
        2,3,0,
        0,1,2,
        5,1,0,
        0,4,5,
        6,7,3,
        3,2,6      
    };
    // clang-format on
    std::vector<float> normals(vertices.size(), 1);

    uploadToGPU(&box, vertices, indices, normals);
    return (box);
}

shape CreateBoxMinMax(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
{
    shape box = {};

    box.numIndices = 36;

    // clang-format off
    std::vector<float> vertices = 
    {    
        minX, minY, maxZ,//        7--------6
        maxX, minY, maxZ,//       /|       /|
        maxX, minY, minZ,//      4--------5 |
        minX, minY, minZ,//      | |      | |
        minX, maxY, maxZ,//      | 3------|-2
        maxX, maxY, maxZ,//      |/       |/
        maxX, maxY, minZ,//      0--------1
        minX, maxY, minZ
    };

    


    // Indices
    std::vector<unsigned int> indices =
    {
     // Right
        6,2,1,
        1,5,6,
     // Left
        7,4,0,
        0,3,7,
     // Top
        6,5,4,
        4,7,6,
     // Bottom
        2,3,0,
        0,1,2,
     // Front
        5,1,0,
        0,4,5,
     // Back
        6,7,3,
        3,2,6      
    };
    // clang-format on
    std::vector<float> normals(vertices.size(), 1);

    uploadToGPU(&box, vertices, indices, normals);
    return (box);
}

shape createBox(float w, float h, float l)
{
    float nw = w / (float) WIDTH;
    float nh = h / (float) HEIGHT;
    float nl = l / 1000.f;
    w *= 0.5f;
    h *= 0.5f;
    l *= 0.5f;

    shape box = {};

    box.numIndices = 36;

    // clang-format off
    std::vector<float> vertices = 
    {    
	    -w, -h,  l,//        7--------6
	     w, -h,  l,//       /|       /|
	     w, -h, -l,//      4--------5 |
	    -w, -h, -l,//      | |      | |
	    -w,  h,  l,//      | 3------|-2
	     w,  h,  l,//      |/       |/
	     w,  h, -l,//      0--------1
	    -w,  h, -l
	};

    


	// Indices
	std::vector<unsigned int> indices =
	{
	    6,2,1,
	    1,5,6,
	    7,4,0,
	    0,3,7,
	    6,5,4,
	    4,7,6,
	    2,3,0,
	    0,1,2,
	    5,1,0,
	    0,4,5,
	    6,7,3,
	    3,2,6      
	};
    // clang-format on
    std::vector<float> normals(vertices.size(), 1);

    uploadToGPU(&box, vertices, indices, normals);
    return (box);
}

shape createPlane(float base, float size, float uvScale)
{
    shape plane = {};

    plane.numIndices = 6;

    // clang-format off
    std::vector<float> vertices = 
 	{
	    -size, base, -size,
	     size, base, -size,
	     size, base,  size,
	    -size, base,  size,
    }; 
    std::vector<unsigned int> indices =
    {
	    2, 1, 0,
	    0, 3, 2
	};

        std::vector<float> normals = 
    {
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0

    };

std::vector<float> texCoords = 
    {
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    };

    // clang-format on
    uploadToGPU(&plane, vertices, indices, normals, texCoords);

    return (plane);
}

shape createSphere(float rad, uint32_t hSegs, uint32_t vSegs)
{
    shape ball      = {};
    ball.numIndices = (hSegs * vSegs * 2) * 3;

    float dphi   = (float) (2.0 * M_PI) / (float) (hSegs);
    float dtheta = (float) (M_PI) / (float) (vSegs);

    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;

    std::vector<unsigned int> indices;

    for (uint32_t v = 0; v <= vSegs; ++v)
    {
        float theta = v * dtheta;

        for (uint32_t h = 0; h <= hSegs; ++h)
        {
            float phi = h * dphi;

            float x = std::sin(theta) * std::cos(phi);
            float y = std::cos(theta);
            float z = std::sin(theta) * std::sin(phi);

            vertices.insert(vertices.end(), {rad * x, rad * y, rad * z});
            normals.insert(normals.end(), {x, y, z});

            // vertex tex coord (s, t) range between [0, 1]
            float s = (float) v / vSegs;
            float t = (float) h / hSegs;
            texCoords.insert(texCoords.end(), {s, t});
        }
    }

    for (uint32_t v = 0; v < vSegs; v++)
    {
        for (uint32_t h = 0; h < hSegs; h++)
        {
            uint32_t topRight   = v * (hSegs + 1) + h;
            uint32_t topLeft    = v * (hSegs + 1) + h + 1;
            uint32_t lowerRight = (v + 1) * (hSegs + 1) + h;
            uint32_t lowerLeft  = (v + 1) * (hSegs + 1) + h + 1;

            std::vector<unsigned int> tri0;
            std::vector<unsigned int> tri1;

            tri0.push_back(lowerLeft);
            tri0.push_back(lowerRight);
            tri0.push_back(topRight);

            tri1.push_back(lowerLeft);
            tri1.push_back(topRight);
            tri1.push_back(topLeft);

            indices.insert(indices.end(), tri0.begin(), tri0.end());
            indices.insert(indices.end(), tri1.begin(), tri1.end());
        }
    }

    uploadToGPU(&ball, vertices, indices, normals, texCoords);
    return (ball);
}

shape CreateSphere(float radius, unsigned int stackCount, unsigned int sectorCount)
{
    shape sphere      = {};
    sphere.numIndices = (stackCount * sectorCount * 2) * 3;

    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;

    float x, y, z, xy;                          // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;// vertex normal
    float s, t;                                 // vertex texCoord

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep  = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep; // starting from pi/2 to -pi/2
        xy         = radius * cosf(stackAngle);// r * cos(u)
        z          = radius * sinf(stackAngle);// r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;// starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);// r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);// r * cos(u) * sin(v)
            vertices.insert(vertices.end(), {x, y, z});

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            normals.insert(normals.end(), {nx, ny, nz});

            // vertex tex coord (s, t) range between [0, 1]
            s = (float) j / sectorCount;
            t = (float) i / stackCount;
            texCoords.insert(texCoords.end(), {s, t});
        }
    }

    // generate CCW index list of sphere triangles
    // k1--k1+1
    // |  / |
    // | /  |
    // k2--k2+1
    std::vector<unsigned int> indices;
    std::vector<int>          lineIndices;
    int                       k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);// beginning of current stack
        k2 = k1 + sectorCount + 1; // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if (i != (stackCount - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }

            // store indices for lines
            // vertical lines for all stacks, k1 => k2
            lineIndices.push_back(k1);
            lineIndices.push_back(k2);
            if (i != 0)// horizontal lines except 1st stack, k1 => k+1
            {
                lineIndices.push_back(k1);
                lineIndices.push_back(k1 + 1);
            }
        }
    }

    uploadToGPU(&sphere, vertices, indices, normals);

    return sphere;
}

void RenderShape(shape *shape)
{
    glBindVertexArray(shape->VAO);
    glDrawElements(GL_TRIANGLES, shape->numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void RenderSide(shape *shape, unsigned int Side)
{
    glBindVertexArray(shape->VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *) (sizeof(unsigned int) * 6 * Side));
    glBindVertexArray(0);
}

shape CreateCubeTextured(r32 w, r32 h, r32 l)
{
    // initialize (if necessary)
    std::vector<r32> vertices = {
    // clang-format off
        -w, -h, -l,  // bottom-left
         w,  h, -l,  // top-right
         w, -h, -l,  // bottom-right
         w,  h, -l,  // top-right
        -w, -h, -l,  // bottom-left
        -w,  h, -l,  // top-left
        // front face
        -w, -h,  l,  // bottom-left
         w, -h,  l,  // bottom-right
         w,  h,  l,  // top-right
         w,  h,  l,  // top-right
        -w,  h,  l,  // top-left
        -w, -h,  l,  // bottom-left
        // left face
        -w,  h,  l,  // top-right
        -w,  h, -l,  // top-left
        -w, -h, -l,  // bottom-left
        -w, -h, -l,  // bottom-left
        -w, -h,  l,  // bottom-right
        -w,  h,  l,  // top-right
        // right face
         w,  h,  l,  // top-left
         w, -h, -l,  // bottom-right
         w,  h, -l,  // top-right
         w, -h, -l,  // bottom-right
         w,  h,  l,  // top-left
         w, -h,  l,  // bottom-left
        // bottom face
        -w, -h, -l,  // top-right
         w, -h, -l,  // top-left
         w, -h,  l,  // bottom-left
         w, -h,  l,  // bottom-left
        -w, -h,  l,  // bottom-right
        -w, -h, -l,  // top-right
        // top face
        -w,  h, -l,  // top-left
         w,  h , l,  // bottom-right
         w,  h, -l,  // top-right
         w,  h,  l,  // bottom-right
        -w,  h, -l,  // top-left
        -w,  h,  l,  // bottom-left

    };

    std::vector<r32> normals =
         {
         0.0f,  0.0f, -1.0f,
         0.0f,  0.0f, -1.0f,
         0.0f,  0.0f, -1.0f,
         0.0f,  0.0f, -1.0f,
         0.0f,  0.0f, -1.0f,
         0.0f,  0.0f, -1.0f,
         0.0f,  0.0f,  1.0f,
         0.0f,  0.0f,  1.0f,
         0.0f,  0.0f,  1.0f,
         0.0f,  0.0f,  1.0f,
         0.0f,  0.0f,  1.0f,
         0.0f,  0.0f,  1.0f,
        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,
         1.0f,  0.0f,  0.0f,
         1.0f,  0.0f,  0.0f,
         1.0f,  0.0f,  0.0f,
         1.0f,  0.0f,  0.0f,
         1.0f,  0.0f,  0.0f,
         1.0f,  0.0f,  0.0f,
         0.0f, -1.0f,  0.0f,
         0.0f, -1.0f,  0.0f,
         0.0f, -1.0f,  0.0f,
         0.0f, -1.0f,  0.0f,
         0.0f, -1.0f,  0.0f,
         0.0f, -1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,
         };

    std::vector<r32> texCoords =
         {
         0.0f, 0.0f,
         1.0f, 1.0f,
         1.0f, 0.0f,
         1.0f, 1.0f,
         0.0f, 0.0f,
         0.0f, 1.0f,
         0.0f, 0.0f,
         1.0f, 0.0f,
         1.0f, 1.0f,
         1.0f, 1.0f,
         0.0f, 1.0f,
         0.0f, 0.0f,
         1.0f, 0.0f,
         1.0f, 1.0f,
         0.0f, 1.0f,
         0.0f, 1.0f,
         0.0f, 0.0f,
         1.0f, 0.0f,
         1.0f, 0.0f,
         0.0f, 1.0f,
         1.0f, 1.0f,
         0.0f, 1.0f,
         1.0f, 0.0f,
         0.0f, 0.0f,
         0.0f, 1.0f,
         1.0f, 1.0f,
         1.0f, 0.0f,
         1.0f, 0.0f,
         0.0f, 0.0f,
         0.0f, 1.0f,
         0.0f, 1.0f,
         1.0f, 0.0f,
         1.0f, 1.0f,
         1.0f, 0.0f,
         0.0f, 1.0f,
         0.0f, 0.0f
         };
    std::vector<u32> indices =
    {
     0,  1,  2,
     3,  4,  5,
     6,  7,  8,
     9,  10, 11,
     12, 13, 14,
     15, 16, 17,
     18, 19, 20,
     21, 22, 23,
     24, 25, 26,
     27, 28, 29,
     30, 31, 32,
     33, 34, 35
    };
    // clang-format on

    shape box = {};

    box.numIndices = 36;

    uploadToGPU(&box, vertices, indices, normals, texCoords);
    return (box);
}

shape CreateQuad()
{
    // clang-format off
    std::vector<r32> Vertices = 
    {
    -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
    };

    std::vector<u32> Indices = 
    {
      1,2,0,
      2,3,0,
    };

    std::vector<r32> TextCoords =
    {
     0.0f, 1.0f,
     0.0f, 0.0f,
     1.0f, 0.0f,
     1.0f, 1.0f
    };

    std::vector<r32> Normals = 
    {
     0.0f,  1.0f, 0.0f,
     0.0f,  1.0f, 0.0f,
     0.0f,  1.0f, 0.0f,
     0.0f,  1.0f, 0.0f,
    };
    // clang-format on

    shape Quad = {};

    Quad.numIndices = 6;

    uploadToGPU(&Quad, Vertices, Indices, Normals, TextCoords);

    return Quad;
}