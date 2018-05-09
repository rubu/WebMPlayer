#include <cstdlib>
#include <iostream>
#include <string>

#include <vpx/vpx_decoder.h>
#include <vpx/vp8dx.h>

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

struct FileDeleter
{
	void operator()(FILE* file)
	{
		fclose(file);
	}
};


enum class EbmlParserState
{
	ParseElementId,
	ParseElementLength,
	ParseElementValue,
};

size_t get_ebml_element_id_length(unsigned char first_byte_of_element_id)
{
	for (unsigned char position = 7; position > 3; position--)
	{
		if (first_byte_of_element_id & 1 << position)
		{
			return 8 - position;
		}
	}
	throw std::runtime_error("invalid element id");
}

enum class EbmlElementId
{
	Ebml = 0x1a45dfa3,
	EbmlVersion = 0x4286,
	EbmlReadVersion = 0x42f7,
	EbmlMaxIdLength = 0x42f2,
	EbmlMaxSizeLength = 0x42f3,
	DocType = 0x4282,
	DocTypeVersion = 0x4287,
	DocTypeReadVersion = 0x4285,
	Void = 0xec,
	Segment = 0x18538067,
	SeekHead = 0x114d9b74,
	Seek = 0x4dbb,
	SeekID = 0x53ab,
	SeekPosition = 0x53ac,
	Info = 0x1549a966,
	TimecodeScale = 0x2ad7b1,
	Duration = 0x4489,
	DateUTC = 0x4461,
	Title = 0x7ba9,
	MuxingApp = 0x4d80,
	WritingApp = 0x5741,
	Cluster = 0x1f43b675,
	Timecode = 0xe7,
	SimpleBlock = 0xa3,
	SegmentUID = 0x73a4,
	Tracks = 0x1654ae6b,
	TrackEntry = 0xae,
	TrackNumber = 0xd7,
	TrackUID = 0x73c5,
	TrackType = 0x83,
	FlagEnabled = 0xb9,
	FlagDefault = 0x88,
	FlagForced = 0x55aa,
	FlagLacing = 0x9c,
	MinCache = 0x6de7,
	MaxBlockAdditionID = 0x55ee,
	CodecID = 0x86,
	CodecDecodeAll = 0xaa,
	SeekPreRoll = 0x56bb,
	Video = 0xe0,
	FlagInterlaced = 0x9a,
	PixelWidth = 0xb0,
	PixelHeight = 0xba,
	DefaultDuration = 0x23e383,
	TrackTimecodeScale = 0x23314f,
	CodecName = 0x258688,
	CodecPrivate = 0x63a2,
	Audio = 0xe1,
	SamplingFrequency = 0xb5,
	Channels = 0x9f,
	Cues = 0x1c53bb6b,
	CuePoint = 0xbb,
	CueTime = 0xb3,
	CueTrackPositions = 0xb7,
	CueTrack = 0xf7,
	CueClusterPosition = 0xf1,
	CueRefTime = 0x96,
	CueBlockNumber = 0x5378,
	Position = 0xa7,
}; 

