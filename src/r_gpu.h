//
// Draw call packers for tinyGPU
//
// @sylefeb
// MIT license

// -----------------------------------------------------
// Column API
// -----------------------------------------------------

#define COLDRAW_COL(idx,start,end,light) (((idx&511)<<1) | ((start&255)<<10) | ((end&255)<<18) | ((light&15)<<26))

#define WALL        (0<<30)
#define PLANE       (1<<30)
#define TERRAIN     (2<<30)
#define PARAMETER   (3<<30)

#define COLDRAW_WALL(y,v_init,u_init) ((  y) & 65535) | ((((v_init))&255)<<16) | (((u_init)&255) << 24)
#define COLDRAW_TERRAIN(st,ed,pick)   (( ed) & 65535) | (((st) & 65535)<<16) | (pick)
#define COLDRAW_PLANE_B(ded,dr)       ((ded) & 65535) | (((dr) & 65535) << 16)

#define PICK        (1<<31)
#define COLDRAW_EOC (1)

// parameter: ray cs (terrain)
#define PARAMETER_RAY_CS(cs,ss)      (0<<30) | (( cs) & 16383) | ((   ss & 16383 )<<14)
// parameter: plane
#define PARAMETER_PLANE_A(ny,uy,vy)  (2<<30) | ((ny) & 1023)  | (((uy) & 1023)<<10) | (((vy) & 1023)<<20)
#define PARAMETER_PLANE_DTA(du,dv)   (((du) & 16383)<<1) | (((dv) & 16383)<<15)
// parameter: uv offset
#define PARAMETER_UV_OFFSET(uo,vo)   (1<<30) | ((uo) & 16383) | (((vo) & 16383)<<14)

#define MAX_DEPTH 65535
