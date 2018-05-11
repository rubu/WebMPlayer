#pragma once

#include "Ebml.h"

#include <SDL.h>

#include <atomic>

class DecodingThreadBase
{
public:
	DecodingThreadBase(EbmlElement& segment, const int video_track_number, unsigned int milliseconds_per_tick, bool verbose = false) : segment_(segment),
		video_track_number_(video_track_number),
		milliseconds_per_tick_(milliseconds_per_tick),
		verbose_(verbose)
	{
	}

	virtual void decode(SDL_Renderer* renderer, SDL_Texture* texture) = 0;

	void quit()
	{
		quit_ = true;
	}

protected:
	const EbmlElement& segment_;
	const int video_track_number_;
	const unsigned int milliseconds_per_tick_;
	const bool verbose_;
	std::atomic<bool> quit_;
};