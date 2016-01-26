#pragma OPENCL EXTENSION cl_khr_gl_event : enable

__kernel void t2main(__write_only image2d_t img, float amt)
{
    int i = get_global_id(0);
    int j = get_global_id(1);
    write_imagef(img, (int2)(i, j), (float4)(cos((float)i*amt), sin((float)i*amt), sin((float)j*amt), 1));
}
