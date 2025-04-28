struct VertexOutput {
    float3 color : COLOR;
    float4 sv_position : SV_Position;
};

static const float2 positions[3] = float2[](float2(0.0, -0.5), float2(0.5, 0.5), float2(-0.5, 0.5));

static const float3 colors[3] =
    float3[](float3(1.0, 0.0, 0.0), float3(0.0, 1.0, 0.0), float3(0.0, 0.0, 1.0));

float3 linearToSRGB(float3 color) {
    float3x3 BT2020toSRGB = float3x3(1.6605, -0.5876, -0.0728, //
                                     -0.1246, 1.1329, -0.0083, //
                                     -0.0182, -0.1006, 1.1187);

    var srgbColor = clamp(mul(BT2020toSRGB, color), 0.0, 1.0);

    return max(1.055 * pow(srgbColor, 0.416666667) - 0.055, 0);
}

float3 linearToHDR10(float3 color) {
    var m1 = float3(1305.0 / 8192.0);
    var m2 = float3(2523.0 / 32.0);
    var c1 = float3(107.0 / 128.0);
    var c2 = float3(2413.0 / 128.0);
    var c3 = float3(2392.0 / 128.0);
    var y = color / 80.0;
    var ypow = pow(y, m1);
    var numerator = c1 + (c2 * ypow);
    var denominator = 1.0 + (c3 * ypow);
    var fraction = numerator / denominator;
    return pow(fraction, m2);
}

[shader("vertex")]
VertexOutput vertexMain(uint vertexID: SV_VertexID) {
    float4 position = float4(positions[vertexID], 0.0, 1.0);
    float3 color = colors[vertexID];

    return VertexOutput(color, position);
}

[shader("pixel")] // Slang uses the "pixel" keyword for fragment shaders
float4 fragmentMain(float3 color: COLOR) : SV_Target { return float4(linearToSRGB(color), 1.0); }
