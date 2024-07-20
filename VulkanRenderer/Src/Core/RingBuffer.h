#pragma once

#include <vector>
#include <string>
#include <memory>
#include <cassert>

class Context;
class Buffer;

class RingBuffer
{
public:
	RingBuffer();
	RingBuffer(uint32_t inRingSize, Context* inContext, size_t buffSize, const std::string& name = "Ring Buffer");

	void init(uint32_t inRingSize, Context* inContext, size_t buffSize, const std::string& name = "Ring Buffer");

	void advanceBuffer();

	const Buffer* getCurrentBuffer() const
	{
		return ringBuffer[ringIndex].get();
	}

	const std::shared_ptr<Buffer> getBuffer(const uint32_t index)
	{
		assert(index < ringBuffer.size(), "Index out of buffer range!");
		return ringBuffer[index];
	}

private:
	uint32_t ringSize;
	uint32_t ringIndex = 0;
	size_t bufferSize;

	std::string debugName = "";

	std::vector<std::shared_ptr<Buffer>> ringBuffer;
};

