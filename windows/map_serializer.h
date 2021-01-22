#if !defined(__MAP_SERIALIZER_H__)
#define __MAP_SERIALIZER_H__

#include <concepts>
#include <map>
#include <optional>
#include <utility>

#include <flutter/standard_codec_serializer.h>

using namespace flutter;

using EVV = internal::EncodableValueVariant;
using EVV0 = std::variant<std::monostate, int32_t, std::string>;
using EVV1 = std::variant<std::monostate, int32_t>;
using EVAR = std::variant<std::monostate, int, std::string>;

extern EVV evv;
extern EVV0 var_int_str;
extern EVV1 var_int;

// template <typename T, typename VARIANT_T>
// struct isVariantMember;

// template <typename T, typename... ALL_T>
// struct isVariantMember<T, std::variant<ALL_T...>>
//     : public std::disjunction<std::is_same<T, ALL_T>...> {};

template <typename T, template <typename...> class C, typename... Ts>
constexpr auto isTypeInList(C<Ts...> const &)
    -> std::disjunction<std::is_same<T, Ts>...>;
// 👆 函数声明，无函数体
// decltype(isTypeInList<int>(std::declval<EVV0>())) 获得返回值类型，
//    即 std::disjunction<std::is_same<T, Ts>...>
//    其为“Bool类型”类型，可用 ::value 获取其值

template <typename T, typename V>
static constexpr bool isTypeInList_v =
    decltype(isTypeInList<T>(std::declval<V>()))::value;

template <typename T, template <typename...> class C, typename... Ts>
constexpr auto isVariantMember(C<Ts...> const &)
    -> std::disjunction<std::is_same<T, Ts>...> {
  return {};
}

template <typename T>
concept Encodable = requires(T a) {
#define TEST3
#ifdef TEST1
  {
    // (isVariantMember<T, EVV0>)
    isTypeInList_v<T, EVV1> ? true : "err"
  }
  ->std::same_as<bool>;
#endif
#ifdef TEST2
  {
    true  // ->std::same_as<bool>;
    // 1    // ->std::same_as<int>;
    // ((true == true) ? true : 1)
    // ((isVariantMember<T>(var_int_str) == true) ? true : 1)
    // isVariantMember<T>(var_int) ? true : "err"
    // isVariantMember<T>(var_int) ?  "err" : false
  }
  // ->std::same_as<bool>;
  // ->std::same_as<int>;
  // ->std::is_same<std::true_type>;
  // -> std::convertible_to<bool>; // true
  ->std::convertible_to<std::true_type>;

#endif
#ifdef TEST4
  {
    isTypeInList<T>(var_int)
    // isTypeInList<T>(evv)
  }
  ->std::same_as<std::true_type>;
#endif
#ifdef TEST3
  // EncodableValue(a);
  evv = a;
#endif
};

// template <Encodable K, Encodable V>
template <typename K, typename V>
requires Encodable<K> &&Encodable<V> class MapData {
 public:
  MapData(const std::map<K, V> map);
  ~MapData();
  // template <typename KK = K, typename VV = V>
  // const std::map<KK, VV> &map() const;
  const std::map<K, V> &map() const;

 private:
  std::map<K, V> _map;
};

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

  template <typename K, typename V>
  requires Encodable<K> &&Encodable<V> bool WriteMapData(
      const CustomEncodableValue *custom_value, ByteStreamWriter *stream) const;

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

template <class... Args>
auto any_to_variant_cast(std::any a) -> std::variant<Args...> {
  if (!a.has_value()) throw std::bad_any_cast();

  std::optional<std::variant<Args...>> v = std::nullopt;

  bool found = (                                         //
      (                                                  //
          a.type() == typeid(Args) &&                    //
          (v = std::any_cast<Args>(std::move(a)), true)  //
          )                                              //
      ||                                                 //
      ...                                                //
  );

  if (!found) throw std::bad_any_cast{};

  return std::move(*v);
}

#endif  // __MAP_SERIALIZER_H__
