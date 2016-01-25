__kernel void t2main(__write_only image2d_t img)
{
    float4 color = (float4)(255, 255, 255, 255);

    for (int i = 0; i < 640; i++) {
        for (int j = 0; j < 480; j++) {
            write_imagef(img, (int2)(i, j), color);
        }
    }
}