std::string get_ebml_element_name(EbmlElementId ebml_element_id)
{
	switch (ebml_element_id)
	{
	case EbmlElementId::Ebml:
		return "EBML";
	case EbmlElementId::EbmlVersion:
		return "EBMLVersion";
	case EbmlElementId::EbmlReadVersion:
		return "EBMLReadVersion";
	case EbmlElementId::EbmlMaxIdLength:
		return "EBMLMaxIDLength";
	case EbmlElementId::EbmlMaxSizeLength:
		return "EBMLMaxSizeLength";
	case EbmlElementId::DocType:
		return "DocType";
	case EbmlElementId::DocTypeVersion:
		return "DocTypeVersion";
	case EbmlElementId::DocTypeReadVersion:
		return "DocTypeReadVersion";
	case EbmlElementId::Void:
		return "Void";
	case EbmlElementId::Segment:
		return "Segment";
	case EbmlElementId::SeekHead:
		return "SeekHead";
	case EbmlElementId::Seek:
		return "Seek";
	case EbmlElementId::SeekID:
		return "SeekID";
	case EbmlElementId::SeekPosition:
		return "SeekPosition";
	case EbmlElementId::Info:
		return "Info";
	case EbmlElementId::TimecodeScale:
		return "TimecodeScale";
	case EbmlElementId::Duration:
		return "Duration";
	case EbmlElementId::DateUTC:
		return "DateUTC";
	case EbmlElementId::Title:
		return "Title";
	case EbmlElementId::MuxingApp:
		return "MuxingApp";
	case EbmlElementId::WritingApp:
		return "WritinApp";
	case EbmlElementId::Cluster:
		return "Cluster";
	case EbmlElementId::Timecode:
		return "Timecode";
	case EbmlElementId::SimpleBlock:
		return "SimpleBlock";
	case EbmlElementId::SegmentUID:
		return "SegmentUID (unsupported by WebM)";
	case EbmlElementId::Tracks:
		return "Tracks";
	case EbmlElementId::TrackEntry:
		return "TrackEntry";
	case EbmlElementId::TrackNumber:
		return "TrackNumber";
	case EbmlElementId::TrackUID:
		return "TrackUID";
	case EbmlElementId::TrackType:
		return "TrackType";
	case EbmlElementId::FlagEnabled:
		return "FlagEnabled";
	case EbmlElementId::FlagDefault:
		return "FlagDefault";
	case EbmlElementId::FlagForced:
		return "FlagForced";
	case EbmlElementId::FlagLacing:
		return "FlagLacing";
	case EbmlElementId::MinCache:
		return "MinCache";
	case EbmlElementId::MaxBlockAdditionID:
		return "MaxBlockAdditionID";
	case EbmlElementId::CodecID:
		return "CodecID";
	case EbmlElementId::CodecDecodeAll:
		return "CodecDecodeAll";
	case EbmlElementId::SeekPreRoll:
		return "SeekPreRoll";
	case EbmlElementId::Video:
		return "Video";
	case EbmlElementId::FlagInterlaced:
		return "FlagInterlaced";
	case EbmlElementId::PixelWidth:
		return "PixelWidth";
	case EbmlElementId::PixelHeight:
		return "PixelHeight";
	case EbmlElementId::DefaultDuration:
		return "DefaultDuration";
	case EbmlElementId::TrackTimecodeScale:
		return "TrackTimecodeScale";
	case EbmlElementId::CodecName:
		return "CodecName";
	case EbmlElementId::CodecPrivate:
		return "CodecPrivate";
	case EbmlElementId::Audio:
		return "Audio";
	case EbmlElementId::SamplingFrequency:
		return "SamplingFrequency";
	case EbmlElementId::Channels:
		return "Channels";
	case EbmlElementId::Cues:
		return "Cues";
	case EbmlElementId::CuePoint:
		return "CuePoint";
	case EbmlElementId::CueTime:
		return "CueTime";
	case EbmlElementId::CueTrackPositions:
		return "CueTrackPositions";
	case EbmlElementId::CueTrack:
		return "CueTrack";
	case EbmlElementId::CueClusterPosition:
		return "CueClusterPosition";
	case EbmlElementId::CueRefTime:
		return "CueRefTime";
	case EbmlElementId::CueBlockNumber:
		return "CueBlockNumber";
	case EbmlElementId::Position:
		return "Position";
	default:
		return "<unknown element>";
	}
}

enum class EbmlElementType
{
	Unknown,
	Master,
	UnsignedInteger,
	SignedInteger,
	String,
	Utf8String,
	Binary,
	Float,
	Date
};

