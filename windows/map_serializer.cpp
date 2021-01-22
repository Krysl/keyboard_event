#include <any>
#include <iostream>

#include "map_serializer.h"

// template <Encodable K, Encodable V>
template <typename K, typename V>
requires Encodable<K> &&Encodable<V> MapData<K, V>::MapData(
    const std::map<K, V> map)
    : _map(map) {}

// template <Encodable K, Encodable V>
template <typename K, typename V>
requires Encodable<K> &&Encodable<V> MapData<K, V>::~MapData() = default;

// template <typename K, typename V>
// template <typename KK, typename VV>
// const std::map<KK, VV> &MapData<K, V>::map() const {
//   return _map;
// }

// template <Encodable K, Encodable V>
template <typename K, typename V>
requires Encodable<K> &&Encodable<V> const std::map<K, V> &MapData<K, V>::map()
    const {
  return _map;
}

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

EVAR evar{0};

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
#if 0
    const MapData<std::string, int> &mapData =
        std::any_cast<MapData<std::string, int>>(*custom_value);
    const std::map<std::string, int> &map = mapData.map();
#else

#if defined(FLUTTER_ENABLE_RTTI) && FLUTTER_ENABLE_RTTI
    EVV variant{0};
    for_each_variant_type_kv(variant, custom_value, stream);
    // if (custom_value->type() == typeid(MapData<std::string, int>)) {
    //   WriteMapData<std::string, int>(custom_value, stream);
    // }
#else
#endif

#endif
    // size_t data_size = map.size();
    // WriteSize(data_size, stream);
    // for (const auto &pair : map) {
    //   WriteValue(EncodableValue(pair.first), stream);
    //   WriteValue(EncodableValue(pair.second), stream);
    // }
  } catch (const std::bad_cast &e) {
    std::cerr << e.what() << '\n';
    throw e;
  }
}

// EVV evv{0};
EVV0 var_int_str{0};
EVV1 var_int{0};

static_assert(decltype(isTypeInList<int>(std::declval<EVV0>()))::value);
static_assert(isTypeInList_v<int, EVV0>);
static_assert(isTypeInList_v<std::string, EVV0>);
static_assert(isTypeInList_v<std::string, EVV1> == false);

constexpr auto toFalse() -> std::false_type { return {}; }
constexpr auto toTrue() -> std::true_type { return {}; }

static_assert(toFalse() == false);
static_assert(toTrue() == true);

static_assert(isVariantMember<int>(var_int) == true);
static_assert(isVariantMember<std::string>(var_int) == false);

static_assert(isVariantMember<int>(var_int_str) == true);
static_assert(isVariantMember<std::string>(var_int_str) == true);

//*
auto a = isVariantMember<int>(var_int);  // 编译通过
/*/
auto a = isVariantMember<std::string>(var_int);
//*/

// 避免报错
// error LNK2019: 无法解析的外部符号
template MapData<std::string, int>;
template MapData<int, std::string>;
// template MapData<int, std::map<std::string, int>>;
template MapData<int, char>;
int ccccc = (char)0x1;
