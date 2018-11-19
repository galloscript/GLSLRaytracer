//#version 430
//precision highp float;

out vec4 outColor;


//layout(binding = 0, rgba32f) uniform image2D uRaytracerOutputBuffer;


void main(void)
{
    ivec2 lOutputPosition = ivec2(gl_FragCoord.xy);
    vec4 lRaytracerOutput = imageLoad(uRaytracerOutputBuffer, lOutputPosition);
    vec4 lColor = vec4( sqrt(lRaytracerOutput.xyz / float(uFrameData.mSamplesCount)), 1.0);
    //vec4 lColor = vec4(float(uFrameData.mTime)*0.01, 0.0, 0.0, 1.0);
    outColor = lColor;
}
