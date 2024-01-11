#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include <Metal/Metal.hpp>
#include <iostream>
#include "render_types.h"

MTL::ComputePipelineState *pipelineState;
MTL::CommandQueue *commandQueue;


void generate_vector(MTL::Buffer *vector, size_t length) {
    float *dataPtr = (float *) vector->contents();

    for (unsigned long int index = 0; index < length; index++) {
        dataPtr[index] = float(index);
    }
}

void print_vector(MTL::Buffer *vector, size_t length) {
    float *dataPtr = (float *) vector->contents();

    for (unsigned long int index = 0; index < length; index++) {
        std::cout << dataPtr[index] << std::endl;
    }
    std::cout << std::endl;
}

void init(MTL::Device *device) {
    NS::Error *error;

    auto libraryPath = NS::String::string("./metal_build/shaders.metallib", NS::ASCIIStringEncoding);
    auto library = device->newLibrary(libraryPath, &error);

    if (!library) {
        std::cerr << "Failed to find the metal library.\n";
        exit(-1);
    }

    auto kernelName = NS::String::string("add_vectors", NS::ASCIIStringEncoding);
    auto kernel = library->newFunction(kernelName);

    if (!kernel) {
        std::cerr << "Failed to find the compute function.\n";
    }

    pipelineState = device->newComputePipelineState(kernel, &error);

    if (!pipelineState) {
        std::cerr << "Failed to create the pipeline state object.\n";
        exit(-1);
    }

    commandQueue = device->newCommandQueue();

    if (!commandQueue) {
        std::cerr << "Failed to find command queue.\n";
        exit(-1);
    }
}

void compute(MTL::Buffer *vecA, MTL::Buffer *vecB, MTL::Buffer *result, size_t size) {
    MTL::CommandBuffer *commandBuffer = commandQueue->commandBuffer();
    assert(commandBuffer != nullptr);

    MTL::ComputeCommandEncoder *computeEncoder = commandBuffer->computeCommandEncoder();
    assert(computeEncoder != nullptr);

    computeEncoder->setComputePipelineState(pipelineState);
    computeEncoder->setBuffer(vecA, 0, 0);
    computeEncoder->setBuffer(vecB, 0, 1);
    computeEncoder->setBuffer(result, 0, 2);

    MTL::Size gridSize = MTL::Size(size, 1, 1);

    NS::UInteger threadGroupSize = pipelineState->maxTotalThreadsPerThreadgroup();
    if (threadGroupSize > size) {
        threadGroupSize = size;
    }
    MTL::Size threadgroupSize = MTL::Size(threadGroupSize, 1, 1);

    computeEncoder->dispatchThreads(gridSize, threadgroupSize);

    computeEncoder->endEncoding();
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();
}

// https://developer.apple.com/documentation/metal/using_a_render_pipeline_to_render_primitives?language=objc

void test_render_trigle(MTL::Device *device) {
    // render a triangle to a framebuffer
    auto vertexData = device->newBuffer(sizeof(float) * 9, MTL::ResourceStorageModeShared);
    auto *dataPtr = (float *) vertexData->contents();
    dataPtr[0] = -1.0f;
    dataPtr[1] = -1.0f;
    dataPtr[2] = 0.0f;
    dataPtr[3] = 1.0f;
    dataPtr[4] = -1.0f;
    dataPtr[5] = 0.0f;
    dataPtr[6] = 0.0f;
    dataPtr[7] = 1.0f;
    dataPtr[8] = 0.0f;

    // render a triangle to a texture
    auto textureDescriptor = MTL::TextureDescriptor::texture2DDescriptor(
            MTL::PixelFormatRGBA8Unorm,
            512, 512,
            false
            );
    auto texture = device->newTexture(textureDescriptor);
    auto textureView = texture->newTextureView(MTL::PixelFormatRGBA8Unorm);
    auto renderPassDescriptor2 = MTL::RenderPassDescriptor::alloc()->init();
    auto colorAttachment2 = renderPassDescriptor2->colorAttachments()->object(0);
    colorAttachment2->setClearColor(MTL::ClearColor(0.0, 0.5, 0.5, 1.0));
    colorAttachment2->setLoadAction(MTL::LoadActionClear);
    colorAttachment2->setStoreAction(MTL::StoreActionStore);
    colorAttachment2->setTexture(texture);
    auto commandBuffer2 = commandQueue->commandBuffer();
    auto commandEncoder2 = commandBuffer2->renderCommandEncoder(renderPassDescriptor2);
    commandEncoder2->setVertexBuffer(vertexData, 0, 0);
    commandEncoder2->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(3));
    commandEncoder2->endEncoding();
    commandBuffer2->commit();
    commandBuffer2->waitUntilCompleted();
}

int main(int argc, const char *argv[]) {
    NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();
    MTL::Device *device = MTL::CreateSystemDefaultDevice();

    init(device);

    auto size = 100000000;

    auto result = device->newBuffer(sizeof(float) * size, MTL::ResourceStorageModeShared);

    auto vecA = device->newBuffer(sizeof(float) * size, MTL::ResourceStorageModeShared);
    auto vecB = device->newBuffer(sizeof(float) * size, MTL::ResourceStorageModeShared);

    generate_vector(vecA, size);
    generate_vector(vecB, size);

    compute(vecA, vecB, result, size);

    test_render_trigle(device);

    pool->release();

    print_vector(result, 10);

    return 0;
}
