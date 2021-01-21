#include <any>
#include <iostream>

#include "map_serializer.h"

MapSerializer::MapSerializer() = default;

MapSerializer::~MapSerializer() = default;

// // static
// const MapSerializer &MapSerializer::GetInstance() {
//   static MapSerializer sInstance;
//   return sInstance;
// }

EncodableValue MapSerializer::ReadValueOfType(uint8_t type,
                                              ByteStreamReader *stream) const {
  return StandardCodecSerializer::ReadValueOfType(type, stream);
}
void MapSerializer::WriteValue(const EncodableValue &value,
                               ByteStreamWriter *stream) const {
  auto custom_value = std::get_if<CustomEncodableValue>(&value);
  if (!custom_value) {
    StandardCodecSerializer::WriteValue(value, stream);
    return;
  }
  stream->WriteByte(13);  // Type: EncodedType::kMap
  try {
    const MapData<std::string, int> &mapData =
        std::any_cast<MapData<std::string, int>>(*custom_value);
    size_t data_size = mapData.map().size();
    WriteSize(data_size, stream);
    for (const auto &pair : mapData.map()) {
      WriteValue(EncodableValue(pair.first), stream);
      WriteValue(EncodableValue(pair.second), stream);
    }
  } catch (const std::bad_cast &e) {
    std::cerr << e.what() << '\n';
    throw e;
  }
}