EbmlElementType get_ebml_element_type(EbmlElementId ebml_element_id)
{
	switch (ebml_element_id)
	{
	case EbmlElementId::Segment:
	case EbmlElementId::Ebml:
	case EbmlElementId::SeekHead:
	case EbmlElementId::Seek:
	case EbmlElementId::Cluster:
	case EbmlElementId::Info:
	case EbmlElementId::Tracks:
	case EbmlElementId::TrackEntry:
	case EbmlElementId::Video:
	case EbmlElementId::Audio:
	case EbmlElementId::Cues:
	case EbmlElementId::CuePoint:
	case EbmlElementId::CueTrackPositions:
		return EbmlElementType::Master;
	case EbmlElementId::EbmlVersion:
	case EbmlElementId::EbmlReadVersion:
	case EbmlElementId::EbmlMaxIdLength:
	case EbmlElementId::EbmlMaxSizeLength:
	case EbmlElementId::DocTypeVersion:
	case EbmlElementId::DocTypeReadVersion:
	case EbmlElementId::SeekPosition:
	case EbmlElementId::TimecodeScale:
	case EbmlElementId::Timecode:
	case EbmlElementId::TrackNumber:
	case EbmlElementId::TrackUID:
	case EbmlElementId::TrackType:
	case EbmlElementId::FlagEnabled:
	case EbmlElementId::FlagDefault:
	case EbmlElementId::FlagForced:
	case EbmlElementId::FlagLacing:
	case EbmlElementId::MinCache:
	case EbmlElementId::MaxBlockAdditionID:
	case EbmlElementId::CodecDecodeAll:
	case EbmlElementId::SeekPreRoll:
	case EbmlElementId::FlagInterlaced:
	case EbmlElementId::PixelWidth:
	case EbmlElementId::PixelHeight:
	case EbmlElementId::DefaultDuration:
	case EbmlElementId::Channels:
	case EbmlElementId::CueTime:
	case EbmlElementId::CueTrack:
	case EbmlElementId::CueClusterPosition:
	case EbmlElementId::CueRefTime:
	case EbmlElementId::CueBlockNumber:
	case EbmlElementId::Position:
		return EbmlElementType::UnsignedInteger;
	case EbmlElementId::DocType:
	case EbmlElementId::CodecID:
		return EbmlElementType::String;
	case EbmlElementId::Void:
	case EbmlElementId::SeekID:
	case EbmlElementId::SimpleBlock:
	case EbmlElementId::SegmentUID:
	case EbmlElementId::CodecPrivate:
		return EbmlElementType::Binary;
	case EbmlElementId::Duration:
	case EbmlElementId::TrackTimecodeScale:
	case EbmlElementId::SamplingFrequency:
		return EbmlElementType::Float;
	case EbmlElementId::DateUTC:
		return EbmlElementType::Date;
	case EbmlElementId::Title:
	case EbmlElementId::MuxingApp:
	case EbmlElementId::WritingApp:
	case EbmlElementId::CodecName:
		return EbmlElementType::Utf8String;
	default:
		throw std::runtime_error("unknown element");
	}
}

int get_ebml_element_level(EbmlElementId ebml_element_id)
{
	switch (ebml_element_id)
	{
	case EbmlElementId::Ebml:
	case EbmlElementId::Segment:
		return 0;
	case EbmlElementId::Cluster:
	case EbmlElementId::Cues:
	case EbmlElementId::DocType:
	case EbmlElementId::DocTypeReadVersion:
	case EbmlElementId::DocTypeVersion:
	case EbmlElementId::EbmlMaxIdLength:
	case EbmlElementId::EbmlMaxSizeLength:
	case EbmlElementId::EbmlReadVersion:
	case EbmlElementId::EbmlVersion:
	case EbmlElementId::Info:
	case EbmlElementId::SeekHead:
	case EbmlElementId::Tracks:
		return 1;
	case EbmlElementId::CuePoint:
	case EbmlElementId::DateUTC:
	case EbmlElementId::Duration:
	case EbmlElementId::MuxingApp:
	case EbmlElementId::Position:
	case EbmlElementId::SegmentUID:
	case EbmlElementId::Seek:
	case EbmlElementId::SimpleBlock:
	case EbmlElementId::Timecode:
	case EbmlElementId::TimecodeScale:
	case EbmlElementId::Title:
	case EbmlElementId::TrackEntry:
	case EbmlElementId::WritingApp:
		return 2;
	case EbmlElementId::Audio:
	case EbmlElementId::CodecDecodeAll:
	case EbmlElementId::CodecID:
	case EbmlElementId::CodecName:
	case EbmlElementId::CodecPrivate:
	case EbmlElementId::CueTime:
	case EbmlElementId::CueTrackPositions:
	case EbmlElementId::DefaultDuration:
	case EbmlElementId::FlagDefault:
	case EbmlElementId::FlagEnabled:
	case EbmlElementId::FlagForced:
	case EbmlElementId::FlagLacing:
	case EbmlElementId::MaxBlockAdditionID:
	case EbmlElementId::MinCache:
	case EbmlElementId::SeekID:
	case EbmlElementId::SeekPosition:
	case EbmlElementId::SeekPreRoll:
	case EbmlElementId::TrackNumber:
	case EbmlElementId::TrackTimecodeScale:
	case EbmlElementId::TrackType:
	case EbmlElementId::TrackUID:
	case EbmlElementId::Video:
		return 3;
	case EbmlElementId::Channels:
	case EbmlElementId::CueBlockNumber:
	case EbmlElementId::CueClusterPosition:
	case EbmlElementId::CueTrack:
	case EbmlElementId::FlagInterlaced:
	case EbmlElementId::PixelHeight:
	case EbmlElementId::PixelWidth:
	case EbmlElementId::SamplingFrequency:
		return 4;
	case EbmlElementId::CueRefTime:
		return 5;
	case EbmlElementId::Void:
		return -1;
	default:
		throw std::runtime_error("unknown element");
	}
}

