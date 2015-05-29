#pragma once

#include "../Container/HashBase.h"
#include "../Math/StringHash.h"

namespace Clockwork
{

class Vector3;

/// Graphics capability support level. HTML5 (Emscripten) also uses OpenGL ES, but is considered a desktop platform capability-wise
#if defined(ANDROID) || defined(IOS) || defined(RPI)
#define MOBILE_GRAPHICS
#else
#define DESKTOP_GRAPHICS
#endif

/// Primitive type.
enum PrimitiveType
{
    TRIANGLE_LIST = 0,
    LINE_LIST,
    POINT_LIST,
    TRIANGLE_STRIP,
    LINE_STRIP,
    TRIANGLE_FAN
};

/// %Geometry type.
enum GeometryType
{
    GEOM_STATIC = 0,
    GEOM_SKINNED = 1,
    GEOM_INSTANCED = 2,
    GEOM_BILLBOARD = 3,
    GEOM_STATIC_NOINSTANCING = 4,
    MAX_GEOMETRYTYPES = 4,
};

/// Blending mode.
enum BlendMode
{
    BLEND_REPLACE = 0,
    BLEND_ADD,
    BLEND_MULTIPLY,
    BLEND_ALPHA,
    BLEND_ADDALPHA,
    BLEND_PREMULALPHA,
    BLEND_INVDESTALPHA,
    BLEND_SUBTRACT,
    BLEND_SUBTRACTALPHA,
    MAX_BLENDMODES
};

/// Depth or stencil compare mode.
enum CompareMode
{
    CMP_ALWAYS = 0,
    CMP_EQUAL,
    CMP_NOTEQUAL,
    CMP_LESS,
    CMP_LESSEQUAL,
    CMP_GREATER,
    CMP_GREATEREQUAL,
    MAX_COMPAREMODES
};

/// Culling mode.
enum CullMode
{
    CULL_NONE = 0,
    CULL_CCW,
    CULL_CW,
    MAX_CULLMODES
};

/// Fill mode.
enum FillMode
{
    FILL_SOLID = 0,
    FILL_WIREFRAME,
    FILL_POINT
};

/// Stencil operation.
enum StencilOp
{
    OP_KEEP = 0,
    OP_ZERO,
    OP_REF,
    OP_INCR,
    OP_DECR
};

/// Vertex/index buffer lock state.
enum LockState
{
    LOCK_NONE = 0,
    LOCK_HARDWARE,
    LOCK_SHADOW,
    LOCK_SCRATCH
};

/// Vertex elements.
enum VertexElement
{
    ELEMENT_POSITION = 0,
    ELEMENT_NORMAL,
    ELEMENT_COLOR,
    ELEMENT_TEXCOORD1,
    ELEMENT_TEXCOORD2,
    ELEMENT_CUBETEXCOORD1,
    ELEMENT_CUBETEXCOORD2,
    ELEMENT_TANGENT,
    ELEMENT_BLENDWEIGHTS,
    ELEMENT_BLENDINDICES,
    ELEMENT_INSTANCEMATRIX1,
    ELEMENT_INSTANCEMATRIX2,
    ELEMENT_INSTANCEMATRIX3,
    MAX_VERTEX_ELEMENTS
};

/// Texture filtering mode.
enum TextureFilterMode
{
    FILTER_NEAREST = 0,
    FILTER_BILINEAR,
    FILTER_TRILINEAR,
    FILTER_ANISOTROPIC,
    FILTER_DEFAULT,
    MAX_FILTERMODES
};

/// Texture addressing mode.
enum TextureAddressMode
{
    ADDRESS_WRAP = 0,
    ADDRESS_MIRROR,
    ADDRESS_CLAMP,
    ADDRESS_BORDER,
    MAX_ADDRESSMODES
};

/// Texture coordinates.
enum TextureCoordinate
{
    COORD_U = 0,
    COORD_V,
    COORD_W,
    MAX_COORDS
};

/// Texture usage types.
enum TextureUsage
{
    TEXTURE_STATIC = 0,
    TEXTURE_DYNAMIC,
    TEXTURE_RENDERTARGET,
    TEXTURE_DEPTHSTENCIL
};

/// Cube map faces.
enum CubeMapFace
{
    FACE_POSITIVE_X = 0,
    FACE_NEGATIVE_X,
    FACE_POSITIVE_Y,
    FACE_NEGATIVE_Y,
    FACE_POSITIVE_Z,
    FACE_NEGATIVE_Z,
    MAX_CUBEMAP_FACES
};

/// Cubemap single image layout modes.
enum CubeMapLayout
{
    CML_HORIZONTAL = 0,
    CML_HORIZONTALNVIDIA,
    CML_HORIZONTALCROSS,
    CML_VERTICALCROSS,
    CML_BLENDER
};

/// Update mode for render surface viewports.
enum RenderSurfaceUpdateMode
{
    SURFACE_MANUALUPDATE = 0,
    SURFACE_UPDATEVISIBLE,
    SURFACE_UPDATEALWAYS
};

/// Shader types.
enum ShaderType
{
    VS = 0,
    PS,
};

/// Shader parameter groups for determining need to update. On APIs that support constant buffers, these correspond to different constant buffers.
enum ShaderParameterGroup
{
    SP_FRAME = 0,
    SP_CAMERA,
    SP_ZONE,
    SP_LIGHT,
    SP_MATERIAL,
    SP_OBJECT,
    SP_CUSTOM,
    MAX_SHADER_PARAMETER_GROUPS
};

/// Texture units.
enum TextureUnit
{
    TU_DIFFUSE = 0,
    TU_ALBEDOBUFFER = 0,
    TU_ROUGHNESS = 1,
    TU_MATALIC = 2,
    TU_NORMAL = 10,
    TU_NORMALBUFFER = 3,
    TU_SPECULAR = 4,
    TU_EMISSIVE = 5,
    TU_ENVIRONMENT = 6,
#ifdef DESKTOP_GRAPHICS
    TU_VOLUMEMAP = 7,
    TU_CUSTOM1 = 8,
    TU_CUSTOM2 = 9,
    TU_LIGHTRAMP = 10,
    TU_LIGHTSHAPE = 11,
    TU_SHADOWMAP = 12,
    TU_FACESELECT = 13,
    TU_INDIRECTION = 14,
    TU_DEPTHBUFFER = 15,
    TU_LIGHTBUFFER = 16,
    TU_ZONE = 17,
    MAX_MATERIAL_TEXTURE_UNITS = 10,
    MAX_TEXTURE_UNITS = 18
#else
    TU_LIGHTRAMP = 7,
    TU_LIGHTSHAPE = 8,
    TU_SHADOWMAP = 9,
    MAX_MATERIAL_TEXTURE_UNITS = 7,
    MAX_TEXTURE_UNITS = 10
#endif
};

/// Billboard camera facing modes.
enum FaceCameraMode
{
    FC_NONE = 0,
    FC_ROTATE_XYZ,
    FC_ROTATE_Y,
    FC_LOOKAT_XYZ,
    FC_LOOKAT_Y
};

// Inbuilt shader parameters.
extern CLOCKWORK_API const StringHash VSP_AMBIENTSTARTCOLOR;
extern CLOCKWORK_API const StringHash VSP_AMBIENTENDCOLOR;
extern CLOCKWORK_API const StringHash VSP_BILLBOARDROT;
extern CLOCKWORK_API const StringHash VSP_CAMERAPOS;
extern CLOCKWORK_API const StringHash VSP_CAMERAROT;
extern CLOCKWORK_API const StringHash VSP_CLIPPLANE;
extern CLOCKWORK_API const StringHash VSP_NEARCLIP;
extern CLOCKWORK_API const StringHash VSP_FARCLIP;
extern CLOCKWORK_API const StringHash VSP_DEPTHMODE;
extern CLOCKWORK_API const StringHash VSP_DELTATIME;
extern CLOCKWORK_API const StringHash VSP_ELAPSEDTIME;
extern CLOCKWORK_API const StringHash VSP_FRUSTUMSIZE;
extern CLOCKWORK_API const StringHash VSP_GBUFFEROFFSETS;
extern CLOCKWORK_API const StringHash VSP_LIGHTDIR;
extern CLOCKWORK_API const StringHash VSP_LIGHTPOS;
extern CLOCKWORK_API const StringHash VSP_MODEL;
extern CLOCKWORK_API const StringHash VSP_VIEWPROJ;
extern CLOCKWORK_API const StringHash VSP_UOFFSET;
extern CLOCKWORK_API const StringHash VSP_VOFFSET;
extern CLOCKWORK_API const StringHash VSP_ZONE;
extern CLOCKWORK_API const StringHash VSP_LIGHTMATRICES;
extern CLOCKWORK_API const StringHash VSP_SKINMATRICES;
extern CLOCKWORK_API const StringHash VSP_VERTEXLIGHTS;
extern CLOCKWORK_API const StringHash PSP_AMBIENTCOLOR;
extern CLOCKWORK_API const StringHash PSP_CAMERAPOS;
extern CLOCKWORK_API const StringHash PSP_DELTATIME;
extern CLOCKWORK_API const StringHash PSP_DEPTHRECONSTRUCT;
extern CLOCKWORK_API const StringHash PSP_ELAPSEDTIME;
extern CLOCKWORK_API const StringHash PSP_FOGCOLOR;
extern CLOCKWORK_API const StringHash PSP_FOGPARAMS;
extern CLOCKWORK_API const StringHash PSP_GBUFFERINVSIZE;
extern CLOCKWORK_API const StringHash PSP_LIGHTCOLOR;
extern CLOCKWORK_API const StringHash PSP_LIGHTDIR;
extern CLOCKWORK_API const StringHash PSP_LIGHTPOS;
extern CLOCKWORK_API const StringHash PSP_MATDIFFCOLOR;
extern CLOCKWORK_API const StringHash PSP_MATEMISSIVECOLOR;
extern CLOCKWORK_API const StringHash PSP_MATENVMAPCOLOR;
extern CLOCKWORK_API const StringHash PSP_MATSPECCOLOR;
extern CLOCKWORK_API const StringHash PSP_NEARCLIP;
extern CLOCKWORK_API const StringHash PSP_FARCLIP;
extern CLOCKWORK_API const StringHash PSP_SHADOWCUBEADJUST;
extern CLOCKWORK_API const StringHash PSP_SHADOWDEPTHFADE;
extern CLOCKWORK_API const StringHash PSP_SHADOWINTENSITY;
extern CLOCKWORK_API const StringHash PSP_SHADOWMAPINVSIZE;
extern CLOCKWORK_API const StringHash PSP_SHADOWSPLITS;
extern CLOCKWORK_API const StringHash PSP_LIGHTMATRICES;

// Scale calculation from bounding box diagonal.
extern CLOCKWORK_API const Vector3 DOT_SCALE;

static const int QUALITY_LOW = 0;
static const int QUALITY_MEDIUM = 1;
static const int QUALITY_HIGH = 2;
static const int QUALITY_MAX = 15;

static const int SHADOWQUALITY_LOW_16BIT = 0;
static const int SHADOWQUALITY_LOW_24BIT = 1;
static const int SHADOWQUALITY_HIGH_16BIT = 2;
static const int SHADOWQUALITY_HIGH_24BIT = 3;

static const unsigned CLEAR_COLOR = 0x1;
static const unsigned CLEAR_DEPTH = 0x2;
static const unsigned CLEAR_STENCIL = 0x4;

static const unsigned MASK_NONE = 0x0;
static const unsigned MASK_POSITION = 0x1;
static const unsigned MASK_NORMAL = 0x2;
static const unsigned MASK_COLOR = 0x4;
static const unsigned MASK_TEXCOORD1 = 0x8;
static const unsigned MASK_TEXCOORD2 = 0x10;
static const unsigned MASK_CUBETEXCOORD1 = 0x20;
static const unsigned MASK_CUBETEXCOORD2 = 0x40;
static const unsigned MASK_TANGENT = 0x80;
static const unsigned MASK_BLENDWEIGHTS = 0x100;
static const unsigned MASK_BLENDINDICES = 0x200;
static const unsigned MASK_INSTANCEMATRIX1 = 0x400;
static const unsigned MASK_INSTANCEMATRIX2 = 0x800;
static const unsigned MASK_INSTANCEMATRIX3 = 0x1000;
static const unsigned MASK_DEFAULT = 0xffffffff;
static const unsigned NO_ELEMENT = 0xffffffff;

static const int MAX_RENDERTARGETS = 4;
static const int MAX_VERTEX_STREAMS = 4;
static const int MAX_CONSTANT_REGISTERS = 256;

static const int BITS_PER_COMPONENT = 8;
}