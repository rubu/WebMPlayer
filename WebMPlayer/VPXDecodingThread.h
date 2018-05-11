#pragma once

#include "DecodingThread.h"

#include <vpx/vpx_decoder.h>
#include <vpx/vp8dx.h>

class VPXDecodingThread : public DecodingThreadBase
{
public:
	VPXDecodingThread(vpx_codec_ctx_t codec_context, EbmlElement& segment, const int video_track_number, unsigned int milliseconds_per_tick, bool verbose = false);

	void decode(SDL_Renderer* renderer, SDL_Texture* texture) override;

private:
	vpx_codec_ctx_t codec_context_;
};