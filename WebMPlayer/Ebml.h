#pragma once

#include <string>
#include <list>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <ctime>

#include <Winsock2.h>

enum class EbmlParserState
{
	ParseElementId,
	ParseElementLength,
	ParseElementValue,
};

size_t get_ebml_element_id_length(unsigned char first_byte_of_element_id);

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

std::string get_ebml_element_name(EbmlElementId ebml_element_id);

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

EbmlElementType get_ebml_element_type(EbmlElementId ebml_element_id);
int get_ebml_element_level(EbmlElementId ebml_element_id);
EbmlElementId read_ebml_element_id(unsigned char* data, size_t& available_data_length, size_t& ebml_element_id_length);
size_t get_ebml_element_size_length(const unsigned char* data, size_t available_data_length);
__int64 get_ebml_element_size(const unsigned char* data, size_t available_data_length, size_t &ebml_element_size_length);
EbmlElementId read_ebml_element_id(unsigned char* data, size_t& available_data_length, size_t& ebml_element_id_length);
std::string get_ebml_element_value(EbmlElementId id, EbmlElementType type, unsigned char* data, unsigned __int64 size);

class EbmlElement
{
public:
	EbmlElement(EbmlElementId id, EbmlElementType type, __int64 size, size_t id_length, size_t size_length, unsigned char* data = nullptr);

	void add_child(EbmlElement child);
	void print(unsigned int level);
	__int64 size();
	__int64 element_size();
	void calculate_size();
	EbmlElementId id() const;
	const std::list<EbmlElement>& children() const;
	const std::string value() const;
	const unsigned char* data() const;
	__int64 size() const;

private:
	EbmlElementId id_;
	EbmlElementType type_;
	__int64 size_;
	size_t id_length_;
	size_t size_length_;
	std::list<EbmlElement> children_;
	unsigned char* data_;
};