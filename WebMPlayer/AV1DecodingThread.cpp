#include "AV1DecodingThread.h"
#include "AV1.h"
#include "Ebml.h"

#include <algorithm>
#include <iostream>
#include <string>

#include <Winsock2.h>

// #define PARSE_AV1_BLOCKS

AV1DecodingThread::AV1DecodingThread(aom_codec_ctx_t codec_context, EbmlElement& segment, const int video_track_number, unsigned int milliseconds_per_tick, bool verbose) : DecodingThreadBase(segment, video_track_number, milliseconds_per_tick, verbose),
	codec_context_(codec_context)
{
}

void AV1DecodingThread::decode(SDL_Renderer* renderer, SDL_Texture* texture)
{
	auto start = timeGetTime();
	unsigned int cluster_pts = 0;
	auto Cluster = std::find_if(segment_.children().begin(), segment_.children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::Cluster; });
	char timestamp[13] = { 0 };
	while (Cluster != segment_.children().end())
	{
		auto Timecode = std::find_if(Cluster->children().begin(), Cluster->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::Timecode; });
		if (Timecode == (Cluster->children().end()))
		{
			throw std::runtime_error("Cluster element does not contain a Timecode child element");
		}
		cluster_pts = std::stoi(Timecode->value());
		auto SimpleBlock = std::find_if(Cluster->children().begin(), Cluster->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::SimpleBlock; });
		while (SimpleBlock != Cluster->children().end())
		{
			auto* data = SimpleBlock->data();
			auto size = SimpleBlock->size();
			size_t track_number_size_length;
			auto track_number = get_ebml_element_size(data, static_cast<size_t>(size), track_number_size_length);
			unsigned char flags = *(data + track_number_size_length + 2);
			if (track_number == video_track_number_)
			{
				unsigned char flags = *(data + track_number_size_length + 2);
				if ((flags & 0x6e) != 0)
				{
					throw std::runtime_error("lacing is not supported");
				}
				auto pts = cluster_pts + ntohs(*reinterpret_cast<const short*>(data + track_number_size_length));
				if (verbose_)
				{
					auto pts_in_milliseconds = pts * milliseconds_per_tick_;
					auto milliseconds = pts_in_milliseconds % 1000;
					pts_in_milliseconds -= milliseconds;
					auto seconds = (pts % 60000) / 1000;
					pts_in_milliseconds -= seconds * 1000;
					auto minutes = (pts % 3600000) / 60000;
					pts_in_milliseconds -= minutes * 3600000;
					auto hours = pts_in_milliseconds / 3600000;
					sprintf(timestamp, "%02u:%02u:%02u.%03u", hours, minutes, seconds, milliseconds);
					std::cout << "frame @ " << timestamp << ", size - " << size - track_number_size_length - 3 << std::endl;
				}
#if defined(PARSE_AV1_BLOCKS)
				{
					const auto av1_bitstream = data + track_number_size_length + 3;
					const auto av1_bitsream_length = size - track_number_size_length - 3;
					// https://aomediacodec.github.io/av1-isobmff/ 2.5 states the the sample entry should correspond to the open_bistream_unit syntax
					// open_bitstream_unit(av1_bitstream, av1_bitsream_length);
				}
#endif
				auto aom_error = aom_codec_decode(&codec_context_, data + track_number_size_length + 3, size - track_number_size_length - 3, nullptr);
				if (aom_error != AOM_CODEC_OK)
				{
					throw std::runtime_error(std::string("aom_codec_decode() failed with").append(std::to_string(aom_error)));
				}
				if (pts > 0)
				{
					auto end = timeGetTime(), difference = end - start;
					if (difference < pts * milliseconds_per_tick_)
					{
#if defined(LOG_DECODING_PERFORMANCE)
						std::cout << "decoder finished before presentation time, sleeping for " << (pts * milliseconds_per_tick_ - difference) << " milliseconds" << std::endl;
#endif
						::Sleep((pts * milliseconds_per_tick_) - difference);
					}
#if defined(LOG_DECODING_PERFORMANCE)
					else
					{
						std::cout << "decoder is late by " << (difference - pts * milliseconds_per_tick_) << " milliseconds" << std::endl;
					}
#endif
				}
				aom_codec_iter_t iterator = nullptr;
				aom_image* image = nullptr;
				while ((image = aom_codec_get_frame(&codec_context_, &iterator)) != nullptr)
				{
					auto sdl_error = SDL_UpdateYUVTexture(texture, nullptr, image->planes[0], image->stride[0], image->planes[1], image->stride[1], image->planes[2], image->stride[2]);
					if (sdl_error)
					{
						throw std::runtime_error(std::string("SDL_UpdateTexture() failed with ").append(std::to_string(sdl_error)));
					}
					sdl_error = SDL_RenderCopy(renderer, texture, nullptr, nullptr);
					if (sdl_error)
					{
						throw std::runtime_error(std::string("SDL_RenderCopy() failed with ").append(std::to_string(sdl_error)));
					}
					SDL_RenderPresent(renderer);
				}
				if (quit_)
				{
					return;
				}
			}
			SimpleBlock = std::find_if(++SimpleBlock, Cluster->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::SimpleBlock; });
		}
		Cluster = std::find_if(++Cluster, segment_.children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::Cluster; });
	}
}
