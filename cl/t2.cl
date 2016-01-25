__kernel void t2main(__write_only image2d_t img)
{
    for (int i = 0; i < 640; i++) {
        for (int j = 0; j < 480; j++) {
            write_imagef(img, (int2)(i, j), (float4)(i/2, j/2, 0, 255));
        }
    }
}
