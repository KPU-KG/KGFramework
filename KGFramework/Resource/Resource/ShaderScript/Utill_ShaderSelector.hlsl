
#ifdef GEOMETRY_NORMAL
#include "Geometry_Default.hlsl"
#endif 

#ifdef GEOMETRY_CUBE

#endif

#ifdef GEOMETRY_CUBE_SHADOW
#include "Geometry_GSShadow.hlsl"
#endif

#ifdef GEOMETRY_CASCADE_SHADOW
#include "Geometry_CascadeShadow.hlsl"
#endif

#ifdef GEOMETRY_SKYBOX
#include "Geometry_SkyBox.hlsl"
#endif

#ifdef PIXEL_NORMAR_DEFERRED
#include "Pixel_Default.hlsl"
#endif
#ifdef PIXEL_GSCUBE_SHADOW
#include "Pixel_GSShadow.hlsl"
#endif
#ifdef PIXEL_GREEN_WIREFRAME
#include "Pixel_WireFrame.hlsl"
#endif
