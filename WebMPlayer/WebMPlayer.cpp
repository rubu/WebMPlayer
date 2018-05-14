#include "Ebml.h"
#include "VPXDecodingThread.h"
#include "AV1DecodingThread.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include <vpx/vpx_decoder.h>
#include <vpx/vp8dx.h>
#include <aom/aom.h>
#include <aom/aom_decoder.h>
#include <aom/aomdx.h>
#include <SDL.h>
#undef main

#include <memory>
#include <stack>
#include <tuple>
#include <list>
#include <sstream>
#include <ctime>
#include <algorithm>

#include <Winsock2.h>

#include <atlbase.h>
#include <altcom.h>
#include <Shobjidl.h>

struct FileDeleter
{
	void operator()(FILE* file)
	{
		fclose(file);
	}
};

void put_frame_callback(void *user_private, const vpx_image_t *img)
{
}

int main(int argc, char* argv[])
{
	try
	{
		std::string executable_name(argv[0]);
		auto last_path_separator = executable_name.find_last_of('\\');
		if (last_path_separator != std::string::npos)
		{
			executable_name = executable_name.substr(last_path_separator + 1);
		}
		std::string file_path;
		if (argc < 2)
		{
			CoInitialize(NULL);
			CComPtr<IFileDialog> pFileDialog;
			HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
			if (pFileDialog)
			{

			}
			std::cout << "Usage: " << executable_name << " <input file> [-v]" << std::endl;
			return EXIT_FAILURE;
		}
		bool verbose = false;
		if (argc == 3 && strcmp(argv[2], "-v") == 0)
		{
			verbose = true;
		}
		std::unique_ptr<FILE, FileDeleter> webm_file(_fsopen(argv[1], "rb", _SH_DENYWR));
		if (webm_file == nullptr)
		{
			throw std::runtime_error(std::string("could not open file ").append(argv[1]));
		}
		fseek(webm_file.get(), 0L, SEEK_END);
		size_t file_size = ftell(webm_file.get()), remaining_file_size = file_size;
		fseek(webm_file.get(), 0L, SEEK_SET);
		std::cout << "file: " << argv[1] << std::endl;
		std::cout << "size: " << file_size << std::endl;
		std::unique_ptr<unsigned char[]> webm_file_contents(new unsigned char[file_size]);
		if (fread(webm_file_contents.get(), 1, file_size, webm_file.get()) != file_size)
		{
			throw std::runtime_error(std::string("could not read ").append(std::to_string(file_size)).append(" bytes from file ").append(argv[1]));
		}
		EbmlParserState ebml_parser_state = EbmlParserState::ParseElementId;
		std::stack<EbmlElement> ebml_element_stack;
		std::stack<__int64> ebml_element_size_stack;
		std::list<EbmlElement> ebml_element_tree;
		EbmlElementId current_ebml_element_id = EbmlElementId::Ebml;
		EbmlElementType current_ebml_element_type = EbmlElementType::Unknown;
		__int64 current_ebml_element_size = 0;
		ebml_element_size_stack.push(file_size);
		size_t current_ebml_element_id_length = 0, current_ebml_element_size_length  = 0;
		try
		{
			while (remaining_file_size > 0)
			{
				auto file_offset = file_size - remaining_file_size;
				switch (ebml_parser_state)
				{
				case EbmlParserState::ParseElementId:
				{
					current_ebml_element_id = read_ebml_element_id(&webm_file_contents[file_offset], remaining_file_size, current_ebml_element_id_length);
					if (ebml_element_size_stack.top() != -1)
					{
						ebml_element_size_stack.top() -= current_ebml_element_id_length;
					}
					else if (ebml_element_size_stack.top() == -1 && get_ebml_element_level(current_ebml_element_id) == get_ebml_element_level(ebml_element_stack.top().id()))
					{
						auto element = ebml_element_stack.top();
						ebml_element_stack.pop();
						element.calculate_size();
						ebml_element_size_stack.pop();
						if (ebml_element_size_stack.top() != -1)
						{
							ebml_element_size_stack.top() -= element.size();
						}
						ebml_element_stack.top().add_child(element);
					}
					ebml_parser_state = EbmlParserState::ParseElementLength;
				}
				break;
				case EbmlParserState::ParseElementLength:
					current_ebml_element_size = get_ebml_element_size(&webm_file_contents[file_offset], remaining_file_size, current_ebml_element_size_length);
					remaining_file_size -= current_ebml_element_size_length;
					if (ebml_element_size_stack.top() != -1)
					{
						ebml_element_size_stack.top() -= current_ebml_element_size_length;
					}
					ebml_parser_state = EbmlParserState::ParseElementValue;
					break;
				case EbmlParserState::ParseElementValue:
					current_ebml_element_type = get_ebml_element_type(current_ebml_element_id);
					if (current_ebml_element_type == EbmlElementType::Master)
					{
						ebml_element_stack.push(EbmlElement(current_ebml_element_id, current_ebml_element_type, current_ebml_element_size, current_ebml_element_id_length, current_ebml_element_size_length));
						ebml_element_size_stack.push(current_ebml_element_size);
					}
					else
					{
						remaining_file_size -= static_cast<size_t>(current_ebml_element_size);
						if (ebml_element_size_stack.top() != -1)
						{
							ebml_element_size_stack.top() -= static_cast<size_t>(current_ebml_element_size);
						}
						if (ebml_element_stack.size() > 0)
						{
							ebml_element_stack.top().add_child(EbmlElement(current_ebml_element_id, current_ebml_element_type, current_ebml_element_size, current_ebml_element_id_length, current_ebml_element_size_length, &webm_file_contents[file_offset]));
							while (ebml_element_size_stack.top() == 0)
							{
								auto parent = ebml_element_stack.top();
								ebml_element_stack.pop();
								ebml_element_size_stack.pop();
								if (parent.size() == -1)
								{
									parent.calculate_size();
								}
								if (ebml_element_size_stack.top() != -1)
								{
									ebml_element_size_stack.top() -= parent.size();
								}
								if (ebml_element_stack.size() == 0)
								{
									ebml_element_tree.push_back(parent);
									break;
								}
								else
								{
									ebml_element_stack.top().add_child(parent);
								}
							}
						}
						else
						{
							ebml_element_tree.push_back(EbmlElement(current_ebml_element_id, current_ebml_element_type, current_ebml_element_size, current_ebml_element_id_length, current_ebml_element_size_length, &webm_file_contents[file_offset]));
						}
					}
					ebml_parser_state = EbmlParserState::ParseElementId;
					break;
				default:
					throw std::runtime_error("parser state error");
				}
			}
		}
		catch (std::exception& exception)
		{
			std::cout << "parsing error at offset 0x" << std::hex << file_size - remaining_file_size << ": " << exception.what() << std::endl;
		}
		while (ebml_element_stack.size() > 0)
		{
			EbmlElement parent = ebml_element_stack.top();
			ebml_element_stack.pop();
			if (ebml_element_stack.size() == 0)
			{
				ebml_element_tree.push_back(parent);
			}
			else
			{
				ebml_element_stack.top().add_child(parent);
			}
		}
		if (verbose)
		{
			std::cout << std::endl << "*** EBML element tree ***" << std::endl;
			for (EbmlElement& ebml_element : ebml_element_tree)
			{
				ebml_element.print(0);
			}
			std::cout << std::endl;
		}
		auto segment = std::find_if(ebml_element_tree.begin(), ebml_element_tree.end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::Segment; });
		if (segment == ebml_element_tree.end())
		{
			throw std::runtime_error("no Segment element present");
		}
		auto info = std::find_if(segment->children().begin(), segment->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::Info; });
		if (info == segment->children().end())
		{
			throw std::runtime_error("no Info element present");
		}
		auto timecode_scale = std::find_if(info->children().begin(), info->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::TimecodeScale; });
		if (timecode_scale == info->children().end())
		{
			throw std::runtime_error("Info does not contain a TimecodeScale element");
		}
		unsigned int milliseconds_per_tick = std::stoi(timecode_scale->value()) / 1000000;
		auto Tracks = std::find_if(segment->children().begin(), segment->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::Tracks; });
		if (Tracks == segment->children().end())
		{
			throw std::runtime_error("no Tracks element present");
		}
		vpx_codec_iface_t* codec_interface = nullptr;
		vpx_codec_ctx_t codec_context;
		vpx_codec_dec_cfg_t decoder_configuration;
		vpx_codec_err_t vpx_error = VPX_CODEC_OK;
		vpx_codec_caps_t codec_capabilities = 0;
		aom_codec_ctx_t aom_codec_context;
		unsigned int video_track_number = 0;
		bool frame_callback_enabled = false;
		auto TrackEntry = std::find_if(Tracks->children().begin(), Tracks->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::TrackEntry; });
		std::unique_ptr<DecodingThreadBase> decoding_thread;
		while (TrackEntry != Tracks->children().end())
		{
			auto TrackNumber = std::find_if(TrackEntry->children().begin(), TrackEntry->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::TrackNumber; });
			if (TrackNumber == TrackEntry->children().end())
			{
				throw std::runtime_error("TrackEntry does not contain a TrackNumber element");
			}
			auto CodecId = std::find_if(TrackEntry->children().begin(), TrackEntry->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::CodecID; });
			if (CodecId != TrackEntry->children().end())
			{
				auto sCodec = CodecId->value();
				if (sCodec == "V_VP8")
				{
					codec_interface = vpx_codec_vp8_dx();
					codec_capabilities = vpx_codec_get_caps(codec_interface);
				}
				else if (sCodec == "V_VP9")
				{
					codec_interface = vpx_codec_vp9_dx();
					codec_capabilities = vpx_codec_get_caps(codec_interface);
				}
				else if (sCodec == "V_AV1")
				{
					video_track_number = std::stoi(TrackNumber->value());
					auto Video = std::find_if(TrackEntry->children().begin(), TrackEntry->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::Video; });
					aom_codec_iface_t* aom_codec_interface = aom_codec_av1_dx();
					aom_codec_caps_t aom_codec_capabilities = aom_codec_get_caps(aom_codec_interface);
					aom_codec_dec_cfg_t aom_codec_configuration;
					if (Video == (TrackEntry->children().end()))
					{
						throw std::runtime_error("TrackEntry with codec " + sCodec + " does not contain a Video element");
					}
					auto PixelWidth = std::find_if(Video->children().begin(), Video->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::PixelWidth; });
					if (PixelWidth == (Video->children().end()))
					{
						throw std::runtime_error("Video element does not contain a PixelWidth child element");
					}
					auto PixelHeight = std::find_if(Video->children().begin(), Video->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::PixelHeight; });
					if (PixelHeight == (Video->children().end()))
					{
						throw std::runtime_error("Video element does not contain a PixelHeight child element");
					}
					decoder_configuration.w = std::stoi(PixelWidth->value());
					decoder_configuration.h = std::stoi(PixelHeight->value());;
					decoder_configuration.threads = 1;
					auto aom_error = aom_codec_dec_init(&aom_codec_context, aom_codec_interface, &aom_codec_configuration, 0);
					if (aom_error != AOM_CODEC_OK)
					{
						throw std::runtime_error(std::string("aom_codec_dec_init() failed with ").append(std::to_string(aom_error)));
					}
					decoding_thread.reset(new AV1DecodingThread(aom_codec_context, *segment, video_track_number, milliseconds_per_tick, verbose));
				}
				if (codec_interface != nullptr)
				{
					video_track_number = std::stoi(TrackNumber->value());
					auto Video = std::find_if(TrackEntry->children().begin(), TrackEntry->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::Video; });
					if (Video == (TrackEntry->children().end()))
					{
						throw std::runtime_error("TrackEntry with codec " + sCodec + " does not contain a Video element");
					}
					auto PixelWidth = std::find_if(Video->children().begin(), Video->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::PixelWidth; });
					if (PixelWidth == (Video->children().end()))
					{
						throw std::runtime_error("Video element does not contain a PixelWidth child element");
					}
					auto PixelHeight = std::find_if(Video->children().begin(), Video->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::PixelHeight; });
					if (PixelHeight == (Video->children().end()))
					{
						throw std::runtime_error("Video element does not contain a PixelHeight child element");
					}
					decoder_configuration.w = std::stoi(PixelWidth->value());
					decoder_configuration.h = std::stoi(PixelHeight->value());
					decoder_configuration.threads = 1;
					vpx_error = vpx_codec_dec_init(&codec_context, codec_interface, &decoder_configuration, 0);
					if (vpx_error != VPX_CODEC_OK)
					{
						throw std::runtime_error(std::string("vpx_codec_dec_init() failed with ").append(std::to_string(vpx_error)));
					}
					if (codec_capabilities & VPX_CODEC_CAP_PUT_FRAME)
					{
						vpx_error = vpx_codec_register_put_frame_cb(&codec_context, &put_frame_callback, nullptr);
						if (vpx_error != VPX_CODEC_OK)
						{
							throw std::runtime_error(std::string("vpx_codec_register_put_frame_cb() failed with ").append(std::to_string(vpx_error)));
						}
						frame_callback_enabled = true;
					}
					decoding_thread.reset(new VPXDecodingThread(codec_context, *segment, video_track_number, milliseconds_per_tick, verbose));
					break;
				}
			}
			TrackEntry = std::find_if(++TrackEntry, Tracks->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::TrackEntry; });
		}
		int sdl_error = SDL_Init(SDL_INIT_VIDEO);
		if (sdl_error)
		{
			throw std::runtime_error(std::string("SDL_Init() failed with ").append(std::to_string(sdl_error)));
		}
		SDL_Window* window = nullptr;
		SDL_Renderer* renderer = nullptr;
		sdl_error = SDL_CreateWindowAndRenderer(decoder_configuration.w, decoder_configuration.h, 0, &window, &renderer);
		if (sdl_error)
		{
			throw std::runtime_error(std::string("SDL_CreateWindowAndRenderer() failed with ").append(std::to_string(sdl_error)));
		}
		SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, decoder_configuration.w, decoder_configuration.h);
		auto decoding_thread_instance = decoding_thread.get();
		std::thread thread([decoding_thread_instance, renderer, texture]()
		{
			try
			{
				decoding_thread_instance->decode(renderer, texture);
			}
			catch (const std::exception& exception)
			{
				std::cout << "exception: " << exception.what();
			}
		});
		SDL_Event event;
		bool quit = false;
		while (quit == false)
		{
			/* Check for new events */
			while (SDL_PollEvent(&event))
			{
				/* If a quit event has been sent */
				if (event.type == SDL_QUIT)
				{
					quit = true;
					break;
				}
			}
		}
		decoding_thread_instance->quit();
		thread.join();
	}
	catch (std::exception& exception)
	{
		std::cout << "exception: " << exception.what();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}