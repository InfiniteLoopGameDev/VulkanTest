struct VertexOutput {
    float3 color : COLOR;
    float4 sv_position : SV_Position;
};

static float2 positions[3] = float2[](float2(0.0, -0.5), float2(0.5, 0.5), float2(-0.5, 0.5));

static float3 colors[3] =
    float3[](float3(1.0, 0.0, 0.0), float3(0.0, 1.0, 0.0), float3(0.0, 0.0, 1.0));

[shader("vertex")]
VertexOutput vertexMain(uint vertexID: SV_VertexID) {
    float4 position = float4(positions[vertexID], 0.0, 1.0);
    float3 color = colors[vertexID];

    VertexOutput output;
    output.sv_position = position;
    output.color = color;

    return output;
}

[shader("pixel")] // Slang uses the "pixel" keyword for fragment shaders
float4 fragmentMain(VertexOutput input) : SV_Target { return float4(input.color, 1.0); }
