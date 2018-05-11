#include "Ebml.h"

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

EbmlElement::EbmlElement(EbmlElementId id, EbmlElementType type, __int64 size, size_t id_length, size_t size_length, unsigned char* data) : id_(id), type_(type), size_(size), id_length_(id_length), size_length_(size_length), data_(data)
{
}

void EbmlElement::add_child(EbmlElement child)
{
	children_.push_back(child);
}

void EbmlElement::print(unsigned int level)
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

__int64 EbmlElement::size()
{
	return size_;
}

__int64 EbmlElement::element_size()
{
	return id_length_ + size_length_ + size_;
}

void EbmlElement::calculate_size()
{
	unsigned __int64 size = 0;
	for (auto& child : children_)
	{
		size += child.element_size();
	}
	size_ = size;
}

EbmlElementId EbmlElement::id() const
{
	return id_;
}

const std::list<EbmlElement>& EbmlElement::children() const
{
	return children_;
}

const std::string EbmlElement::value() const
{
	if (type_ != EbmlElementType::Master)
	{
		return get_ebml_element_value(id_, type_, data_, size_);
	}
	throw std::runtime_error("cannot obtain a value from a master element");
}

const unsigned char* EbmlElement::data() const
{
	return data_;
}

__int64 EbmlElement::size() const
{
	return size_;
}
