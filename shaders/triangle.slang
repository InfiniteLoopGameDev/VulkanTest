struct VertexInput {
    float2 position;
    float3 color;
}

[vk::binding(0, 0)]
StructuredBuffer<VertexInput, ScalarDataLayout> vertexIn;

struct VertexOutput {
    float4 position : SV_Position;
    float3 color;
};

[[vk::push_constant]]
uniform uint vertexCount;

struct MeshData {
    float3 averageColour;
};

[shader("task")]
[numthreads(1, 1, 1)]
void taskMain() {
    MeshData payload;

    for (int i = 0; i < vertexCount; i++) {
        payload.averageColour += vertexIn[i].color / vertexCount;
    }

    DispatchMesh(vertexCount, 1, 1, payload);
}

float4 calculatePoint(float n) {
    static const float2 up = { 0.0, -0.5 };

    float rotationAngle = 2 * float.getPi() * n / vertexCount;

    float2x2 rotation = { cos(rotationAngle), -sin(rotationAngle), sin(rotationAngle), cos(rotationAngle) };

    return { mul(rotation, up), 0.0, 1.0 };
}

[shader("mesh")]
[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void meshMain(in payload MeshData payload, in uint3 groupId: SV_GroupID, out indices uint3 triangles[1],
              out vertices VertexOutput vertices[3]) {
    SetMeshOutputCounts(3, 1);

    uint next = (groupId.x + 1) % vertexCount;

    vertices[0] = { calculatePoint(float(groupId.x)), vertexIn[groupId.x].color };
    vertices[1] = { calculatePoint(float(next)), vertexIn[next].color };
    vertices[2] = { { 0.0, 0.0, 0.0, 1.0 }, payload.averageColour };

    triangles[0] = uint3(0, 1, 2);
}
/*
[shader("vertex")]
VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    output.position = float4(input.position, 0.0, 1.0);
    output.color = input.color;

    return output;
}*/

[shader("pixel")] // Slang uses the "pixel" keyword for fragment shaders
float4 fragmentMain(VertexOutput input) : SV_Target { return float4(input.color, 1.0); }