EbmlElementId read_ebml_element_id(unsigned char* data, size_t& available_data_length, size_t& ebml_element_id_length)
{
	ebml_element_id_length = get_ebml_element_id_length(*data);
	if (ebml_element_id_length > available_data_length)
	{
		throw std::runtime_error(std::string("not enough data to read element id (element id length - ").append(std::to_string(ebml_element_id_length)).append(" bytes)"));
	}
	available_data_length -= ebml_element_id_length;
	unsigned char ebml_element_id_buffer[4] = { 0 };
	memcpy(&ebml_element_id_buffer[4 - ebml_element_id_length], data, ebml_element_id_length);
	auto ebml_element_id = ntohl(*reinterpret_cast<unsigned int*>(ebml_element_id_buffer));
	switch (ebml_element_id)
	{
	case EbmlElementId::Ebml:
	case EbmlElementId::EbmlVersion:
	case EbmlElementId::EbmlReadVersion:
	case EbmlElementId::EbmlMaxIdLength:
	case EbmlElementId::EbmlMaxSizeLength:
	case EbmlElementId::DocType:
	case EbmlElementId::DocTypeVersion:
	case EbmlElementId::DocTypeReadVersion:
	case EbmlElementId::Void:
	case EbmlElementId::Segment:
	case EbmlElementId::SeekHead:
	case EbmlElementId::Seek:
	case EbmlElementId::SeekID:
	case EbmlElementId::SeekPosition:
	case EbmlElementId::Info:
	case EbmlElementId::TimecodeScale:
	case EbmlElementId::Duration:
	case EbmlElementId::DateUTC:
	case EbmlElementId::Title:
	case EbmlElementId::MuxingApp:
	case EbmlElementId::WritingApp:
	case EbmlElementId::Cluster:
	case EbmlElementId::Timecode:
	case EbmlElementId::SimpleBlock:
	case EbmlElementId::SegmentUID:
	case EbmlElementId::Tracks:
	case EbmlElementId::TrackEntry:
	case EbmlElementId::TrackNumber:
	case EbmlElementId::TrackUID:
	case EbmlElementId::TrackType:
	case EbmlElementId::FlagEnabled:
	case EbmlElementId::FlagDefault:
	case EbmlElementId::FlagForced:
	case EbmlElementId::FlagLacing:
	case EbmlElementId::MinCache:
	case EbmlElementId::MaxBlockAdditionID:
	case EbmlElementId::CodecID:
	case EbmlElementId::CodecDecodeAll:
	case EbmlElementId::SeekPreRoll:
	case EbmlElementId::Video:
	case EbmlElementId::FlagInterlaced:
	case EbmlElementId::PixelWidth:
	case EbmlElementId::PixelHeight:
	case EbmlElementId::DefaultDuration:
	case EbmlElementId::TrackTimecodeScale:
	case EbmlElementId::CodecName:
	case EbmlElementId::CodecPrivate:
	case EbmlElementId::Audio:
	case EbmlElementId::SamplingFrequency:
	case EbmlElementId::Channels:
	case EbmlElementId::Cues:
	case EbmlElementId::CuePoint:
	case EbmlElementId::CueTime:
	case EbmlElementId::CueTrackPositions:
	case EbmlElementId::CueTrack:
	case EbmlElementId::CueClusterPosition:
	case EbmlElementId::CueRefTime:
	case EbmlElementId::CueBlockNumber:
	case EbmlElementId::Position:
		return static_cast<EbmlElementId>(ebml_element_id);
	default:
		{
			std::stringstream error;
			error << "unknown element id (0x" << std::hex << htonl(ebml_element_id) << ")";
			throw std::runtime_error(error.str());
		}
	}
	
}

