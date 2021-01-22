#if !defined(__MAP_SERIALIZER_H__)
#define __MAP_SERIALIZER_H__

#include <concepts>
#include <map>
#include <vector>

#include <flutter/standard_codec_serializer.h>

using namespace flutter;

using EVV = internal::EncodableValueVariant;
using EV = flutter::EncodableValue;

extern EV ev;

template <typename T>
concept Encodable = requires(T a) {
  ev = a;
};

template <Encodable K, Encodable V>
class MapData {
 public:
  MapData(const std::map<K, V> map);
  ~MapData();

  const std::map<K, V> &map() const;

 private:
  std::map<K, V> _map;
};

class MapSerializer : public StandardCodecSerializer {
 public:
  virtual ~MapSerializer();

  static const MapSerializer &GetInstance();

  // Prevent copying.
  MapSerializer(MapSerializer const &) = delete;
  MapSerializer &operator=(MapSerializer const &) = delete;

  void WriteValue(const EncodableValue &value,
                  ByteStreamWriter *stream) const override;

 protected:
  template <typename K, typename V>
  requires Encodable<K> &&Encodable<V> bool WriteMapData(
      const CustomEncodableValue *custom_value, ByteStreamWriter *stream) const;

  MapSerializer();

 private:
  template <typename KEY, template <typename...> class C, typename... VAL>
  bool for_each_variant_type_v(C<VAL...> const &,
                               const CustomEncodableValue *custom_value,
                               ByteStreamWriter *stream) const;
  template <template <typename...> class C, typename... KEY>
  void for_each_variant_type_k(C<KEY...> const &variant,
                               const CustomEncodableValue *custom_value,
                               ByteStreamWriter *stream) const;

  template <template <typename...> class C, typename... KEY>
  void for_each_variant_type_kv(C<KEY...> const &variant,
                                const CustomEncodableValue *custom_value,
                                ByteStreamWriter *stream) const;
};

#endif  // __MAP_SERIALIZER_H__
