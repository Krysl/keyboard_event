#if !defined(__MAP_SERIALIZER_H__)
#define __MAP_SERIALIZER_H__

#include <map>

#include <flutter/standard_codec_serializer.h>

using namespace flutter;

class MapSerializer : public StandardCodecSerializer {
 public:
  MapSerializer();
  virtual ~MapSerializer();

  // static const MapSerializer &GetInstance();

 protected:
  // |TestCodecSerializer|
  EncodableValue ReadValueOfType(uint8_t type,
                                 ByteStreamReader *stream) const override;

  // |TestCodecSerializer|
  void WriteValue(const EncodableValue &value,
                  ByteStreamWriter *stream) const override;
};

template <typename K = internal::EncodableValueVariant,
          typename V = internal::EncodableValueVariant>
class MapData {
 public:
  MapData(const std::map<K, V> map) : _map(map) {}
  ~MapData() = default;
  const std::map<K, V> map() const { return _map; }

 private:
  std::map<K, V> _map;
};

#endif  // __MAP_SERIALIZER_H__
