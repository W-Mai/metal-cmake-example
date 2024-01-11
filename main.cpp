#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include <Metal/Metal.hpp>
#include <iostream>

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

    pool->release();

    print_vector(result, 10);

    return 0;
}
