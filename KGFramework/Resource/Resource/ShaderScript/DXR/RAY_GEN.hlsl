RWTexture2D<float4> colorOutput : register(u0);

[shader("raygeneration")]
void RayGeneration()
{
    int2 launchIndex = DispatchRaysIndex();
    int2 dimensions = DispatchRaysDimensions();
    colorOutput[launchIndex] = float4(0.3, 0, 0, 1.0);
}