size_t get_ebml_element_size_length(const unsigned char* data, size_t available_data_length)
{
	for (unsigned char position = 7; position > 0; position--)
	{
		if (*data & 1 << position)
		{
			return 8 - position;
		}
	}
	if (*data == 1)
	{
		return 8;
	}
	throw std::runtime_error("invalid element size");
}

__int64 get_ebml_element_size(const unsigned char* data, size_t available_data_length, size_t &ebml_element_size_length)
{
	ebml_element_size_length = get_ebml_element_size_length(data, available_data_length);
	if (ebml_element_size_length > available_data_length)
	{
		throw std::runtime_error(std::string("not enough data to read element size (element size length - ").append(std::to_string(ebml_element_size_length)).append(" bytes)"));
	}
	available_data_length -= ebml_element_size_length;
	unsigned char element_size[8] = { 0 };
	memcpy(&element_size[8 - ebml_element_size_length], data, ebml_element_size_length);
	element_size[8 - ebml_element_size_length] -= 1 << (8 - ebml_element_size_length);
	if (ebml_element_size_length == 1 && element_size[7] == 0x7f)
	{
		return -1;
	}
	return ntohll(*reinterpret_cast<unsigned __int64*>(element_size));
}

std::string get_ebml_element_value(EbmlElementId id, EbmlElementType type, unsigned char* data, unsigned __int64 size)
{
	switch (type)
	{
	case EbmlElementType::UnsignedInteger:
	case EbmlElementType::SignedInteger:
		{
			unsigned char Integer[8] = { 0 };
			memcpy(&Integer[8 - size], data, static_cast<size_t>(size));
			return std::to_string(type == EbmlElementType::UnsignedInteger ? ntohll(*reinterpret_cast<unsigned __int64*>(Integer)) : static_cast<long long>(ntohll(*reinterpret_cast<unsigned __int64*>(Integer))));
		}
	case EbmlElementType::String:
	case EbmlElementType::Utf8String:
		return std::string(reinterpret_cast<char*>(data), static_cast<unsigned int>(size));
	case EbmlElementType::Float:
		return std::to_string(size == 4 ? ntohf(*reinterpret_cast<unsigned int*>(data)) : ntohd(*reinterpret_cast<unsigned long long*>(data)));
	case EbmlElementType::Date:
		{
			time_t timestamp = 978307200 + ntohll(*reinterpret_cast<unsigned __int64*>(data)) / 100000000;
			char buffer[20];
			tm date_and_time;
			gmtime_s(&date_and_time, &timestamp);
			strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", &date_and_time);
			return std::string(buffer);
		}
	case EbmlElementType::Binary:
		{
			switch (id)
			{
			case EbmlElementId::SimpleBlock:
				{
					size_t track_number_size_length;
					auto track_number = get_ebml_element_size(data, static_cast<size_t>(size), track_number_size_length);
					char buffer[56];
					sprintf(buffer, "track %I64u, timecode %d, flags 0x%02x", track_number, ntohs(*reinterpret_cast<short*>(data + track_number_size_length)), *(data + track_number_size_length + 2));
					return std::string(buffer);
				}
			}
		}
	default:
		return "<cannot parse the content of this element>";
	}
}

