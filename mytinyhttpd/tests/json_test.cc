#include <gtest/gtest.h>

#include <fstream>
#include <nlohmann/json.hpp>

using namespace nlohmann;

TEST(JsonTest, SerializationTest) {
  json j1;
  j1["pi"] = 3.141;
  j1["happy"] = true;
  j1["name"] = "Niels";
  j1["nothing"] = nullptr;
  j1["answer"]["everything"] = 42;
  j1["list"] = {1, 0, 2};
  j1["object"] = {{"currency", "USD"}, {"value", 42.99}};

  json j2 = {{"pi", 3.141},
             {"happy", true},
             {"name", "Niels"},
             {"nothing", nullptr},
             {"answer", {{"everything", 42}}},
             {"list", {1, 0, 2}},
             {"object", {{"currency", "USD"}, {"value", 42.99}}}};

  ASSERT_EQ(j1.dump(), j2.dump());
  ASSERT_EQ(j1.dump(4), j2.dump(4));
  ASSERT_NE(j1.dump(), j2.dump(4));
}

TEST(JsonTest, DeserializationTest) {
  // create object from string literal
  json j1 = "{ \"happy\": true, \"pi\": 3.141 }"_json;

  auto j2 = R"({
    "happy": true,
    "pi": 3.141
  })"_json;

  auto j3 = json::parse(R"( {"happy": true, "pi": 3.141} )");

  ASSERT_EQ(j1.dump(), j2.dump());
  ASSERT_EQ(j1.dump(), j3.dump());
}

TEST(JsonTest, ToFromStreamTest) {
  auto j1 = json::parse(R"( {"happy": true, "pi": 3.141} )");
  std::ofstream o("test.json");
  o << std::setw(4) << j1 << std::endl;
  std::ifstream i("test.json");
  json j2;
  i >> j2;
  ASSERT_EQ(j1.dump(), j2.dump());
}

TEST(JsonTest, ReadFromIteratorRangeTest) {
  std::vector<std::uint8_t> v = {'t', 'r', 'u', 'e'};
  json j1 = json::parse(v.begin(), v.end());
  json j2 = json::parse(v);
  ASSERT_EQ(j1.dump(), j2.dump());
}

TEST(JsonTest, StlLikeAccessTest) {
  {
    // json array
    json j;
    j.push_back("foo");
    j.push_back(1);
    j.push_back(true);
    j.emplace_back(1.78);

    for (json::iterator it = j.begin(); it != j.end(); ++it) {
      std::cout << *it << std::endl;
    }
    for (auto& element : j) {
      std::cout << element << std::endl;
    }

    ASSERT_EQ(j[0].get<std::string>(), "foo");
    ASSERT_EQ(j[1], 1);
    ASSERT_TRUE(j.at(2));

    ASSERT_EQ(j, R"(["foo", 1, true, 1.78])"_json);

    ASSERT_EQ(j.size(), 4);
    ASSERT_FALSE(j.empty());
    ASSERT_EQ(j.type(), json::value_t::array);

    j.clear();

    ASSERT_FALSE(j.is_null());
    ASSERT_FALSE(j.is_boolean());
    ASSERT_FALSE(j.is_number());
    ASSERT_FALSE(j.is_object());
    ASSERT_TRUE(j.is_array());
    ASSERT_FALSE(j.is_string());
  }

  {
    // json object
    json o;
    o["foo"] = 23;
    o["bar"] = false;
    o["baz"] = 3.141;
    o.emplace("weather", "sunny");

    for (json::iterator it = o.begin(); it != o.end(); ++it) {
      std::cout << it.key() << " : " << it.value() << "\n";
    }
    for (auto& el : o.items()) {
      std::cout << el.key() << " : " << el.value() << "\n";
    }

    ASSERT_TRUE(o.contains("foo"));
    ASSERT_TRUE(o.find("foo") != o.end());
    ASSERT_EQ(o.count("foo"), 1);
    ASSERT_EQ(o.count("fob"), 0);

    o.erase("foo");
  }
}