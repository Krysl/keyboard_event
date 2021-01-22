#include <any>
#include <iostream>

#include "map_serializer.h"

EV ev{0}; // used by concepts "Encodable"

template <Encodable K, Encodable V>
MapData<K, V>::MapData(const std::map<K, V> map) : _map(map) {}

template <Encodable K, Encodable V>
MapData<K, V>::~MapData() = default;

template <Encodable K, Encodable V>
const std::map<K, V> &MapData<K, V>::map() const {
  return _map;
}

MapSerializer::MapSerializer() = default;

MapSerializer::~MapSerializer() = default;

// static
const MapSerializer &MapSerializer::GetInstance() {
  static MapSerializer sInstance;
  return sInstance;
}

template <typename K, typename V>
requires Encodable<K> &&Encodable<V> bool MapSerializer::WriteMapData(
    const CustomEncodableValue *custom_value, ByteStreamWriter *stream) const {
  const MapData<K, V> &mapData = std::any_cast<MapData<K, V>>(*custom_value);
  const std::map<K, V> &map = mapData.map();
  size_t data_size = map.size();
  WriteSize(data_size, stream);
  for (const auto &pair : map) {
    WriteValue(EncodableValue(pair.first), stream);
    WriteValue(EncodableValue(pair.second), stream);
  }
  return true;
}
#pragma warning(disable : 4706)
template <typename KEY, template <typename...> class C, typename... VAL>
bool MapSerializer::for_each_variant_type_v(
    C<VAL...> const &, const CustomEncodableValue *custom_value,
    ByteStreamWriter *stream) const {
  bool ret = false;
  ((                                                           //
       (custom_value->type() == typeid(MapData<KEY, VAL>)) &&  //
       WriteMapData<KEY, VAL>(custom_value, stream) &&         //
       (ret = true)                                            //
       //
       ) ||
   ...);
  return ret;
}

template <template <typename...> class C, typename... KEY>
void MapSerializer::for_each_variant_type_k(
    C<KEY...> const &variant, const CustomEncodableValue *custom_value,
    ByteStreamWriter *stream) const {
  ((for_each_variant_type_v<KEY>(variant, custom_value, stream)) || ...);
}

template <template <typename...> class C, typename... KEY>
void MapSerializer::for_each_variant_type_kv(
    C<KEY...> const &variant, const CustomEncodableValue *custom_value,
    ByteStreamWriter *stream) const {
  for_each_variant_type_k(variant, custom_value, stream);
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
#if defined(FLUTTER_ENABLE_RTTI) && FLUTTER_ENABLE_RTTI
    EVV variant{0};
    for_each_variant_type_kv(variant, custom_value, stream);
#endif
  } catch (const std::bad_cast &e) {
    std::cerr << e.what() << '\n';
    throw e;
  }
}

// 避免报错
// error LNK2019: 无法解析的外部符号
template MapData<std::string, int>;
template MapData<int, std::string>;
template MapData<int, std::vector<EV>>;
// template MapData<int, std::map<std::string, int>>;  // 尚未实现