class EbmlElement
{
public:
	EbmlElement(EbmlElementId id, EbmlElementType type, __int64 size, size_t id_length, size_t size_length, unsigned char* data = nullptr) : id_(id), type_(type), size_(size), id_length_(id_length), size_length_(size_length), data_(data)
	{
	}

	void add_child(EbmlElement child)
	{
		children_.push_back(child);
	}

	void print(unsigned int level)
	{
		std::string indent;
		if (level > 0)
		{
			std::cout << "|" << std::string(level - 1, ' ');
		}
		std::cout << "+ " << get_ebml_element_name(id_) << " (" << size_ << ")";
		if (type_ != EbmlElementType::Master)
		{
			std::cout << ": " << get_ebml_element_value(id_, type_, data_, size_);
		}
		std::cout << std::endl;
		for (EbmlElement& child : children_)
		{
			child.print(level + 1);
		}
	}

	__int64 size()
	{
		return size_;
	}

	__int64 element_size()
	{
		return id_length_ + size_length_ + size_;
	}

	void calculate_size()
	{
		unsigned __int64 size = 0;
		for (auto& child : children_)
		{
			size += child.element_size();
		}
		size_ = size;
	}

	EbmlElementId id() const
	{
		return id_;
	}

	const std::list<EbmlElement>& children() const
	{
		return children_;
	}

	const std::string value() const
	{
		if (type_ != EbmlElementType::Master)
		{
			return get_ebml_element_value(id_, type_, data_, size_);
		}
		throw std::runtime_error("cannot obtain a value from a master element");
	}

	const unsigned char* data() const
	{
		return data_;
	}

	__int64 size() const
	{
		return size_;
	}

private:
	EbmlElementId id_;
	EbmlElementType type_;
	__int64 size_;
	size_t id_length_;
	size_t size_length_;
	std::list<EbmlElement> children_;
	unsigned char* data_;
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
		if (argc < 2)
		{
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
		unsigned int video_track_number = 0;
		bool frame_callback_enabled = false;
		auto TrackEntry = std::find_if(Tracks->children().begin(), Tracks->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::TrackEntry; });
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
		auto start = timeGetTime();
		unsigned int cluster_pts = 0;
		auto Cluster = std::find_if(segment->children().begin(), segment->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::Cluster; });
		char timestamp[13] = { 0 };
		while (Cluster != segment->children().end())
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
				if (track_number == video_track_number)
				{
					unsigned char flags = *(data + track_number_size_length + 2);
					if ((flags & 0x6e) != 0)
					{
						throw std::runtime_error("lacing is not supported");
					}
					auto pts = cluster_pts + ntohs(*reinterpret_cast<const short*>(data + track_number_size_length));
					if (verbose)
					{
						auto pts_in_milliseconds = pts * milliseconds_per_tick;
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
					vpx_error = vpx_codec_decode(&codec_context, data + track_number_size_length + 3, size - track_number_size_length - 3, nullptr, 0);
					if (pts > 0)
					{
						auto end = timeGetTime(), difference = end - start;
						if (difference < pts * milliseconds_per_tick)
						{
							::Sleep((pts * milliseconds_per_tick) - difference);
						}
						else
						{
							std::cout << "decoder is late" << std::endl;
						}
					}
					if (vpx_error == VPX_CODEC_OK)
					{
						vpx_codec_iter_t iterator = nullptr;
						vpx_image_t* image = nullptr;
						while ((image = vpx_codec_get_frame(&codec_context, &iterator)) != nullptr)
						{
							sdl_error = SDL_UpdateYUVTexture(texture, nullptr, image->planes[0], image->stride[0], image->planes[1], image->stride[1], image->planes[2], image->stride[2]);
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
					}
				}
				SimpleBlock = std::find_if(++SimpleBlock, Cluster->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::SimpleBlock; });
			}
			Cluster = std::find_if(++Cluster, segment->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::Cluster; });
		}
	}
	catch (std::exception& exception)
	{
		std::cout << "exception: " << exception.what();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}