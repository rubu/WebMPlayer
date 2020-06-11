#pragma once

#if defined(WITH_AV1)
#include "DecodingThread.h"

#include <aom/aom.h>
#include <aom/aom_decoder.h>

class AV1DecodingThread : public DecodingThreadBase
{
public:
	AV1DecodingThread(aom_codec_ctx_t codec_context, EbmlElement& segment, const int video_track_number, unsigned int milliseconds_per_tick, bool verbose = false);

	void decode(SDL_Renderer* renderer, SDL_Texture* texture) override;

private:
	aom_codec_ctx_t codec_context_;
};
#endif